//
// Created by guoze.lin on 16/5/3.
//

#include "baidu_bidding_handler.h"
#include "utility/utility.h"

namespace protocol {
    namespace bidding {

        using namespace adservice::utility::serialize;

        bool BaiduBiddingHandler::parseRequestData(const std::string& data){
            return false;
        }

        void BaiduBiddingHandler::fillLogItem(protocol::log::LogItem &logItem) {
        }

        bool BaiduBiddingHandler::filter(const BiddingFilterCallback& filterCb){
            isBidAccepted = false;
            return false;
        }

        void BaiduBiddingHandler::match(HttpResponse &response) {
        }

        void BaiduBiddingHandler::reject(HttpResponse &response) {
        }

    }
}