//
// Created by guoze.lin on 16/5/3.
//

#include "tanx_bidding_handler.h"
#include "utility.h"

namespace protocol {
    namespace bidding {

        using namespace protocol::Tanx;
        using namespace adservice::utility::serialize;

        inline int max(const int& a,const int& b){
            return a>b?a:b;
        }

        bool TanxBiddingHandler::parseRequestData(const std::string& data){
            bidRequest.Clear();
            return getProtoBufObject(bidRequest,data);
        }

        void TanxBiddingHandler::fillLogItem(protocol::log::LogItem &logItem) {
            logItem.reqStatus = 200;
            logItem.userAgent = bidRequest.user_agent();
            logItem.ipInfo.proxy = bidRequest.ip();
            if(isBidAccepted){
                if(bidRequest.has_mobile()){
                    const BidRequest_Mobile& mobile = bidRequest.mobile();
                    if(mobile.has_device()){
                        const BidRequest_Mobile_Device& device = mobile.device();
                        logItem.deviceInfo = device.DebugString();
                    }
                }
                logItem.adInfo.advId = adInfo.advId;
                logItem.adInfo.adxid = adInfo.adxid;
                logItem.adInfo.adxpid = adInfo.adxpid;
                logItem.adInfo.adxuid = adInfo.adxuid;
                logItem.adInfo.bannerId = adInfo.bannerId;
                logItem.adInfo.cid = adInfo.cid;
                logItem.adInfo.mid = adInfo.mid;
                logItem.adInfo.cpid = adInfo.cpid;
                logItem.adInfo.offerPrice = adInfo.offerPrice;
            }
        }

        bool TanxBiddingHandler::filter(const BiddingFilterCallback& filterCb){
            if(bidRequest.is_ping()!=0){
                return bidFailedReturn();
            }
            //从BID Request中获取请求的广告位信息,目前只取第一个
            const BidRequest_AdzInfo& adzInfo = bidRequest.adzinfo(0);
            const std::string& pid = adzInfo.pid();
            AdSelectCondition queryCondition;
            queryCondition.pid = pid;
            if(!filterCb(this,queryCondition)){
                return bidFailedReturn();
            }
            isBidAccepted = false;
            return false;
        }

        void TanxBiddingHandler::buildBidResult(const SelectResult &result) {
            bidResponse.Clear();
            bidResponse.set_version(bidRequest.version());
            bidResponse.set_bid(bidRequest.bid());
            bidResponse.clear_ads();
            BidResponse_Ads* adResult = bidResponse.add_ads();
            rapidjson::Value& finalSolution = *(result.finalSolution);
            rapidjson::Value& adplace = *(result.adplace);
            rapidjson::Value& banner = *(result.banner);
            int advId = finalSolution["advid"].GetInt();
            const BidRequest_AdzInfo& adzInfo = bidRequest.adzinfo(0);
            int maxCpmPrice = max(result.bidPrice,adzInfo.min_cpm_price());
            auto buyerRules = adzInfo.buyer_rules();
            for(auto iter = buyerRules.begin();iter!=buyerRules.end();iter++){
                if(advId == iter->advertiser_ids()){
                    maxCpmPrice = max(maxCpmPrice,iter->min_cpm_price());
                    break;
                }
            }
            adResult->set_max_cpm_price(maxCpmPrice);
            //缓存最终广告结果
            adInfo.advId = finalSolution["advId"].GetInt();
            adInfo.adxid = ADX_TANX;
            adInfo.adxpid = adplace["adxpid"].GetInt();
            adInfo.adxuid = bidRequest.tid();
            adInfo.bannerId = banner["bid"].GetInt();
            adInfo.cid = adplace["cid"].GetInt();
            adInfo.mid = adplace["mid"].GetInt();
            adInfo.cpid = adInfo.advId;
            adInfo.offerPrice = maxCpmPrice;
        }

        void TanxBiddingHandler::match(HttpResponse &response) {
            std::string result;
            if(!writeProtoBufObject(bidResponse,&result)){
                DebugMessageWithTime("failed to write protobuf object in TanxBiddingHandler::match");
                reject(response);
                return;
            }
            response.setStatusCode(HttpResponse::k200Ok);
            response.setBody(result);
        }

        void TanxBiddingHandler::reject(HttpResponse &response) {
            bidResponse.Clear();
            bidResponse.set_version(bidRequest.version());
            bidResponse.set_bid(bidRequest.bid());
            std::string result;
            if(!writeProtoBufObject(bidResponse,&result)){
                DebugMessageWithTime("failed to write protobuf object in TanxBiddingHandler::reject");
                return;
            }
            response.setStatusCode(HttpResponse::k200Ok);
            response.setBody(result);
        }
    }
}