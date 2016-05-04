//
// Created by guoze.lin on 16/5/3.
//

#ifndef ADCORE_ABSTRACT_BIDDING_HANDLER_H
#define ADCORE_ABSTRACT_BIDDING_HANDLER_H

#include "protocol/log/log.h"
#include "muduo/net/http/HttpResponse.h"
#include "common/types.h"

namespace protocol{
    namespace bidding{

        using namespace muduo::net;

        class AbstractBiddingHandler{
        public:
            /**
             * 从Adx Bid Post请求数据中获取具体的请求信息
             */
            virtual bool parseRequestData(const std::string& data) = 0;

            /**
             * 根据Bid 的相关信息对日志进行信息填充
             */
            virtual void fillLogItem(protocol::log::LogItem& logItem){}

            /**
             * 根据ADX的请求进行竞价匹配,决定是否接受这个流量,当不接受时将调用reject方法,同时设置isBidAccepted
             * @return: 是否接受流量,true接受,false不接受
             */
            virtual bool match(INOUT HttpResponse& response) = 0;

        protected:
            /**
             * 不接受ADX的流量请求
             */
            virtual void reject(INOUT HttpResponse& response) = 0;

            bool isBidAccepted;
        };

    }
}

#endif //ADCORE_ABSTRACT_BIDDING_HANDLER_H
