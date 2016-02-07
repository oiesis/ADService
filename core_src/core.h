//
// Created by guoze.lin on 16/1/20.
//

#ifndef ADCORE_CORE_H
#define ADCORE_CORE_H

#include <fstream>
#include <sys/types.h>
#include <memory>
#include "types.h"
#include "atomic.h"
#include "utility.h"
#include "net/click_service.h"
#include "functions.h"

#ifdef UNIT_TEST
int launch_service();
#endif

namespace adservice{

    namespace server{

        using namespace click;

        const int MAX_MODULE = 10;

        const int MTTY_EXIT_SUCCESS = 2000;

        enum MODULE_TYPE : char{
            MODULE_NON = -1,
            MODULE_FIRST = 0,
            MODULE_CLICK = 0,
            MODULE_SHOW,
            MODULE_BIDDING,
            MODULE_LAST = 9
        };

        typedef struct ServerConfig{
            int clickPort;
            int clickThreads;
            bool runClick;
            bool isDaemon;
        } *PServerConfig;

        static const char* DEFAULT_CONFIG_PATH="../conf/service.conf";
        static const char* DEFAULT_DAEMON_FILE="adservice.pid";

        inline void  loadServerConfig(ServerConfig& config,const char* path = DEFAULT_CONFIG_PATH){
            using namespace utility::json;
	    MessageWraper mw;
	    bool bSuccess = parseJsonFile(path,mw);
            if(!bSuccess){
	    	DebugMessage("failed to read json file");
	    }
            config.clickPort = mw.getInt("click_port",8808);
            config.clickThreads = mw.getInt("click_threads",24);
            config.runClick = mw.getBoolean("load_click", false);
            config.isDaemon = mw.getBoolean("isDaemon",true);
        }

        bool daemon_init(const char *pidfile);


        class ADService;
        typedef std::shared_ptr<ADService> ADServicePtr;

        class ADService final{
        private:
            static volatile int instanceCnt;
            static ADServicePtr instance;
        public:
            static void initClassVar(){
                instance = nullptr;
                instanceCnt = 0;
            }
            static ADServicePtr getInstance(){
                if(instance.use_count()==0){ //use_count 为 0,意味着没有初始化
                    if(ATOM_INC(&instanceCnt)==1) {
                        instance = std::make_shared<ADService>();
                    }else{
                        assert(false); //理论上这里应该设一个屏障,然后返回正确的对象
                    }
                }
                return instance;
            }
	    explicit ADService(){
                autoDetectEnv();
                running = true;
                if(instanceCnt>1){
                    throw "more than one instance of ADService";
                }
            }
            void initWithConfig(ServerConfig& config){
                this->config = config;
            }
            ~ADService(){
                DebugMessage("in pid ",getpid()," adservice gone");
            }
            void start(){
                adservice_init();
                adservice_start();
                adservice_exit();
            }
            void stop(){
                running = false;
            }
            void reLaunchModule(pid_t pid);

        private:
            void autoDetectEnv(){};
            void dosignals();
            void adservice_init();
            void adservice_start();
            void adservice_exit();
            void launchModule(MODULE_TYPE mt);
            MODULE_TYPE moduleTypeOfPid(pid_t pid);
        private:
            ServerConfig config;
            const char* daemonFile;
            pid_t modules[MAX_MODULE+1];
            bool running;
        };
    }
}

#endif
