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
            explicit ClickService(int port,int threads,bool logRemote=true):executor("mtty_click"){
                init(port,threads,logRemote);
            }
            ClickService(const ClickService&) = delete;

            virtual ~ClickService(){
                DebugMessage("in pid ",getpid()," clickservice module gone");
            }

            virtual void onRequest(const TcpConnectionPtr& conn,const HttpRequest& req, bool isClose);


            void init(int port,int threads,bool logRemote=true);

            virtual void start();

            void stop(){
                loop.quit();
                executor.stop();
                clickLogger->stop();
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
