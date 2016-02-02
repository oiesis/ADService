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

        class ClickService {
        public:
            explicit void ClickService(int port,int threads){
                init(port,threads);
            }
            ClickService(const ClickService&) = delete;

            void onRequest(const TcpConnectionPtr& conn,
                           FastCgiCodec::ParamMap& params,
                           Buffer* in);
            void onConnection(const TcpConnectionPtr& conn);

            void init(int port,int threads);

            void start();

        private:
            typedef share_ptr<TcpServer> ServerPtr;
            ServerPtr server;
        };
    }

}


#endif //ADCORE_CLICK_SERVICE_H
