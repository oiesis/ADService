//
// Created by guoze.lin on 16/3/16.
//

#include <functional>
#include <muduo/base/Logging.h>
#include "core_adselect_manager.h"
#include "core_cache_manager.h"
#include "utility/json.h"
#include "utility/mttytime.h"
#include "constants.h"
#include "atomic.h"
#include "common/types.h"

namespace adservice{
    namespace adselect{

        using namespace muduo;
        using namespace adservice::cache;
        using namespace adservice::utility::json;
        using namespace adservice::utility::time;

        static const int DEFAULT_KEY_LENGTH = 128;
        static const int LITTLE_BUFFER_SIZE = 1024;
        static const int LARGE_BUFFER_SIZE = 10240;
        static const int CACHE_LEVEL_1_SIZE = 1;
        static const int CACHE_LEVEL_2_SIZE = 2048;
        static const int CACHE_LEVEL_3_SIZE = 4096;
        static const char* DEFAULT_SUPPORT_BANNERTYPE = "1,2,3,4,5";
        static const char CACHE_RESULT_MTSTATUS[9] = "mtstatus";

        /**
         * 当请求结果并没有逻辑错误时,添加成功标记
         */
        inline void appendMtStatus(rapidjson::Document& result){
            result.AddMember("mtstatus",MakeStringConstValue("ok"), result.GetAllocator());
        }

        /**
         * 检查结果是否有成功标记
         */
        inline bool hasMtStatus(rapidjson::Document& result){
            return result.HasMember("mtstatus");
        }

        rapidjson::Value& AdSelectManager::queryCreativeById(int seqId,const std::string& bannerId,rapidjson::Document& result){
            ElasticSearch& agent = getAvailableConnection(seqId);
            int cnt = 0;
            try {
                char buffer[LITTLE_BUFFER_SIZE];
                snprintf(buffer,LITTLE_BUFFER_SIZE,dsl_query_banner, std::stoi(bannerId));
                cnt = agent.search(ES_INDEX_SOLUTIONS, ES_DOCUMENT_BANNER, ES_FILTER_FORMAT, buffer, result);
            }catch(std::exception& e){
                DebugMessageWithTime("exception in queryCreativeById,e:",e.what());
            }
            if(cnt!=1) {
                DebugMessageWithTime("error occured in queryCreativeById,document id:",bannerId);
                //throw AdSelectException("not exactly one docuemnts fetched in queryCreativeById", -1);
                return result;
            }
            return result["hits"]["hits"][0]["_source"];
        }

        rapidjson::Value& AdSelectManager::queryCreativeByIdCache(int seqId,const std::string& bannerId,rapidjson::Document& result){
            try{
                ElasticSearch& agent = getAvailableConnection(seqId);
                char key[DEFAULT_KEY_LENGTH];
                snprintf(key,DEFAULT_KEY_LENGTH,"creative_id_%s",bannerId.c_str());
                CacheResult* cacheResult = cacheManager.get(key,CACHE_LEVEL_1_SIZE,[&bannerId,this,&agent,&result](CacheResult& newCache){
                    int cnt = 0;
                    try {
                        char buffer[LITTLE_BUFFER_SIZE];
                        snprintf(buffer,LITTLE_BUFFER_SIZE, this->dsl_query_banner, std::stoi(bannerId));
                        cnt = agent.search(ES_INDEX_SOLUTIONS, ES_DOCUMENT_BANNER, ES_FILTER_FORMAT, buffer, result);
                    }catch(std::exception& e){
                        DebugMessageWithTime("exception in queryCreativeByIdCache,e:",e.what());
                    }
                    if(cnt != 1){
                        DebugMessageWithTime("error occured in queryCreativeByIdCache,document id:",bannerId);
                        return false;
                    }
                    appendMtStatus(result);
                    std::string jsonResult = toJson(result);
                    if(jsonResult.length()>=newCache.size) {
                        DebugMessage("in queryCreativeByIdCache result too large,bannerId:",bannerId ,",result size:",jsonResult.length());
                        return false;
                    }
                    memcpy(newCache.data, jsonResult.data(), jsonResult.length());
                    newCache.data[jsonResult.length()] = '\0';
                    newCache.size = jsonResult.length();
                    newCache.expireTime = getCurrentTimeStamp() + ADSELECT_CACHE_EXPIRE_TIME;
                    return true;
                });
                if(cacheResult!=NULL && result.ObjectEmpty()){
                    parseJson((const char*)cacheResult->data,result);
                }else if(result.ObjectEmpty()||!hasMtStatus(result)){
                    DebugMessage("in queryCreativeByIdCache,failed to fetch valid creative for bannerId ",bannerId);
                    return result;
                }
            }catch(std::exception& e){
                DebugMessageWithTime("error occured in queryCreativeById,document id:",bannerId);
                return result;
            }
            return result["hits"]["hits"][0]["_source"];
        }

#define ES_SIZE_FILTER "{\"bool\": {\"must\": [{\"term\":{\"width\":%d}},{\"term\":{\"height\":%d}}]}}"


