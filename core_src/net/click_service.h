//
// Created by guoze.lin on 16/2/2.
//

#ifndef ADCORE_CLICK_SERVICE_H
#define ADCORE_CLICK_SERVICE_H

#include <memory>
#include "muduo/base/Logging.h"
#include "muduo/net/EventLoop.h"
#include "muduo/net/TcpServer.h"
#include "core_cgi.h"
#include "types.h"

namespace adservice{

    namespace click {

        using namespace muduo::net;
        using namespace adservice::net;
        using namespace server;

        class ClickService : public AbstractService{
        public:
            explicit void ClickService(int port,int threads){
                init(port,threads);
            }
            ClickService(const ClickService&) = delete;

            virtual ~ClickService(){}

            virtual void onRequest(const TcpConnectionPtr& conn,
                           FastCgiCodec::ParamMap& params,
                           Buffer* in);

            virtual void onConnection(const TcpConnectionPtr& conn);

            inline void init(int port,int threads){
                InetAddress addr(static_cast<uint16_t>(port));
                server = std::make_shared(&loop,addr,"mtty::click_service");
                server->setConnectionCallback(std::bind(onConnection,this,std::placeholders::_1));
                server->setThreadNum(threads);
            }

            virtual void start();

            void stop(){
                loop.quit();
            }

        private:
            typedef std::shared_ptr<TcpServer> ServerPtr;
            ServerPtr server;
            muduo::net::EventLoop loop;
        };

        typedef std::shared_ptr<ClickService> ClickModule;
        typedef std::weak_ptr<ClickService> ClickModule_weak;
     }

}


#endif //ADCORE_CLICK_SERVICE_H
