//
// Created by guoze.lin on 16/2/2.
//

#ifndef ADCORE_CLICK_SERVICE_H
#define ADCORE_CLICK_SERVICE_H

#include <memory>
#include "muduo/base/Logging.h"

#include "types.h"
#include "abstract_service.h"
#include "functions.h"
#include "core_executor.h"
#include "logpusher/log_pusher.h"
#include "core_http_server.h"
#include "constants.h"
#include "core_threadlocal_manager.h"
#include "core_ip_manager.h"
#include "adselect/core_adselect_manager.h"

namespace adservice{

    namespace corelogic {

        using namespace muduo::net;
        using namespace adservice::server;
        using namespace adservice::adselect;

        class CoreService;

        typedef std::shared_ptr<CoreService> CoreModule;
        typedef std::weak_ptr<CoreService> CoreModule_weak;

        /**
         * to future developers and maintainers:
         * 本来打算每一个进程负责一个模块分别开一个服务,但是被要求只能使用一个对外端口,而我不希望将过多的时间浪费在多进程通信请求分发上,
         * 所以所有业务逻辑模块都在单个进程中,重构成了现在这个样子.我为这样的设计感到羞愧和抱歉.
         */

        class CoreService : public adservice::server::AbstractService{
        public:
            typedef std::shared_ptr<adservice::server::CoreHttpServer> ServerPtr;
            static CoreModule getInstance();
        public:
            explicit CoreService():executor("mtty_core"),needRestart(false){
                ConfigManager::init();
                init();
            }
            CoreService(const CoreService&) = delete;

            virtual ~CoreService(){
                DebugMessage("in pid ",getpid()," coreservice module gone");
                adservice::server::ThreadLocalManager::getInstance().destroy();
                AdSelectManager::release();
                ConfigManager::exit();
                IpManager::destroy();
            }

            virtual void onRequest(const TcpConnectionPtr& conn,const HttpRequest& req, HttpResponse* resp);


            void init();

            virtual void start();

            void stop(){
                loop.quit();
                executor.stop();
                serviceLogger->stop();
                if(trackLogger.use_count()>0) {
                    trackLogger->stop();
                }
                adservice::log::LogPusher::removeLogger(MTTY_SERVICE_LOGGER);
            }

            adservice::log::LogPusherPtr& getLogger(){
                return serviceLogger;
            }

            adservice::log::LogPusherPtr& getTrackLogger(){
                return trackLogger;
            }

            bool isNeedRestart(){
                return needRestart;
            }

            void setNeedRestart(){
                needRestart = true;
            }

            adservice::server::Executor& getExecutor(){
                return executor;
            }

        private:
            bool needRestart;
            ServerPtr server;
            adservice::log::LogPusherPtr serviceLogger;
            adservice::log::LogPusherPtr trackLogger;
            muduo::net::EventLoop loop;
            adservice::server::Executor executor;
        };


     }

}


#endif //ADCORE_CLICK_SERVICE_H