        rapidjson::Value& AdSelectManager::queryAdInfoByPid(int seqId,const std::string& pid,rapidjson::Document& result,bool isAdxPid){
            AdSelectCondition condition;
            if(isAdxPid){
                condition.adxpid = pid;
                return queryAdInfoByAdxPid(seqId,condition,result);
            }else{
                condition.mttyPid = pid;
                return queryAdInfoByMttyPid(seqId,condition,result);
            }
        }

        rapidjson::Value& AdSelectManager::queryAdInfoByCondition(int seqId, AdSelectCondition &condition,
                                                                  rapidjson::Document &result, bool isAdxPid,bool simpleQuery) {
            if(isAdxPid){
                return queryAdInfoByAdxPidNoCache(seqId,condition,result,simpleQuery);
            }else{
                return queryAdInfoByMttyPidNoCache(seqId,condition,result,simpleQuery);
            }
        }

        static int countryGeo(int geo){
            return geo - (geo%AREACODE_MARGIN);
        }

        int frontEndFlowType2BackEndType(int flowType){
            switch(flowType){
                case FLOWTYPE_FRONTEND_PC:
                    return SOLUTION_FLOWTYPE_PC;
                case FLOWTYPE_FRONTEND_MOBILE:
                case FLOWTYPE_FRONTEND_INAPP:
                    return SOLUTION_FLOWTYPE_MOBILE;
                default:
                    return SOLUTION_FLOWTYPE_ALL;
            }
        }

        /**
         * 进行条件参数绑定
         */
        void bindSelectCondition(rapidjson::Value& adplaceInfo,AdSelectCondition& bindCondition,const char* cTemplate,INOUT char* output,const char* pid){
            //创意支持类型过滤
            const char* supportBanner = adplaceInfo["supportbanner"].IsNull()?DEFAULT_SUPPORT_BANNERTYPE:adplaceInfo["supportbanner"].GetString();
            if(bindCondition.mttyPid.empty()){
                bindCondition.mttyPid = std::to_string(adplaceInfo["pid"].GetInt());
            }
            if(bindCondition.adxpid.empty()){
                bindCondition.adxpid = adplaceInfo["adxpid"].GetString();
            }
            bindCondition.mediaType = adplaceInfo["mediatype"].GetInt();
            bindCondition.adplaceType = adplaceInfo["adplacetype"].GetInt();
            bindCondition.displayNumber = adplaceInfo["displaynumber"].GetInt();
            bindCondition.width = adplaceInfo["width"].GetInt();
            bindCondition.height = adplaceInfo["height"].GetInt();
            //时间定点过滤
            std::string dHour = adSelectTimeCodeUtc();
            bindCondition.dHour = dHour;
            //国家通投编码
            int dCountryGeo = countryGeo(bindCondition.dGeo);
            sprintf(output,cTemplate,
                    pid,
                    pid,
                    bindCondition.adxid,
                    bindCondition.mediaType,
                    pid,
                    bindCondition.adplaceType,
                    bindCondition.displayNumber,
                    bindCondition.flowType,
                    bindCondition.dHour.data(),
                    bindCondition.dGeo,dCountryGeo,
                    bindCondition.mobileDevice,
                    bindCondition.pcOS,
                    bindCondition.dealId.data(),
                    bindCondition.width,
                    bindCondition.height,
                    supportBanner,
                    pid,
                    bindCondition.dGeo,
                    bindCondition.width,
                    bindCondition.height,
                    supportBanner,
                    bindCondition.adxid,
                    bindCondition.mediaType,
                    pid,
                    bindCondition.adplaceType,
                    bindCondition.displayNumber,
                    bindCondition.flowType,
                    bindCondition.dHour.data(),
                    bindCondition.dGeo,dCountryGeo,
                    bindCondition.mobileDevice,
                    bindCondition.pcOS,
                    bindCondition.dealId.data(),
                    pid,
                    bindCondition.dGeo
            );
        }

