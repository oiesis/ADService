//
// Created by guoze.lin on 16/3/16.
//

#ifndef ADCORE_CORE_ADSELECT_MANAGER_H
#define ADCORE_CORE_ADSELECT_MANAGER_H

#include <string>
#include <exception>
#include "elasticsearch/elasticsearch.h"
#include "utility/utility.h"
#include "constants.h"
#include "atomic.h"
#include "core_config_manager.h"
#include "core_cache_manager.h"

namespace adservice{
    namespace adselect{

        using namespace adservice::utility::json;
        using namespace adservice::utility::file;
        using namespace adservice::server;
        using namespace adservice::cache;

        class AdSelectException : public std::exception{
        public:
            AdSelectException() _GLIBCXX_USE_NOEXCEPT {}
            AdSelectException(const char* str,int error) _GLIBCXX_USE_NOEXCEPT :message(str),errorCode(error){}
            const char* GetMsg() const { return message.c_str();}
            const char* what() const _GLIBCXX_USE_NOEXCEPT {return message.c_str();}
            int GetError() const {return errorCode;}
        private:
            int errorCode;
            std::string message;
        };

        static const int ADSELECT_MAX_CONNECTION = 24;

        class AdSelectManager{
        public:
            static AdSelectManager& getInstance(){
                ADSelectConfig* config = (ADSelectConfig*)ConfigManager::getInstance().get(CONFIG_ADSELECT);
                static AdSelectManager instance(config->entryNode);
                return instance;
            }
            static void release(){
                getInstance().destroy();
            }
        public:
            ~AdSelectManager(){
                destroy();
            }
            void destroy(){
                for(int i=0;i<ADSELECT_MAX_CONNECTION;i++){
                    if(agents[i]!=NULL)
                        delete agents[i];
                    agents[i]=NULL;
                }
            }


            /**
             * 根据创意ID查询创意内容
             */
            rapidjson::Value& queryCreativeById(int seqId,const std::string& bannerId,rapidjson::Document& result);

            /**
             * 根据PID查询广告信息
             */
            rapidjson::Value& queryAdInfoByPid(int seqId,const std::string& pid,rapidjson::Document& result,bool isAdxPid = false);

            /**
             * 根据Mtty PID找到合适的广告信息
             */
            rapidjson::Value& queryAdInfoByMttyPid(int seqId,const std::string& mttyPid,rapidjson::Document& result);

            /**
             * 根据ADX PID找到合适的广告信息
             */
            rapidjson::Value& queryAdInfoByAdxPid(int seqId,const std::string& adxPid,rapidjson::Document& result);

        private:
            AdSelectManager(const std::string& node){
                for(int i=0;i<ADSELECT_MAX_CONNECTION;i++) {
                    agents[i]=new ElasticSearch(node, true);
                }
                //todo
                loadFile(dsl_query_banner,ES_QUERY_CREATIVE);
                loadFile(dsl_query_adplace_pid,ES_QUERY_ADPLACE_BY_PID);
                loadFile(dsl_query_adplace_adxpid,ES_QUERY_ADPLACE_BY_ADXPID);
                loadFile(dsl_query_adinfo_condition,ES_QUERY_ADINFO_BY_CONDITION);
            }
        public:
            ElasticSearch& getAvailableConnection(int seqId) const{
                if(seqId<0||seqId>=ADSELECT_MAX_CONNECTION)
                    throw AdSelectException("in getAvailableConnection,seqId invalid",seqId);
                return *(agents[seqId]);
            }

        private:
            // adselect 缓存
            CacheManager cacheManager;
            // es 连接池
            ElasticSearch* agents[ADSELECT_MAX_CONNECTION];
            // 查询创意的DSL
            char dsl_query_banner[256];
            // 根据mtty pid查询广告位信息
            char dsl_query_adplace_pid[128];
            // 根据adxpid查询广告位信息
            char dsl_query_adplace_adxpid[128];
            // 根据广告位条件查询广告信息
            char dsl_query_adinfo_condition[8192];
        };

    }
}

#endif //ADCORE_CORE_ADSELECT_MANAGER_H
