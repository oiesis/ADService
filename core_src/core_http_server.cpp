//
// Created by guoze.lin on 16/2/14.
//

#include "core_http_server.h"


namespace adservice{
    namespace server{

        using namespace muduo::net;

        void CoreHttpServer::onRequest(const TcpConnectionPtr& conn, const HttpRequest& req){
          const muduo::string& connection = req.getHeader("Connection");
          bool close = connection == "close" ||
            (req.getVersion() == HttpRequest::kHttp10 && connection != "Keep-Alive");
          httpCallback_(conn,req, &close);
        }

    }
}