        void bindSelectConditionSimple(rapidjson::Value& adplaceInfo,AdSelectCondition& bindCondition,const char* cTemplate,INOUT char* output,const char* pid){
            //创意支持类型过滤
            const char* supportBanner = adplaceInfo["supportbanner"].IsNull()?DEFAULT_SUPPORT_BANNERTYPE:adplaceInfo["supportbanner"].GetString();
            if(bindCondition.mttyPid.empty()){
                bindCondition.mttyPid = std::to_string(adplaceInfo["pid"].GetInt());
            }
            if(bindCondition.adxpid.empty()){
                bindCondition.adxpid = adplaceInfo["adxpid"].GetString();
            }
            bindCondition.mediaType = adplaceInfo["mediatype"].GetInt();
            bindCondition.adplaceType = adplaceInfo["adplacetype"].GetInt();
            bindCondition.displayNumber = adplaceInfo["displaynumber"].GetInt();
            bindCondition.width = adplaceInfo["width"].GetInt();
            bindCondition.height = adplaceInfo["height"].GetInt();
            //时间定点过滤
            std::string dHour = adSelectTimeCodeUtc();
            bindCondition.dHour = dHour;
            //国家通投编码
            int dCountryGeo = countryGeo(bindCondition.dGeo);
            sprintf(output,cTemplate,
                    pid,
                    pid,
                    bindCondition.width,
                    bindCondition.height,
                    supportBanner,
                    bindCondition.width,
                    bindCondition.height,
                    supportBanner
            );
        }

        //一般是通投条件,目前用于广点通
        void bindSelectConditionMultiSize(AdSelectCondition& bindCondition,const char* cTemplate,INOUT char* output){
            PreSetAdplaceInfo& adplaceInfo = *bindCondition.pAdplaceInfo;
            //创意支持类型过滤
            const char* supportBanner = DEFAULT_SUPPORT_BANNERTYPE;
            char multiSizeBuffer[1024]="\0";
            vector<std::tuple<int,int>>& sizeArray = adplaceInfo.sizeArray;
            int len = 0;
            for(auto& iter : sizeArray){
                int l = snprintf(multiSizeBuffer+len,sizeof(multiSizeBuffer)-len,ES_SIZE_FILTER,std::get<0>(iter),std::get<1>(iter));
                len += l;
                multiSizeBuffer[len++]=',';
            }
            multiSizeBuffer[len-1]='\0';
            //时间定点过滤
            std::string dHour = adSelectTimeCodeUtc();
            bindCondition.dHour = dHour;
            //国家通投编码
            int dCountryGeo = countryGeo(bindCondition.dGeo);
            sprintf(output,cTemplate,
                    bindCondition.adxid,
                    bindCondition.flowType,
                    bindCondition.dHour.data(),
                    bindCondition.dGeo,dCountryGeo,
                    bindCondition.mobileDevice,
                    bindCondition.pcOS,
                    supportBanner,
                    multiSizeBuffer,
                    bindCondition.dGeo,
                    supportBanner,
                    multiSizeBuffer,
                    bindCondition.adxid,
                    bindCondition.flowType,
                    bindCondition.dHour.data(),
                    bindCondition.dGeo,dCountryGeo,
                    bindCondition.mobileDevice,
                    bindCondition.pcOS,
                    bindCondition.dGeo
            );
        }

