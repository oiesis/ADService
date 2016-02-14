//
// Created by guoze.lin on 16/2/2.
//

#ifndef ADCORE_CLICK_SERVICE_H
#define ADCORE_CLICK_SERVICE_H

#include <memory>
#include "muduo/base/Logging.h"
#include "muduo/net/EventLoop.h"
#include "muduo/net/http/HttpServer.h"
#include "muduo/net/http/HttpRequest.h"
#include "muduo/net/http/HttpResponse.h"

#include "types.h"
#include "abstract_service.h"
#include "functions.h"
#include "core_executor.h"
#include "net/log_pusher.h"

namespace adservice{

    namespace click {

        using namespace muduo::net;

        class ClickService : public adservice::server::AbstractService{
        public:
            typedef std::shared_ptr<HttpServer> ServerPtr;
        public:
            explicit ClickService(int port,int threads):executor("mtty_click"){
                init(port,threads);
            }
            ClickService(const ClickService&) = delete;

            virtual ~ClickService(){
                DebugMessage("in pid ",getpid()," clickservice module gone");
            }

            virtual void onRequest(const HttpRequest& req, HttpResponse* resp);


            void init(int port,int threads);

            virtual void start();

            void stop(){
                loop.quit();
                executor.stop();
                clickLogger.stop();
            }

            adservice::log::LogPusher& getLogger(){
                return clickLogger;
            }

        private:
            ServerPtr server;
            adservice::log::LogPusher clickLogger;
            muduo::net::EventLoop loop;
            adservice::server::Executor executor;
        };

        typedef std::shared_ptr<ClickService> ClickModule;
        typedef std::weak_ptr<ClickService> ClickModule_weak;
     }

}


#endif //ADCORE_CLICK_SERVICE_H
