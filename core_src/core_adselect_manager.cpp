//
// Created by guoze.lin on 16/3/16.
//

#include <functional>
#include "core_adselect_manager.h"
#include "core_cache_manager.h"
#include "utility/json.h"
#include "constants.h"
#include "atomic.h"

namespace adservice{
    namespace adselect{

        using namespace adservice::cache;
        using namespace adservice::utility::json;

        static const int DEFAULT_KEY_LENGTH = 56;
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
                char buffer[256];
                sprintf(buffer, dsl_query_banner, std::stoi(bannerId));
                cnt = agent.search(ES_INDEX_SOLUTIONS, ES_DOCUMENT_BANNER, ES_FILTER_FORMAT, buffer, result);
            }catch(std::exception& e){
                DebugMessageWithTime("exception in queryCreativeById,e:",e.what());
            }
            if(cnt!=1) {
                DebugMessageWithTime("error occured in queryCreativeById,document id:",bannerId);
                //throw AdSelectException("not exactly one docuemnts fetched in queryCreativeById", -1);
                return result["hits"];
            }
            return result["hits"]["hits"][0]["_source"];
        }

        rapidjson::Value& AdSelectManager::queryCreativeByIdCache(int seqId,const std::string& bannerId,rapidjson::Document& result){
            try{
                ElasticSearch& agent = getAvailableConnection(seqId);
                char key[DEFAULT_KEY_LENGTH];
                sprintf(key,"creative_id_%s",bannerId.c_str());
                CacheResult* cacheResult = cacheManager.get(key,CACHE_LEVEL_1_SIZE,[&bannerId,this,&agent,&result](CacheResult& newCache){
                    int cnt = 0;
                    try {
                        char buffer[256];
                        sprintf(buffer, this->dsl_query_banner, std::stoi(bannerId));
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
                if(cacheResult!=NULL && result.Empty()){
                    parseJson((const char*)cacheResult->data,result);
                }else if(result.Empty()||!hasMtStatus(result)){
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
            if(isAdxPid){
                return queryAdInfoByAdxPid(seqId,pid,result);
            }else{
                return queryAdInfoByMttyPid(seqId,pid,result);
            }
        }


        rapidjson::Value& AdSelectManager::queryAdInfoByMttyPid(int seqId,const std::string& mttyPid,rapidjson::Document& result){
            try{
                ElasticSearch& agent = getAvailableConnection(seqId);
                char key[DEFAULT_KEY_LENGTH];
                sprintf(key,"adinfo_pid_%s",mttyPid.c_str());
                CacheResult* cacheResult = cacheManager.get(key,CACHE_LEVEL_3_SIZE,[&mttyPid,this,&agent,&result](CacheResult& newCache){
                    try {
                        char buffer[LARGE_BUFFER_SIZE];
                        sprintf(buffer,this->dsl_query_adplace_pid, mttyPid.c_str());
                        rapidjson::Document adplace;
                        int cnt = agent.search(ES_INDEX_SOLUTIONS, ES_DOCUMENT_ADPLACE, ES_FILTER_FORMAT, buffer,
                                               adplace);
                        if (cnt == 0)
                            return false;
                        rapidjson::Value& adplaceInfo = adplace["hits"]["hits"][0]["_source"];
                        const char* supportBanner = adplaceInfo["supportbanner"].IsNull()?DEFAULT_SUPPORT_BANNERTYPE:adplaceInfo["supportbanner"].GetString();
                        sprintf(buffer,this->dsl_query_adinfo_condition,mttyPid.c_str(),
                                adplaceInfo["media_type"].GetInt(),
                                mttyPid.c_str(),
                                adplaceInfo["adplacetype"].GetInt(),
                                adplaceInfo["displaynumber"].GetInt(),
                                adplaceInfo["flowtype"].GetInt(),
                                adplaceInfo["width"].GetInt(),
                                adplaceInfo["height"].GetInt(),
                                supportBanner,
                                adplaceInfo["width"].GetInt(),
                                adplaceInfo["height"].GetInt(),
                                supportBanner,
                                adplaceInfo["media_type"].GetInt(),
                                mttyPid.c_str(),
                                adplaceInfo["adplacetype"].GetInt(),
                                adplaceInfo["displaynumber"].GetInt(),
                                adplaceInfo["flowtype"].GetInt()
                        );
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
                if(cacheResult!=NULL && result.Empty()){
                    parseJson((const char*)cacheResult->data,result);
                }else if(result.Empty()||!hasMtStatus(result)){
                    DebugMessage("in queryAdInfoByMttyPid,failed to fetch valid adinfo for pid ",mttyPid);
                    return result;
                }
            }catch(std::exception& e){
                DebugMessageWithTime("exception in queryAdInfoByMttyPid,e:",e.what());
                return result;
            }
            return result["hits"]["hits"];
        }

        rapidjson::Value& AdSelectManager::queryAdInfoByAdxPid(int seqId,const std::string& adxPid,rapidjson::Document& result){
            try{
                ElasticSearch& agent = getAvailableConnection(seqId);
                char key[DEFAULT_KEY_LENGTH];
                sprintf(key,"adinfo_adxpid_%s",adxPid.c_str());
                CacheResult* cacheResult = cacheManager.get(key,CACHE_LEVEL_3_SIZE,[&adxPid,this,&agent,&result](CacheResult& newCache){
                   try {
                       char buffer[LARGE_BUFFER_SIZE];
                       sprintf(buffer,this->dsl_query_adplace_adxpid, adxPid.c_str());
                       rapidjson::Document adplace;
                       int cnt = agent.search(ES_INDEX_SOLUTIONS, ES_DOCUMENT_ADPLACE, ES_FILTER_FORMAT, buffer,
                                              adplace);
                       if (cnt == 0)
                           return false;
                       rapidjson::Value& adplaceInfo = adplace["hits"]["hits"][0]["_source"];
                       const char* supportBanner = adplaceInfo["supportbanner"].IsNull()?DEFAULT_SUPPORT_BANNERTYPE:adplaceInfo["supportbanner"].GetString();
                       std::string pid = to_string(adplaceInfo["pid"].GetInt());
                       sprintf(buffer,this->dsl_query_adinfo_condition,pid.c_str(),
                               adplaceInfo["media_type"].GetInt(),
                               pid.c_str(),
                               adplaceInfo["adplacetype"].GetInt(),
                               adplaceInfo["displaynumber"].GetInt(),
                               adplaceInfo["flowtype"].GetInt(),
                               adplaceInfo["width"].GetInt(),
                               adplaceInfo["height"].GetInt(),
                               supportBanner,
                               adplaceInfo["width"].GetInt(),
                               adplaceInfo["height"].GetInt(),
                               supportBanner,
                               adplaceInfo["media_type"].GetInt(),
                               pid.c_str(),
                               adplaceInfo["adplacetype"].GetInt(),
                               adplaceInfo["displaynumber"].GetInt(),
                               adplaceInfo["flowtype"].GetInt()
                       );
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
                if(cacheResult!=NULL && result.Empty()){
                    parseJson((const char*)cacheResult->data,result);
                }else if(result.Empty()||!hasMtStatus(result)){
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