        rapidjson::Value& AdSelectManager::queryAdInfoByMttyPidNoCache(int seqId,AdSelectCondition& selectCondition,rapidjson::Document& result,bool simpleQuery){
            try {
                const std::string &mttyPid = selectCondition.mttyPid;
                ElasticSearch &agent = getAvailableConnection(seqId);
                char buffer[LARGE_BUFFER_SIZE];
                int cnt;
                if(selectCondition.pAdplaceInfo==NULL) {
                    snprintf(buffer, LARGE_BUFFER_SIZE, this->dsl_query_adplace_pid, mttyPid.c_str());
                    rapidjson::Document adplace;
                    cnt = agent.search2(ES_INDEX_SOLUTIONS, ES_DOCUMENT_ADPLACE, ES_FILTER_FORMAT, buffer,
                                       adplace);
                    if (cnt == 0)
                        return result;
                    rapidjson::Value &adplaceInfo = adplace["hits"]["hits"][0]["_source"];
                    if(simpleQuery){
                        bindSelectConditionSimple(adplaceInfo,selectCondition,this->dsl_query_adinfo_condition_simple,
                                                  buffer,
                                                  mttyPid.c_str());
                    } else{
                        bindSelectCondition(adplaceInfo, selectCondition, this->dsl_query_adinfo_condition,
                                            buffer,
                                            mttyPid.c_str());
                    }
                    cnt = agent.search2(ES_INDEX_SOLUTIONS, ES_DOCUMENT_SOLBANADPLACE, ES_FILTER_FORMAT2, buffer,
                                       result);
                    if (cnt <= 2)
                        return result;
                }else{
                    bindSelectConditionMultiSize(selectCondition,this->dsl_query_adinfo_condition_multisize,buffer);
                    cnt = agent.search2(ES_INDEX_SOLUTIONS,ES_DOCUMENT_SOLBANADPLACE,ES_FILTER_FORMAT2,buffer,result);
                    if(cnt<2){
                        return result;
                    }
                }
            }catch(std::exception& e){
                DebugMessageWithTime("in queryAdInfoByMttyPidNoCache exception occured:",e.what());
            }
            return result["hits"]["hits"];
        }


        rapidjson::Value& AdSelectManager::queryAdInfoByAdxPidNoCache(int seqId,AdSelectCondition& selectCondition,rapidjson::Document& result,bool simpleQuery){
            try {
                const std::string &adxPid = selectCondition.adxpid;
                ElasticSearch &agent = getAvailableConnection(seqId);
                char buffer[LARGE_BUFFER_SIZE];
                int cnt;
                if(selectCondition.pAdplaceInfo==NULL) {
                    snprintf(buffer, LARGE_BUFFER_SIZE, this->dsl_query_adplace_adxpid, adxPid.c_str());
                    rapidjson::Document adplace;
                    cnt = agent.search2(ES_INDEX_SOLUTIONS, ES_DOCUMENT_ADPLACE, ES_FILTER_FORMAT, buffer,
                                           adplace);
                    if (cnt == 0)
                        return result;
                    rapidjson::Value &adplaceInfo = adplace["hits"]["hits"][0]["_source"];
                    std::string pid = to_string(adplaceInfo["pid"].GetInt());
                    if(simpleQuery){
                        bindSelectConditionSimple(adplaceInfo,selectCondition,this->dsl_query_adinfo_condition_simple,
                                                  buffer,
                                                  pid.c_str());
                    }
                    else {
                        bindSelectCondition(adplaceInfo, selectCondition, this->dsl_query_adinfo_condition,
                                            buffer,
                                            pid.c_str());
                    }
                    cnt = agent.search2(ES_INDEX_SOLUTIONS, ES_DOCUMENT_SOLBANADPLACE, ES_FILTER_FORMAT2, buffer,
                                       result);
                    if (cnt <= 2)
                        return result;
                }else{
                    bindSelectConditionMultiSize(selectCondition,this->dsl_query_adinfo_condition_multisize,buffer);
                    cnt = agent.search2(ES_INDEX_SOLUTIONS,ES_DOCUMENT_SOLBANADPLACE,ES_FILTER_FORMAT2,buffer,result);
                    if(cnt<2){
                        return result;
                    }
                }
            }catch(std::exception& e){
                DebugMessageWithTime("in queryAdInfoByAdxPidNoCache exception occured:,",e.what());
            }
            return result["hits"]["hits"];
        }

