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
        using namespace adservice::utility::userclient;
        using namespace adservice::server;

#define   AD_YOUKU_CM_URL   ""
#define   AD_YOUKU_FEED		"http://show.mtty.com/v?of=3&res=none&p=%s&%s"
#define   AD_YOUKU_CLICK	"http://click.mtty.com/c?%s"
#define   AD_YOUKU_PRICE 		"${AUCTION_PRICE}"
#define   YOUKU_DEVICE_HANDPHONE 0
#define   YOUKU_DEVICE_PAD       1
#define   YOUKU_DEVICE_PC        2
#define   YOUKU_DEVICE_TV        3
#define   YOUKU_OS_WINDOWS       "Windows"
#define   YOUKU_OS_ANDROID       "Android"
#define   YOUKU_OS_iPhone        "ios"
#define   YOUKU_OS_MAC           "Mac"

        int fromYoukuDevTypeOsType(int devType,const std::string& osType,const std::string& ua,int& flowType,int& mobileDev,int& pcOs,std::string& pcBrowser){
            if(devType==YOUKU_DEVICE_HANDPHONE){
                flowType = SOLUTION_FLOWTYPE_MOBILE;
                if(osType==YOUKU_OS_ANDROID){
                    mobileDev = SOLUTION_DEVICE_ANDROID;
                }else if(osType == YOUKU_OS_iPhone){
                    mobileDev = SOLUTION_DEVICE_IPHONE;
                } else
                    mobileDev == SOLUTION_DEVICE_OTHER;
            }else if(devType == YOUKU_DEVICE_PAD){
                flowType = SOLUTION_FLOWTYPE_MOBILE;
                if(osType==YOUKU_OS_ANDROID){
                    mobileDev = SOLUTION_DEVICE_ANDROIDPAD;
                }else if(osType == YOUKU_OS_iPhone){
                    mobileDev = SOLUTION_DEVICE_IPAD;
                } else
                    mobileDev == SOLUTION_DEVICE_OTHER;
            }else if (devType == YOUKU_DEVICE_PC){
                flowType = SOLUTION_FLOWTYPE_PC;
                pcOs = getOSTypeFromUA(ua);
                pcBrowser = getBrowserTypeFromUA(ua);
            }
        }


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
            queryCondition.adxid = ADX_YOUKU;
            queryCondition.adxpid = pid;
            if(bidRequest.HasMember("device")) {
                std::string ip = bidRequest["device"].HasMember("ip") ? bidRequest["device"]["ip"].GetString() : "";
                queryCondition.ip = ip;
                int devType = bidRequest["device"]["devicetype"].GetInt();
                std::string osType = bidRequest["device"]["os"].GetString();
                std::string ua = bidRequest["device"]["ua"].GetString();
                fromYoukuDevTypeOsType(devType,osType,ua,queryCondition.flowType,queryCondition.mobileDevice,queryCondition.pcOS,
                                        queryCondition.pcBrowserStr);
            }
            isDeal = false;
            dealId.clear();
            if(adzinfo.HasMember("pmp")){//deal 请求
                rapidjson::Value& deals = adzinfo["pmp"]["deals"];
                if(deals.Size()>0){
                    queryCondition.priorKey = deals[0]["id"].GetString();
                    isDeal = true;
                    dealId = queryCondition.priorKey;
                }
            }
            if(!filterCb(this,queryCondition)){
                return bidFailedReturn();
            }
            adInfo.pid = queryCondition.mttyPid;
            adInfo.adxpid = queryCondition.adxpid;
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
            char pjson[2048]={'\0'};
            std::string strBannerJson = banner["json"].GetString();
            strncat(pjson,strBannerJson.data(),sizeof(pjson));
            tripslash2(pjson);
            rapidjson::Document bannerJson;
            parseJson(pjson,bannerJson);
            std::string materialUrl = bannerJson["mtls"][0]["p0"].GetString();
            std::string landingUrl = bannerJson["mtls"][0]["p1"].GetString();

            rapidjson::Value& extValue = bidValue["ext"];
            char showParam[2048];
            char clickParam[2048];
            getShowPara(requestId,showParam,sizeof(showParam));
            if(isDeal){ //deal 加特殊参数w
                char dealParam[256];
                int dealParamLen = snprintf(dealParam,sizeof(dealParam),"&"URL_YOUKU_DEAL"=%s",dealId.data());
                strncat(showParam,dealParam,dealParamLen);
            }
            char buffer[2048];
            snprintf(buffer,sizeof(buffer),AD_YOUKU_FEED,AD_YOUKU_PRICE,showParam); //包含of=3
            bidValue["nurl"].SetString(buffer,bidResponse.GetAllocator());
            std::string crid = std::to_string(adInfo.bannerId);
            bidValue["crid"].SetString(crid.data(),crid.length());
            if(adzInfo.HasMember("video")){ //视频流量 adm为素材地址 ldp为点击链
                bidValue["adm"].SetString(materialUrl.data(),materialUrl.length());
                getClickPara(requestId,clickParam,sizeof(clickParam),"",landingUrl);
                snprintf(buffer,sizeof(buffer),AD_YOUKU_CLICK,clickParam);
                std::string cm = buffer;
                extValue["ldp"].SetString(cm.data(),cm.length());
            }else{ //动态创意流量 adm为iframe 设置type
                int w = banner["width"].GetInt();
                int h = banner["height"].GetInt();
                std::string html = generateHtmlSnippet(requestId,w,h,"of=2","");
                bidValue["adm"].SetString(html.data(),html.length());
                extValue["type"].SetString("c");
            }
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