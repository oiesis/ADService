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

        bool TanxBiddingHandler::match(HttpResponse &response) {

            return false;
        }

        void TanxBiddingHandler::reject(HttpResponse &response) {

        }

    }
}