        rapidjson::Value& AdSelectManager::queryAdInfoByMttyPid(int seqId,AdSelectCondition& selectCondition,rapidjson::Document& result,bool simpleQuery){
            try{
                const std::string& mttyPid = selectCondition.mttyPid;
                ElasticSearch& agent = getAvailableConnection(seqId);
                char key[DEFAULT_KEY_LENGTH];
                if(simpleQuery){
                    snprintf(key,DEFAULT_KEY_LENGTH,"adinfo_pid_%s",mttyPid.c_str());
                }else {
                    snprintf(key, DEFAULT_KEY_LENGTH, "adinfo_pid_%s_%d_%d_%d",
                             mttyPid.c_str(),
                             selectCondition.dGeo,
                             selectCondition.mobileDevice,
                             selectCondition.pcOS);
                }
                CacheResult* cacheResult = cacheManager.get(key,CACHE_LEVEL_3_SIZE,[&mttyPid,this,&agent,&result,&selectCondition,simpleQuery](CacheResult& newCache){
                    try {
                        int cnt;
                        char buffer[LARGE_BUFFER_SIZE];
                        if(selectCondition.pAdplaceInfo==NULL) { //没有预设广告位信息
                            snprintf(buffer, LARGE_BUFFER_SIZE, this->dsl_query_adplace_pid, mttyPid.c_str());
                            rapidjson::Document adplace;
                            cnt = agent.search(ES_INDEX_SOLUTIONS, ES_DOCUMENT_ADPLACE, ES_FILTER_FORMAT, buffer,
                                                   adplace);
                            if (cnt == 0) {
                                LOG_DEBUG<<"adplace not found,mttyPid:"<<mttyPid;
                                return false;
                            }
                            rapidjson::Value &adplaceInfo = adplace["hits"]["hits"][0]["_source"];
                            //过滤条件绑定
                            if(simpleQuery){
                                bindSelectConditionSimple(adplaceInfo,selectCondition,this->dsl_query_adinfo_condition_simple,
                                                        buffer,
                                                        mttyPid.c_str());
                            } else{
                                bindSelectCondition(adplaceInfo, selectCondition, this->dsl_query_adinfo_condition,
                                                    buffer,
                                                    mttyPid.c_str());
                            }
                            cnt = agent.search(ES_INDEX_SOLUTIONS,ES_DOCUMENT_SOLBANADPLACE,ES_FILTER_FORMAT2,buffer,result);
                            if(cnt<=2) {
                                LOG_DEBUG<<"solution or banner not found,mttyPid:"<<mttyPid<<",selectCondition:"<<selectCondition.toString();
                                return false;
                            }
                            appendMtStatus(result);
                        }else{
                            bindSelectConditionMultiSize(selectCondition,this->dsl_query_adinfo_condition_multisize,buffer);
                            cnt = agent.search(ES_INDEX_SOLUTIONS,ES_DOCUMENT_SOLBANADPLACE,ES_FILTER_FORMAT2,buffer,result);
                            if(cnt<2){
                                return false;
                            }
                        }
                        std::string jsonResult = toJson(result);
                        if(jsonResult.length()>=newCache.size) {
                            DebugMessage("in queryAdInfoByPid result too large,pid:", mttyPid,",result size:",jsonResult.length());
                            return false;
                        }
                        memcpy(newCache.data, jsonResult.data(), jsonResult.length());
                        newCache.data[jsonResult.length()] = '\0';
                        newCache.size = jsonResult.length();
                        newCache.expireTime = getCurrentTimeStamp() + ADSELECT_CACHE_EXPIRE_TIME;
                        return true;
                    }catch(...){
                        return false;
                    }
                });
                if(cacheResult!=NULL && result.ObjectEmpty()){
                    parseJson((const char*)cacheResult->data,result);
                }else if(result.ObjectEmpty()||!hasMtStatus(result)){
                    DebugMessage("in queryAdInfoByMttyPid,failed to fetch valid adinfo for pid ",mttyPid);
                    return result;
                }
            }catch(std::exception& e){
                DebugMessageWithTime("exception in queryAdInfoByMttyPid,e:",e.what());
                return result;
            }
            return result["hits"]["hits"];
        }

