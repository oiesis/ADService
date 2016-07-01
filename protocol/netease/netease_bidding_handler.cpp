//
// Created by guoze.lin on 16/6/27.
//

#include "netease_bidding_handler.h"
#include "utility/utility.h"

namespace protocol{
    namespace bidding{

        using namespace adservice::utility::serialize;
        using namespace adservice::utility::json;

#define AD_NETEASE_SHOW_URL "http://show.mtty.com/v?%s"
#define AD_NETEASE_CLICK_URL "http://click.mtty.com/c?%s"

        int getNetEaseDeviceType(const std::string& platform){
            if(platform.find("Android")!=std::string::npos){
                return SOLUTION_DEVICE_ANDROID;
            }else if(platform.find("IOS")!=std::string::npos){
                return SOLUTION_DEVICE_IPHONE;
            } else
                return SOLUTION_DEVICE_OTHER;
        }

        bool NetEaseBiddingHandler::parseRequestData(const std::string& data){
            bidRequest.Clear();
            return parseJson(data.c_str(),bidRequest);
        }

        void NetEaseBiddingHandler::fillLogItem(protocol::log::LogItem &logItem) {
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

        bool NetEaseBiddingHandler::filter(const BiddingFilterCallback& filterCb){
            if(!bidRequest.HasMember("is_test")||bidRequest["is_test"].GetBool()){
                return false;
            }
            rapidjson::Value& adzinfo = bidRequest["adunit"];
            std::string pid = adzinfo["space_id"].GetString();
            AdSelectCondition queryCondition;
            queryCondition.adxid = ADX_NETEASE_MOBILE;
            queryCondition.adxpid = pid;
            //网易接口并没有给我们传ip
            queryCondition.ip = "";
            queryCondition.flowType = SOLUTION_FLOWTYPE_MOBILE;
            std::string platform = adzinfo["platform"].GetString();
            if(!filterCb(this,queryCondition)){
                return bidFailedReturn();
            }
            return isBidAccepted = true;
        }

        static const char* BIDRESPONSE_TEMPLATE = "{\"mainTitle\":\"\","
                "\"subTitle\":\"\","
                "\"monitor\":\"\","
                "\"showMonitorUrl\":\"\","
                "\"clickMonitorUrl\":\"\","
                "\"valid_time\":0,"
                "\"content\":\"\","
                "\"resource_url\":[],"
                "\"linkUrl\":\"\""
                "}";

        void NetEaseBiddingHandler::buildBidResult(const SelectResult &result) {
            rapidjson::Value& adzInfo = bidRequest["adunit"];
            rapidjson::Value& finalSolution = *(result.finalSolution);
            rapidjson::Value& adplace = *(result.adplace);
            rapidjson::Value& banner = *(result.banner);
            int advId = finalSolution["advid"].GetInt();
            bidResponse.Clear();
            if(!parseJson(BIDRESPONSE_TEMPLATE,bidResponse)){
                DebugMessageWithTime("in NetEaseBiddingHandler::buildBidResult parseJson failed");
                isBidAccepted = false;
                return;
            }

            //缓存最终广告结果
            adInfo.sid  = finalSolution["sid"].GetInt64();
            adInfo.advId = advId;
            adInfo.adxid = ADX_NETEASE_MOBILE;
            adInfo.adxpid = adplace["adxpid"].GetString();
            adInfo.adxuid = bidRequest["user"]["id"].GetString();
            adInfo.bannerId = banner["bid"].GetInt();
            adInfo.cid = adplace["cid"].GetInt();
            adInfo.mid = adplace["mid"].GetInt();
            adInfo.cpid = adInfo.advId;
            adInfo.offerPrice = result.bidPrice;
            adInfo.areaId = "0086-0-0";

            //html snippet相关
            std::string requestId = bidRequest["id"].GetString();
            char showParam[2048];
            char clickParam[2048];
            getShowPara(requestId,showParam,sizeof(showParam));
            char buffer[2048];
            snprintf(buffer,sizeof(buffer),AD_NETEASE_SHOW_URL,showParam);
            bidResponse["showMonitorUrl"].SetString(buffer,bidResponse.GetAllocator());

            char pjson[2048]={'\0'};
            std::string strBannerJson = banner["json"].GetString();
            strncat(pjson,strBannerJson.data(),sizeof(pjson));
            tripslash2(pjson);
            rapidjson::Document bannerJson;
            parseJson(pjson,bannerJson);
            std::string materialUrl = bannerJson["mtls"][0]["p0"].GetString();
            std::string landingUrl = bannerJson["mtls"][0]["p1"].GetString();
            getClickPara(requestId,clickParam,sizeof(clickParam),"",landingUrl);
            snprintf(buffer,sizeof(buffer),AD_NETEASE_CLICK_URL,clickParam);
            bidResponse["clickMonitorUrl"].SetString(buffer,bidResponse.GetAllocator());
            bidResponse["linkUrl"].SetString(landingUrl.data(),bidResponse.GetAllocator());
            bidResponse["resource_url"].PushBack(MakeStringValue(materialUrl),bidResponse.GetAllocator());
        }

        void NetEaseBiddingHandler::match(HttpResponse &response) {
            std::string result = toJson(bidResponse);
            if(result.empty()){
                DebugMessageWithTime("NetEaseBiddingHandler::match failed to parse obj to json");
                reject(response);
            }else{
                response.setStatusCode(HttpResponse::k200Ok);
                response.setBody(result);
                response.setContentType("application/json");
            }
        }

        void NetEaseBiddingHandler::reject(HttpResponse &response) {
            response.setStatusCode(HttpResponse::k204OkNoContent);
        }

    }
}