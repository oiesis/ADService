//
// Created by guoze.lin on 16/2/2.
//

#include "click_service.h"

namespace adservice{

    namespace click{

        void ClickService::start(){
            server->start();
            loop.loop();
        }

        void ClickService::onRequest(const TcpConnectionPtr& conn,
                       FastCgiCodec::ParamMap& params,
                       Buffer* in)
        {
            string uri = params["REQUEST_URI"];
            LOG_INFO << conn->name() << ": " << uri;

            for (FastCgiCodec::ParamMap::const_iterator it = params.begin();
                 it != params.end(); ++it)
            {
                LOG_DEBUG << it->first << " = " << it->second;
            }
            if (in->readableBytes() > 0)
            LOG_DEBUG << "stdin " << in->retrieveAllAsString();
            Buffer response;
            response.append("Context-Type: text/plain\r\n\r\n");
            if (uri.size() == kCells + kPath.size() && uri.find(kPath) == 0)
            {
                response.append(solveSudoku(uri.substr(kPath.size())));
            }
            else
            {
                // FIXME: set http status code 400
                response.append("bad request");
            }

            FastCgiCodec::respond(&response);
            conn->send(&response);
        }

        void ClickService::onConnection(const TcpConnectionPtr& conn)
        {
            if (conn->connected())
            {
                typedef boost::shared_ptr<FastCgiCodec> CodecPtr;
                CodecPtr codec(new FastCgiCodec(onRequest));
                conn->setContext(codec);
                conn->setMessageCallback(
                        std::bind(&FastCgiCodec::onMessage, codec, _1, _2, _3));
                conn->setTcpNoDelay(true);
            }
        }
    }

}