        rapidjson::Value& AdSelectManager::queryAdInfoByAdxPid(int seqId,AdSelectCondition& selectCondition,rapidjson::Document& result,bool simpleQuery){
            try{
                const std::string& adxPid = selectCondition.adxpid;
                ElasticSearch& agent = getAvailableConnection(seqId);
                char key[DEFAULT_KEY_LENGTH];
                if(simpleQuery){
                    snprintf(key,DEFAULT_KEY_LENGTH,"adinfo_adxpid_%s",adxPid.c_str());
                }
                else {
                    snprintf(key, DEFAULT_KEY_LENGTH, "adinfo_adxpid_%s_%d_%d_%d",
                             adxPid.c_str(),
                             selectCondition.dGeo,
                             selectCondition.mobileDevice,
                             selectCondition.pcOS);
                }
                CacheResult* cacheResult = cacheManager.get(key,CACHE_LEVEL_3_SIZE,[&adxPid,this,&agent,&result,&selectCondition,simpleQuery](CacheResult& newCache){
                   try {
                       int cnt;
                       char buffer[LARGE_BUFFER_SIZE];
                       if(selectCondition.pAdplaceInfo==NULL) {
                           snprintf(buffer, LARGE_BUFFER_SIZE, this->dsl_query_adplace_adxpid, adxPid.c_str());
                           rapidjson::Document adplace;
                           cnt = agent.search(ES_INDEX_SOLUTIONS, ES_DOCUMENT_ADPLACE, ES_FILTER_FORMAT, buffer,
                                              adplace);
                           if (cnt == 0) {
                               LOG_DEBUG<<"adplace not found,adxPid:"<<adxPid;
                               return false;
                           }
                           rapidjson::Value &adplaceInfo = adplace["hits"]["hits"][0]["_source"];
                           std::string pid = to_string(adplaceInfo["pid"].GetInt());
                           //过滤条件绑定
                           if(simpleQuery){
                                bindSelectConditionSimple(adplaceInfo,selectCondition,this->dsl_query_adinfo_condition_simple,
                                                        buffer,
                                                        pid.c_str());
                           }
                           else {
                               bindSelectCondition(adplaceInfo, selectCondition, this->dsl_query_adinfo_condition,
                                                   buffer,
                                                   pid.c_str());
                           }
                           cnt = agent.search(ES_INDEX_SOLUTIONS, ES_DOCUMENT_SOLBANADPLACE, ES_FILTER_FORMAT2, buffer,
                                              result);
                           if (cnt <= 2) {
                               LOG_DEBUG<<"solution or banner not found,adxPid:"<<adxPid<<",selectCondition:"<<
                                                    selectCondition.toString();
                               return false;
                           }
                           appendMtStatus(result);
                       }else{
                           bindSelectConditionMultiSize(selectCondition,this->dsl_query_adinfo_condition_multisize,buffer);
                           cnt = agent.search(ES_INDEX_SOLUTIONS,ES_DOCUMENT_SOLBANADPLACE,ES_FILTER_FORMAT2,buffer,result);
                           if(cnt<2){
                               return false;
                           }
                       }
                       std::string jsonResult = toJson(result);
                       if(jsonResult.length()>=newCache.size) {
                           DebugMessage("in queryAdInfoByAdxPid result too large,adxpid:", adxPid,",result size:",jsonResult.length());
                           return false;
                       }
                       memcpy(newCache.data, jsonResult.data(), jsonResult.length());
                       newCache.data[jsonResult.length()] = '\0';
                       newCache.size = jsonResult.length();
                       newCache.expireTime = getCurrentTimeStamp() + ADSELECT_CACHE_EXPIRE_TIME;
                       return true;
                   }catch(...){
                       return false;
                   }
                });
                if(cacheResult!=NULL && result.ObjectEmpty()){
                    parseJson((const char*)cacheResult->data,result);
                }else if(result.ObjectEmpty()||!hasMtStatus(result)){
                    DebugMessage("in queryAdInfoByAdxPid,failed to fetch valid adinfo for adxpid ",adxPid);
                    return result;
                }
            }catch(std::exception& e){
                DebugMessageWithTime("exception in queryAdInfoByAdxPid,e:",e.what());
                return result;
            }
            return result["hits"]["hits"];
        }
    }
}