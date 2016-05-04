//
// Created by guoze.lin on 16/5/3.
//

#include "tanx_bidding_handler.h"
#include "utility.h"

namespace protocol {
    namespace bidding {

        using namespace adservice::utility::serialize;

        bool TanxBiddingHandler::parseRequestData(const std::string& data){
            return getProtoBufObject(bidRequest,data);
        }

        void TanxBiddingHandler::fillLogItem(protocol::log::LogItem &logItem) {
        }

        bool TanxBiddingHandler::filter(const BiddingFilterCallback& filterCb){
            isBidAccepted = false;
            return false;
        }

        void TanxBiddingHandler::match(HttpResponse &response) {
        }

        void TanxBiddingHandler::reject(HttpResponse &response) {
        }

    }
}