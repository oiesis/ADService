//
// Created by guoze.lin on 16/5/11.
//

#include "gdt_bidding_handler.h"
#include "utility.h"
#include "core_ip_manager.h"
#include <string>

namespace protocol {
    namespace bidding {

        using namespace protocol::gdt::adx;
        using namespace adservice::utility::serialize;
        using namespace adservice::server;

        inline int max(const int& a,const int& b){
            return a>b?a:b;
        }

        bool GdtBiddingHandler::parseRequestData(const std::string& data){
            bidRequest.Clear();
            return getProtoBufObject(bidRequest,data);
        }

        void GdtBiddingHandler::fillLogItem(protocol::log::LogItem &logItem) {
            logItem.reqStatus = 200;
            logItem.ipInfo.proxy = bidRequest.ip();
            logItem.adInfo.adxid = ADX_TENCENT_GDT;
            if(isBidAccepted){
                if(bidRequest.has_device()){
                    const BidRequest_Device& device = bidRequest.device();
                    logItem.deviceInfo = device.DebugString();
                }
                logItem.adInfo.sid = adInfo.sid;
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

        bool GdtBiddingHandler::filter(const BiddingFilterCallback& filterCb){
            if(bidRequest.is_ping()||bidRequest.is_test()){
                return bidFailedReturn();
            }
            //从BID Request中获取请求的广告位信息,目前只取第一个

            const BidRequest_Impression& adzInfo = bidRequest.impressions(0);
            long pid = adzInfo.placement_id();
            AdSelectCondition queryCondition;
            queryCondition.adxpid = std::to_string(pid);
            queryCondition.ip = bidRequest.ip();
            if(!filterCb(this,queryCondition)){
                return bidFailedReturn();
            }
            return isBidAccepted = true;
        }

        void GdtBiddingHandler::buildBidResult(const SelectResult &result) {
            bidResponse.Clear();
            bidResponse.set_request_id(bidRequest.id());
            bidResponse.clear_seat_bids();
            BidResponse_SeatBid* seatBid = bidResponse.add_seat_bids();
            rapidjson::Value& finalSolution = *(result.finalSolution);
            rapidjson::Value& adplace = *(result.adplace);
            rapidjson::Value& banner = *(result.banner);
            int advId = finalSolution["advid"].GetInt();
            const BidRequest_Impression& adzInfo = bidRequest.impressions(0);
            seatBid->set_impression_id(adzInfo.id());
            BidResponse_Bid* adResult = seatBid->add_bids();
            int maxCpmPrice = max(result.bidPrice,adzInfo.bid_floor());
            adResult->set_bid_price(maxCpmPrice);
            adResult->set_creative_id(std::to_string(banner["bid"].GetInt()));
            //缓存最终广告结果
            adInfo.advId = finalSolution["advId"].GetInt();
            adInfo.sid = finalSolution["sid"].GetInt64();
            adInfo.adxid = ADX_TENCENT_GDT;
            adInfo.adxpid = adplace["adxpid"].GetString();
            adInfo.adxuid = bidRequest.user().id();
            adInfo.bannerId = banner["bid"].GetInt();
            adInfo.cid = adplace["cid"].GetInt();
            adInfo.mid = adplace["mid"].GetInt();
            adInfo.cpid = adInfo.advId;
            adInfo.offerPrice = maxCpmPrice;

            const std::string& userIp = bidRequest.ip();
            IpManager& ipManager = IpManager::getInstance();
            adInfo.areaId = ipManager.getAreaCodeStrByIp(userIp.data());

            //html snippet相关
            char showParam[2048];
            httpsnippet(bidRequest.id(),showParam,sizeof(showParam),NULL,0);
            char buffer[2048];
            snprintf(buffer,sizeof(buffer),"of=3&%s",showParam);
            adResult->set_click_param(buffer);
            adResult->set_impression_param(buffer);
        }

        void GdtBiddingHandler::match(HttpResponse &response) {
            std::string result;
            if(!writeProtoBufObject(bidResponse,&result)){
                DebugMessageWithTime("failed to write protobuf object in GdtBiddingHandler::match");
                reject(response);
                return;
            }
            response.setContentType("application/x-protobuf");
            response.setStatusCode(HttpResponse::k200Ok);
            response.setBody(result);
        }

        void GdtBiddingHandler::reject(HttpResponse &response) {
            bidResponse.Clear();
            bidResponse.set_request_id(bidRequest.id());
            std::string result;
            if(!writeProtoBufObject(bidResponse,&result)){
                DebugMessageWithTime("failed to write protobuf object in GdtBiddingHandler::reject");
                return;
            }
            response.setContentType("application/x-protobuf");
            response.setStatusCode(HttpResponse::k200Ok);
            response.setBody(result);
        }
    }
}