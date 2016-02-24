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
#include "net/log_pusher.h"
#include "core_http_server.h"
#include "constants.h"
#include "core_threadlocal_manager.h"

namespace adservice{

    namespace click {

        using namespace muduo::net;

        class ClickService;

        typedef std::shared_ptr<ClickService> ClickModule;
        typedef std::weak_ptr<ClickService> ClickModule_weak;



        class ClickService : public adservice::server::AbstractService{
        public:
            typedef std::shared_ptr<adservice::server::CoreHttpServer> ServerPtr;
            static ClickModule getInstance();
        public:
            explicit ClickService(int port,int threads,bool logRemote=true,int loggerThreads=10):executor("mtty_click"){
                init(port,threads,logRemote,loggerThreads);
            }
            ClickService(const ClickService&) = delete;

            virtual ~ClickService(){
                DebugMessage("in pid ",getpid()," clickservice module gone");
                adservice::server::ThreadLocalManager::getInstance().destroy();
            }

            virtual void onRequest(const TcpConnectionPtr& conn,const HttpRequest& req, bool isClose);


            void init(int port,int threads,bool logRemote=true,int loggerThreads = 10);

            virtual void start();

            void stop(){
                loop.quit();
                executor.stop();
                clickLogger->stop();
                adservice::log::LogPusher::removeLogger(CLICK_SERVICE_LOGGER);
            }

            adservice::log::LogPusherPtr& getLogger(){
                return clickLogger;
            }
        private:
            ServerPtr server;
            adservice::log::LogPusherPtr clickLogger;
            muduo::net::EventLoop loop;
            adservice::server::Executor executor;
        };


     }

}


#endif //ADCORE_CLICK_SERVICE_H
