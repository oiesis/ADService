//
// Created by guoze.lin on 16/2/2.
//

#ifndef ADCORE_CORE_CGI_H
#define ADCORE_CORE_CGI_H


#include <map>
#include <functional>
#include "muduo/net/TcpConnection.h"

namespace adservice {

    namespace net {
        using muduo::string;

        // one FastCgiCodec per TcpConnection
        // both lighttpd and nginx do not implement multiplexing,
        // so there is no concurrent requests of one connection.
        class FastCgiCodec {
        public:
            typedef std::map<string, string> ParamMap;
            typedef std::function<void(const muduo::net::TcpConnectionPtr &conn, ParamMap &,
                                       muduo::net::Buffer *)> Callback;

            explicit FastCgiCodec(const Callback &cb)
                    : cb_(cb),
                      gotRequest_(false),
                      keepConn_(false) {
            }

            FastCgiCodec(const FastCgiCodec &) = delete;

            void onMessage(const muduo::net::TcpConnectionPtr &conn,
                           muduo::net::Buffer *buf,
                           muduo::Timestamp receiveTime) {
                parseRequest(buf);
                if (gotRequest_) {
                    cb_(conn, params_, &stdin_);
                    stdin_.retrieveAll();
                    paramsStream_.retrieveAll();
                    params_.clear();
                    gotRequest_ = false;
                    if (!keepConn_) {
                        conn->shutdown();
                    }
                }
            }

            static void respond(muduo::net::Buffer *response);

        private:
            struct RecordHeader;

            bool parseRequest(muduo::net::Buffer *buf);

            bool onBeginRequest(const RecordHeader &header, const muduo::net::Buffer *buf);

            void onStdin(const char *content, uint16_t length);

            bool onParams(const char *content, uint16_t length);

            bool parseAllParams();

            uint32_t readLen();

            static void endStdout(muduo::net::Buffer *buf);

            static void endRequest(muduo::net::Buffer *buf);

            Callback cb_;
            bool gotRequest_;
            bool keepConn_;
            muduo::net::Buffer stdin_;
            muduo::net::Buffer paramsStream_;
            ParamMap params_;

            const static unsigned kRecordHeader;
        };
    }

}

#endif //ADCORE_CORE_CGI_H
