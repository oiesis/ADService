//
// Created by guoze.lin on 16/2/14. Happy Valentines' Day!
//

#ifndef ADCORE_CORE_HTTP_SERVER_H
#define ADCORE_CORE_HTTP_SERVER_H

#include "muduo/net/EventLoop.h"
#include "muduo/net/http/HttpServer.h"
#include "muduo/net/http/HttpRequest.h"
#include "muduo/net/http/HttpResponse.h"

namespace adservice{
    namespace server{

        using namespace muduo::net;

        class CoreHttpServer : public HttpServer{
        public:
            typedef std::function<void(const TcpConnectionPtr&,const HttpRequest&,
                                       bool isClose)> HttpCallback;
            CoreHttpServer(EventLoop* loop,
                           const InetAddress& listenAddr,
                           const muduo::string& name,
                           TcpServer::Option option = TcpServer::kNoReusePort):HttpServer(loop,listenAddr,name,option){};
            ~CoreHttpServer(){}

            void setHttpCallback(const HttpCallback& cb)
            {
                httpCallback_ = cb;
            }
        protected:
            virtual void onRequest(const TcpConnectionPtr&, const HttpRequest&);
            HttpCallback httpCallback_;

        };

    }
}

#endif //ADCORE_CORE_HTTP_SERVER_H
