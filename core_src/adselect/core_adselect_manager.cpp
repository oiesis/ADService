//
// Created by guoze.lin on 16/3/16.
//

#include <functional>
#include "core_adselect_manager.h"
#include "core_cache_manager.h"
#include "utility/json.h"
#include "utility/mttytime.h"
#include "constants.h"
#include "atomic.h"
#include "common/types.h"

namespace adservice{
    namespace adselect{

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
            result.AddMember(CACHE_RESULT_MTSTATUS,MakeStringConstValue("ok"), result.GetAllocator());
        }

        /**
         * 检查结果是否有成功标记
         */
        inline bool hasMtStatus(rapidjson::Document& result){
            return result.HasMember(CACHE_RESULT_MTSTATUS);
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
                                                                  rapidjson::Document &result, bool isAdxPid) {
            if(isAdxPid){
                return queryAdInfoByAdxPid(seqId,condition,result);
            }else{
                return queryAdInfoByMttyPid(seqId,condition,result);
            }

        }

        int countryGeo(int geo){
            return geo - (geo%AREACODE_MARGIN);
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
            bindCondition.flowType = adplaceInfo["flowtype"].GetInt();
            bindCondition.width = adplaceInfo["width"].GetInt();
            bindCondition.height = adplaceInfo["height"].GetInt();
            //时间定点过滤
            std::string dHour = adSelectTimeCodeUtc();
            bindCondition.dHour = dHour;
            //国家通投编码
            int dCountryGeo = countryGeo(bindCondition.dGeo);
            sprintf(output,cTemplate,pid,
                    bindCondition.mediaType,
                    pid,
                    bindCondition.adplaceType,
                    bindCondition.displayNumber,
                    bindCondition.flowType,
                    bindCondition.dHour.data(),
                    bindCondition.dGeo,dCountryGeo,
                    bindCondition.width,
                    bindCondition.height,
                    supportBanner,
                    bindCondition.width,
                    bindCondition.height,
                    supportBanner,
                    bindCondition.mediaType,
                    pid,
                    bindCondition.adplaceType,
                    bindCondition.displayNumber,
                    bindCondition.flowType,
                    bindCondition.dHour.data(),
                    bindCondition.dGeo,dCountryGeo
            );
        }

        rapidjson::Value& AdSelectManager::queryAdInfoByMttyPid(int seqId,AdSelectCondition& selectCondition,rapidjson::Document& result){
            try{
                const std::string& mttyPid = selectCondition.mttyPid;
                ElasticSearch& agent = getAvailableConnection(seqId);
                char key[DEFAULT_KEY_LENGTH];
                snprintf(key,DEFAULT_KEY_LENGTH,"adinfo_pid_%s_%s",mttyPid.c_str(),selectCondition.areaCode.data());
                CacheResult* cacheResult = cacheManager.get(key,CACHE_LEVEL_3_SIZE,[&mttyPid,this,&agent,&result,&selectCondition](CacheResult& newCache){
                    try {
                        char buffer[LARGE_BUFFER_SIZE];
                        snprintf(buffer,LARGE_BUFFER_SIZE,this->dsl_query_adplace_pid, mttyPid.c_str());
                        rapidjson::Document adplace;
                        int cnt = agent.search(ES_INDEX_SOLUTIONS, ES_DOCUMENT_ADPLACE, ES_FILTER_FORMAT, buffer,
                                               adplace);
                        if (cnt == 0)
                            return false;
                        rapidjson::Value& adplaceInfo = adplace["hits"]["hits"][0]["_source"];
                        //过滤条件绑定
                        bindSelectCondition(adplaceInfo,selectCondition,this->dsl_query_adinfo_condition,buffer,mttyPid.c_str());
                        cnt = agent.search(ES_INDEX_SOLUTIONS,ES_DOCUMENT_SOLBANADPLACE,ES_FILTER_FORMAT2,buffer,result);
                        if(cnt<2)
                            return false;
                        result.AddMember("adplace",adplaceInfo,result.GetAllocator());
                        appendMtStatus(result);
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

        rapidjson::Value& AdSelectManager::queryAdInfoByAdxPid(int seqId,AdSelectCondition& selectCondition,rapidjson::Document& result){
            try{
                const std::string& adxPid = selectCondition.adxpid;
                ElasticSearch& agent = getAvailableConnection(seqId);
                char key[DEFAULT_KEY_LENGTH];
                snprintf(key,DEFAULT_KEY_LENGTH,"adinfo_adxpid_%s_%s",adxPid.c_str(),selectCondition.areaCode.data());
                CacheResult* cacheResult = cacheManager.get(key,CACHE_LEVEL_3_SIZE,[&adxPid,this,&agent,&result,&selectCondition](CacheResult& newCache){
                   try {
                       char buffer[LARGE_BUFFER_SIZE];
                       snprintf(buffer,LARGE_BUFFER_SIZE,this->dsl_query_adplace_adxpid, adxPid.c_str());
                       rapidjson::Document adplace;
                       int cnt = agent.search(ES_INDEX_SOLUTIONS, ES_DOCUMENT_ADPLACE, ES_FILTER_FORMAT, buffer,
                                              adplace);
                       if (cnt == 0)
                           return false;
                       rapidjson::Value& adplaceInfo = adplace["hits"]["hits"][0]["_source"];
                       std::string pid = to_string(adplaceInfo["pid"].GetInt());
                       //过滤条件绑定
                       bindSelectCondition(adplaceInfo,selectCondition,this->dsl_query_adinfo_condition,buffer,pid.c_str());
                       cnt = agent.search(ES_INDEX_SOLUTIONS,ES_DOCUMENT_SOLBANADPLACE,ES_FILTER_FORMAT2,buffer,result);
                       if(cnt<2)
                           return false;
                       result.AddMember("adplace",adplaceInfo,result.GetAllocator());
                       appendMtStatus(result);
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