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
#define   AD_YOUKU_FEED		"http://show.mtty.com/v?of=3&p=%s&%s"
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
                if(!strcasecmp(osType.data(),YOUKU_OS_ANDROID)){
                    mobileDev = SOLUTION_DEVICE_ANDROID;
                }else if(!strcasecmp(osType.data(),YOUKU_OS_iPhone)){
                    mobileDev = SOLUTION_DEVICE_IPHONE;
                } else
                    mobileDev == SOLUTION_DEVICE_OTHER;
            }else if(devType == YOUKU_DEVICE_PAD){
                flowType = SOLUTION_FLOWTYPE_MOBILE;
                if(!strcasecmp(osType.data(),YOUKU_OS_ANDROID)){
                    mobileDev = SOLUTION_DEVICE_ANDROIDPAD;
                }else if(!strcasecmp(osType.data(),YOUKU_OS_iPhone)){
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
            bidRequest.SetObject();
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
            adInfo.adxid = ADX_YOUKU;
            if(bidRequest.HasMember("device")) {
                rapidjson::Value& device = bidRequest["device"];
                std::string ip = device.HasMember("ip") ? device["ip"].GetString() : "";
                queryCondition.ip = ip;
                int devType = device.HasMember("devicetype")?device["devicetype"].GetInt():YOUKU_DEVICE_PC;
                std::string osType = device.HasMember("os")?device["os"].GetString():"";
                std::string ua = device.HasMember("ua")?device["ua"].GetString():"";
                fromYoukuDevTypeOsType(devType,osType,ua,queryCondition.flowType,queryCondition.mobileDevice,queryCondition.pcOS,
                                        queryCondition.pcBrowserStr);
                if(queryCondition.flowType == SOLUTION_FLOWTYPE_MOBILE){
                    queryCondition.adxid = ADX_YOUKU_MOBILE;
                    adInfo.adxid = ADX_YOUKU_MOBILE;
                }
            }
            isDeal = false;
            dealId.clear();
            if(adzinfo.HasMember("pmp")){//deal 请求
                rapidjson::Value& deals = adzinfo["pmp"]["deals"];
                if(deals.Size()>0){
                    queryCondition.dealId = deals[0]["id"].GetString();
                    isDeal = true;
                    dealId = queryCondition.dealId;
                }
            }
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
                                                    "\"cm\":[],"
                                                    "\"type\":\"\""
                                                    "}"
                                                "}]}]}";

        void YoukuBiddingHandler::buildBidResult(const AdSelectCondition& queryCondition,const SelectResult &result) {
            rapidjson::Value& adzInfo = bidRequest["imp"][0];
            rapidjson::Value& finalSolution = *(result.finalSolution);
            rapidjson::Value& adplace = *(result.adplace);
            rapidjson::Value& banner = *(result.banner);
            int advId = finalSolution["advid"].GetInt();
            int bidFloor = adzInfo["bidfloor"].GetInt();
            bidResponse.SetObject();
            if(!parseJson(BIDRESPONSE_TEMPLATE,bidResponse)){
                DebugMessageWithTime("in YoukuBiddingHandler::buildBidResult parseJson failed");
                isBidAccepted = false;
                return;
            }
            std::string requestId = bidRequest["id"].GetString();
            bidResponse["id"].SetString(requestId.data(),requestId.length(),bidResponse.GetAllocator());
            bidResponse["bidid"].SetString("1");
            rapidjson::Value& bidValue = bidResponse["seatbid"][0]["bid"][0];
            bidValue["id"].SetString("1");
            std::string impId = adzInfo["id"].GetString();
            bidValue["impid"].SetString(impId.data(),impId.length(),bidResponse.GetAllocator());


            //缓存最终广告结果
            adInfo.pid = std::to_string(adplace["pid"].GetInt());
            adInfo.adxpid = adplace["adxpid"].GetString();
            adInfo.adxpid = queryCondition.adxpid;
            adInfo.sid  = finalSolution["sid"].GetInt64();
            adInfo.advId = advId;
            adInfo.adxid = queryCondition.adxid;
            adInfo.adxuid = bidRequest["user"]["id"].GetString();
            adInfo.bannerId = banner["bid"].GetInt();
            adInfo.cid = adplace["cid"].GetInt();
            adInfo.mid = adplace["mid"].GetInt();
            adInfo.cpid = adInfo.advId;
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
            std::string tview = bannerJson["tview"].GetString();

            rapidjson::Value& extValue = bidValue["ext"];
            char showParam[2048];
            char clickParam[2048];
            getShowPara(requestId,showParam,sizeof(showParam));
            if(isDeal){ //deal 加特殊参数w
                char dealParam[256];
                int dealParamLen = snprintf(dealParam,sizeof(dealParam),"&"URL_YOUKU_DEAL"=%s",dealId.data());
                strncat(showParam,dealParam,dealParamLen);
                bidValue.AddMember("dealid",MakeStringValue2(dealId,bidResponse.GetAllocator()),bidResponse.GetAllocator());
            }
            char buffer[2048];
            snprintf(buffer,sizeof(buffer),AD_YOUKU_FEED,AD_YOUKU_PRICE,showParam); //包含of=3
            bidValue["nurl"].SetString(buffer,bidResponse.GetAllocator());
            std::string crid = std::to_string(adInfo.bannerId);
            bidValue["crid"].SetString(crid.data(),crid.length(),bidResponse.GetAllocator());
            if(adzInfo.HasMember("video")||queryCondition.flowType == SOLUTION_FLOWTYPE_MOBILE){ //视频流量 adm为素材地址 ldp为点击链
                bidValue["adm"].SetString(materialUrl.data(),materialUrl.length(),bidResponse.GetAllocator());
                getClickPara(requestId,clickParam,sizeof(clickParam),"",landingUrl);
                snprintf(buffer,sizeof(buffer),AD_YOUKU_CLICK,clickParam);
                std::string cm = buffer;
                extValue["ldp"].SetString(cm.data(),cm.length(),bidResponse.GetAllocator());
                if(!tview.empty()){
                    extValue["pm"].PushBack(MakeStringValue2(tview,bidResponse.GetAllocator()),bidResponse.GetAllocator());
                }
                bidFloor = 0;
            }else{ //动态创意流量 adm为iframe 设置type
                int w = banner["width"].GetInt();
                int h = banner["height"].GetInt();
                std::string html = generateHtmlSnippet(requestId,w,h,"of=2&","");
                bidValue["adm"].SetString(html.data(),html.length(),bidResponse.GetAllocator());
                extValue["type"].SetString("c");
            }
            int maxCpmPrice = std::max(result.bidPrice,bidFloor);
            bidValue["price"].SetInt(maxCpmPrice);
            adInfo.offerPrice = maxCpmPrice;
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