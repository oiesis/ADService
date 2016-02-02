//
// Created by guoze.lin on 16/1/20.
//

#ifndef ADCORE_CORE_H
#define ADCORE_CORE_H

#include <fstream>
#include "types.h"
#include "utility.h"

namespace adservice{

    namespace server{

        typedef struct ServerConfig{
            int port;
            int threads;
            bool isDaemon;
        } *PServerConfig;

        const char* DEFAULT_CONFIG_PATH="conf/service.conf";
        const char* DEFAULT_DAEMON_FILE="adservice.pid";

        inline void  loadServerConfig(ServerConfig& config){
            using namespace utility::json;
            MessageWraper mw;
            assert(true==parseJsonFile(DEFAULT_CONFIG_PATH,mw));
            config.port = mw.getInt("port",8808);
            config.threads = mw.getInt("threads",24);
            config.isDaemon = mw.getBoolean("isDaemon",true);
        }

        bool daemon_init(const char *pidfile);

        class ADService final{
        public:
            ADService(){
                autoDetectEnv();
            }
            ADService(ServerConfig& config){
                port = config.port;
                threadNum = config.threads;
                if(config.isDaemon){
                    daemonFile = DEFAULT_DAEMON_FILE;
                }

            }
            ~ADService(){
            }
            void start(){
                adservice_init();
                adservice_start();
                adservice_exit();
            }
        private:
            void autoDetectEnv();
            void dosignals();
            void adservice_init();
            void adservice_start();
            void adservice_exit();
        private:
            unsigned short port;
            int threadNum;
            const char* daemonFile;
        };
    }
}

#endif
