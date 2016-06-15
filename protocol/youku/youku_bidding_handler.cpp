//
// Created by guoze.lin on 16/5/3.
//

#include "youku_bidding_handler.h"
#include "utility/utility.h"
#include "core_ip_manager.h"

namespace protocol {
    namespace bidding {

        using namespace adservice::utility::serialize;
        using namespace adservice::utility::json;
        using namespace adservice::server;

#define   AD_YOUKU_CM_URL   ""
#define   AD_YOUKU_FEED		"http://show.mtty.com/v?res=none&p=%s&%s"
#define   AD_YOUKU_CLICK	"http://show.mtty.com/c?%s&url=%s"
#define   AD_YOUKU_PRICE 		"${AUCTION_PRICE}"

        bool YoukuBiddingHandler::parseRequestData(const std::string& data){
            bidRequest.Clear();
            return parseJson(data.c_str(),bidRequest);
        }

        void YoukuBiddingHandler::fillLogItem(protocol::log::LogItem &logItem) {
            logItem.reqStatus = 200;
            rapidjson::Value& deviceInfo = bidRequest["device"];
            logItem.userAgent = deviceInfo["ua"].GetString();
            logItem.ipInfo.proxy = deviceInfo["ip"].GetString();
            logItem.adInfo.adxid = ADX_YOUKU;
            if(isBidAccepted){
                rapidjson::Document device(rapidjson::kObjectType);
                device.AddMember("deviceInfo",deviceInfo,device.GetAllocator());
                logItem.deviceInfo = toJson(device);
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

        bool YoukuBiddingHandler::filter(const BiddingFilterCallback& filterCb){
            if(!bidRequest.HasMember("imp")){
                return false;
            }
            rapidjson::Value& impressions = bidRequest["imp"];
            if(!impressions.IsArray()||impressions.Size()<=0){
                return false;
            }
            rapidjson::Value& adzinfo = impressions[0];
            std::string pid = adzinfo["tagid"].GetString();
            AdSelectCondition queryCondition;
            queryCondition.adxpid = pid;
            std::string ip = bidRequest.HasMember("device")&&bidRequest["device"].HasMember("ip")?bidRequest["device"]["ip"].GetString():"";
            queryCondition.ip = ip;
            if(!filterCb(this,queryCondition)){
                return bidFailedReturn();
            }
            return isBidAccepted = true;
        }

        static const char* BIDRESPONSE_TEMPLATE = "{\"id\":\"\","
                                                "\"bidid\":\"\","
                                                "\"seatbid\":[{\"bid\":[{"
                                                "\"adm\":\"\","
                                                "\"id\":\"\","
                                                "\"impid\":\"\","
                                                "\"nurl\":\"\","
                                                "\"price\":\"\","
                                                "\"crid\":\"\","
                                                "\"ext\":{"
                                                    "\"ldp\":\"\","
                                                    "\"pm\":[],"
                                                    "\"cm\":[]"
                                                    "}"
                                                "}]}]}";

        void YoukuBiddingHandler::buildBidResult(const SelectResult &result) {
            rapidjson::Value& adzInfo = bidRequest["imp"][0];
            rapidjson::Value& finalSolution = *(result.finalSolution);
            rapidjson::Value& adplace = *(result.adplace);
            rapidjson::Value& banner = *(result.banner);
            int advId = finalSolution["advid"].GetInt();
            int bidFloor = adzInfo["bidfloor"].GetInt();
            int maxCpmPrice = std::max(result.bidPrice,bidFloor);
            bidResponse.Clear();
            if(!parseJson(BIDRESPONSE_TEMPLATE,bidResponse)){
                DebugMessageWithTime("in YoukuBiddingHandler::buildBidResult parseJson failed");
                isBidAccepted = false;
                return;
            }
            std::string requestId = bidRequest["id"].GetString();
            bidResponse["id"].SetString(requestId.data(),requestId.length());
            bidResponse["bidid"].SetString("1");
            rapidjson::Value& bidValue = bidResponse["seatbid"][0]["bid"][0];
            bidValue["id"].SetString("1");
            std::string impId = adzInfo["id"].GetString();
            bidValue["impid"].SetString(impId.data(),impId.length());
            bidValue["price"].SetInt(maxCpmPrice);

            //缓存最终广告结果
            adInfo.sid  = finalSolution["sid"].GetInt64();
            adInfo.advId = advId;
            adInfo.adxid = ADX_YOUKU;
            adInfo.adxpid = adplace["adxpid"].GetString();
            adInfo.adxuid = bidRequest["user"]["id"].GetString();
            adInfo.bannerId = banner["bid"].GetInt();
            adInfo.cid = adplace["cid"].GetInt();
            adInfo.mid = adplace["mid"].GetInt();
            adInfo.cpid = adInfo.advId;
            adInfo.offerPrice = maxCpmPrice;
            const std::string& userIp = bidRequest["device"]["ip"].GetString();
            IpManager& ipManager = IpManager::getInstance();
            adInfo.areaId = ipManager.getAreaCodeStrByIp(userIp.data());

            //html snippet相关
            rapidjson::Value& extValue = bidValue["ext"];
            char showParam[2048];
            char clickParam[2048];
            httpsnippet(requestId,showParam,sizeof(showParam),clickParam,sizeof(clickParam));
            char buffer[2048];
            snprintf(buffer,sizeof(buffer),AD_YOUKU_FEED,AD_YOUKU_PRICE,showParam);
            bidValue["nurl"].SetString(buffer,bidResponse.GetAllocator());
            std::string landingUrl = "";
            snprintf(buffer,sizeof(buffer),AD_YOUKU_CLICK,showParam,landingUrl.data());
            std::string cm = buffer;
            extValue["cm"].Clear();
            extValue["cm"].PushBack(MakeStringValue(cm),bidResponse.GetAllocator());
        }

        void YoukuBiddingHandler::match(HttpResponse &response) {
            std::string result = toJson(bidResponse);
            if(result.empty()){
                DebugMessageWithTime("YoukuBiddingHandler::match failed to parse obj to json");
                reject(response);
            }else{
                response.setStatusCode(HttpResponse::k200Ok);
                response.setBody(result);
                response.setContentType("application/json");
            }
        }

        void YoukuBiddingHandler::reject(HttpResponse &response) {
            response.setStatusCode(HttpResponse::k204OkNoContent);
        }

    }
}