//
// Created by guoze.lin on 16/5/3.
//

#ifndef ADCORE_TANX_BIDDING_HANDLER_H
#define ADCORE_TANX_BIDDING_HANDLER_H

#include "protocol/base/abstract_bidding_handler.h"
#include "protocol/tanx/bidding.pb.h"

namespace protocol{
    namespace bidding{

        using namespace protocol::Tanx;

        class TanxBiddingHandler : public AbstractBiddingHandler {
        public:
            /**
             * 从Adx Bid Post请求数据中获取具体的请求信息
             */
            bool parseRequestData(const std::string& data);

            /**
             * 根据Bid 的相关信息对日志进行信息填充
             */
            void fillLogItem(protocol::log::LogItem& logItem);
            /**
             * 根据ADX的请求进行竞价匹配,决定是否接受这个流量,当不接受时将调用reject方法,同时设置isBidAccepted
             * @return: 是否接受流量,true接受,false不接受
             */
            bool match(INOUT HttpResponse& response);

        protected:
            /**
             * 不接受ADX的流量请求
             */
            void reject(INOUT HttpResponse& response);

        private:
            BidRequest bidRequest;
            BidResponse bidResponse;
        };

    }
}

#endif //ADCORE_TANX_BIDDING_HANDLER_H
