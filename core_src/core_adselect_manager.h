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

namespace adservice{
    namespace adselect{

        using namespace adservice::utility::json;
        using namespace adservice::utility::file;
        using namespace adservice::server;

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
                if(!agents.empty()){
                    typedef std::vector<ElasticSearch*>::iterator Iter;
                    for(Iter iter = agents.begin();iter!=agents.end();iter++){
                        delete (*iter);
                    }
                    agents.clear();
                }
            }


            /**
             * 根据创意ID查询创意内容
             */
            rapidjson::Value& queryCreativeById(const std::string& bannerId,rapidjson::Document& result);

        private:
            AdSelectManager(const std::string& node){
                agents.reserve(ADSELECT_MAX_CONNECTION);
                for(int i=0;i<ADSELECT_MAX_CONNECTION;i++) {
                    agents.push_back(new ElasticSearch(node, true));
                    roundTable[i] = 0;
                }
                cursor = 0;
                //todo
                loadFile(dsl_query_banner,ES_QUERY_CREATIVE);
            }

            int getAvailableConnection(){
                int cur = cursor;
                while(true) {
                    if (ATOM_CAS(&roundTable[cur], 0, 1)) {
                        return cur;
                    }else{
                        cur = ATOM_INC(&cursor)%ADSELECT_MAX_CONNECTION;
                    }
                }
            }

        private:
            int roundTable[ADSELECT_MAX_CONNECTION];
            std::vector<ElasticSearch*> agents;
            int cursor;
            // 查询创意的DSL
            char dsl_query_banner[128];
        };

    }
}

#endif //ADCORE_CORE_ADSELECT_MANAGER_H
