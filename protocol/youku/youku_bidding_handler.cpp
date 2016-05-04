//
// Created by guoze.lin on 16/5/3.
//

#include "youku_bidding_handler.h"
#include "utility/utility.h"

namespace protocol {
    namespace bidding {

        using namespace adservice::utility::serialize;

        bool YoukuBiddingHandler::parseRequestData(const std::string& data){
            return false;
        }

        void YoukuBiddingHandler::fillLogItem(protocol::log::LogItem &logItem) {
        }

        bool YoukuBiddingHandler::filter(const BiddingFilterCallback& filterCb){
            isBidAccepted = false;
            return false;
        }

        void YoukuBiddingHandler::match(HttpResponse &response) {
        }

        void YoukuBiddingHandler::reject(HttpResponse &response) {
        }

    }
}