//
// Created by guoze.lin on 16/2/14. Happy Valentines' Day!
//

#ifndef ADCORE_CORE_HTTP_SERVER_H
#define ADCORE_CORE_HTTP_SERVER_H

#include <functional>
#include "muduo/net/EventLoop.h"
#include "muduo/net/http/HttpServer.h"
#include "muduo/net/http/HttpRequest.h"
#include "muduo/net/http/HttpResponse.h"
#include "muduo/base/tbb/concurrent_hash_map.h"
#include "common/constants.h"

namespace adservice{
    namespace server{

        using namespace muduo::net;

        static const int MAX_CONNECTION = 5000;

        typedef boost::weak_ptr<muduo::net::TcpConnection> WeakTcpConnectionPtr;

        typedef tbb::concurrent_hash_map<muduo::string,WeakTcpConnectionPtr> ConcurrentWeakConnMap;
        typedef tbb::concurrent_hash_map<muduo::string,WeakTcpConnectionPtr>::accessor ConcurrentWeakMapAccessor;

        class CoreHttpServer : public HttpServer{
        public:
            typedef std::function<void(const TcpConnectionPtr&,const HttpRequest&,
                                       bool isClose)> HttpCallback;
            CoreHttpServer(EventLoop* loop,
                           const InetAddress& listenAddr,
                           const muduo::string& name,
                           bool isCheckIdleConn = false,
                           int connIdleSecond = HTTP_IDLE_MAX_SECOND,
                           TcpServer::Option option = TcpServer::kNoReusePort
                           ):HttpServer(loop,listenAddr,name,option),idleCheck(isCheckIdleConn),maxIdleSecond(connIdleSecond){
                             connectionCnt = 0;
                             server_.setConnectionCallback(std::bind(&CoreHttpServer::onConnection, this, std::placeholders::_1));
                             loop->runEvery(5,std::bind(&CoreHttpServer::onTimer,this));
                           };
            ~CoreHttpServer(){}

            void setHttpCallback(const HttpCallback& cb)
            {
                httpCallback_ = cb;
            }
        protected:
            virtual void onConnection(const TcpConnectionPtr& conn);
            virtual void onRequest(const TcpConnectionPtr&, const HttpRequest&);
            void onTimer();
            HttpCallback httpCallback_;
            int connectionCnt;
            int maxIdleSecond;
            bool idleCheck;
            ConcurrentWeakConnMap weakConnMap;
        };

    }
}

#endif //ADCORE_CORE_HTTP_SERVER_H
