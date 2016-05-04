//
// Created by guoze.lin on 16/5/3.
//

#ifndef ADCORE_ABSTRACT_BIDDING_HANDLER_H
#define ADCORE_ABSTRACT_BIDDING_HANDLER_H

#include <functional>
#include <string>
#include "protocol/log/log.h"
#include "muduo/net/http/HttpResponse.h"
#include "common/types.h"

namespace protocol{
    namespace bidding{

        using namespace muduo::net;

        typedef std::function<bool(const std::string&)> BiddingFilterCallback;

        /**
         * BiddingHandler主要处理协议层数据转换,屏蔽各个平台之间输入输出的差异
         * 实际上应该叫做BiddingDataAdapter
         */
        class AbstractBiddingHandler{
        public:

            AbstractBiddingHandler():isBidAccepted(false){}

            /**
             * 从Adx Bid Post请求数据中获取具体的请求信息
             */
            virtual bool parseRequestData(const std::string& data) = 0;

            /**
             * 根据Bid 的相关信息对日志进行信息填充
             */
            virtual void fillLogItem(protocol::log::LogItem& logItem){}

            /**
             * 根据ADX的请求进行竞价匹配,决定是否接受这个流量,同时设置isBidAccepted
             * @return: true接受流量,false不接受流量
             */
            virtual bool filter(const BiddingFilterCallback& filterCb){isBidAccepted=false;return false;}

            /**
             * 当接受流量时装配合适的输出
             */
            virtual void match(INOUT HttpResponse& response) = 0;

            /**
             * 不接受ADX的流量请求
             */
            virtual void reject(INOUT HttpResponse& response) = 0;

        protected:
            //最近一次匹配的结果
            bool isBidAccepted;
        };

    }
}

#endif //ADCORE_ABSTRACT_BIDDING_HANDLER_H
