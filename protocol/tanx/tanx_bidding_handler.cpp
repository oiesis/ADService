//
// Created by guoze.lin on 16/5/3.
//

#include "tanx_bidding_handler.h"
#include "core_ip_manager.h"
#include "utility.h"

namespace protocol {
    namespace bidding {

        using namespace protocol::Tanx;
        using namespace adservice::utility::serialize;
        using namespace adservice::utility::json;
        using namespace adservice::utility::userclient;
        using namespace adservice::server;

#define AD_TX_CLICK_MACRO	"%%CLICK_URL_PRE_ENC%%"
#define AD_TX_PRICE_MACRO  "%%SETTLE_PRICE%%"
#define AD_COOKIEMAPPING_TANX		""

        inline int max(const int& a,const int& b){
            return a>b?a:b;
        }

        void extractSize(const std::string& size,int& width,int& height){
            const char* start = size.data(),*p = start;
            while(*p!='\0'&&*p!='x')p++;
            if(*p=='\0'){
                width = 0;
                height = 0;
            }else{
                width = atoi(start);
                height = atoi(p+1);
            }
        }

        int getDeviceType(const std::string& deviceInfo){
            if(deviceInfo.find("iphone")!=std::string::npos){
                return SOLUTION_DEVICE_IPHONE;
            }else if(deviceInfo.find("android")!=std::string::npos){
                return SOLUTION_DEVICE_ANDROID;
            }else if(deviceInfo.find("ipad")!=std::string::npos){
                return SOLUTION_DEVICE_IPAD;
            } else
                return SOLUTION_DEVICE_OTHER;
        }

        std::string TanxBiddingHandler::tanxHtmlSnippet(){
            std::string bid = bidRequest.bid();
            bool isMobile = bidRequest.has_mobile();
            const BidRequest_AdzInfo& adzInfo = bidRequest.adzinfo(0);
            std::string bannerSize = adzInfo.size();
            int width,height;
            extractSize(bannerSize,width,height);
            return generateHtmlSnippet(bid,width,height,NULL);
        }

        std::string TanxBiddingHandler::generateHtmlSnippet(const std::string& bid,int width,int height,const char* extShowBuf,const char* cookieMappingUrl){
            char showBuf[2048];
            char clickBuf[2048];
            char html[4096];
            getShowPara(bid,showBuf,sizeof(showBuf));
            int len = snprintf(feedbackUrl,sizeof(feedbackUrl),"%s?%s%s&of=3",SNIPPET_SHOW_URL,"p="AD_TX_PRICE_MACRO"&",showBuf);
            strncat(showBuf,"&of=2",5);
            len = snprintf(html,sizeof(html),SNIPPET_IFRAME,width,height,SNIPPET_SHOW_URL,"l="AD_TX_CLICK_MACRO"&",showBuf,cookieMappingUrl);
            if(len>=sizeof(html)){
                DebugMessageWithTime("generateHtmlSnippet buffer size not enough,needed:",len);
            }
            return std::string(html,html+len);
        }

        bool TanxBiddingHandler::parseRequestData(const std::string& data){
            bidRequest.Clear();
            return getProtoBufObject(bidRequest,data);
        }

        void TanxBiddingHandler::fillLogItem(protocol::log::LogItem &logItem) {
            logItem.reqStatus = 200;
            logItem.userAgent = bidRequest.user_agent();
            logItem.ipInfo.proxy = bidRequest.ip();
            logItem.adInfo.adxid = ADX_TANX;
            logItem.adInfo.pid = adInfo.pid;
            if(isBidAccepted){
                if(bidRequest.has_mobile()){
                    const BidRequest_Mobile& mobile = bidRequest.mobile();
                    if(mobile.has_device()){
                        const BidRequest_Mobile_Device& device = mobile.device();
                        logItem.deviceInfo = device.DebugString();
                        adInfo.adxid = logItem.adInfo.adxid = ADX_TANX_MOBILE;
                    }
                }
                logItem.adInfo.sid = adInfo.sid;
                logItem.adInfo.advId = adInfo.advId;
                logItem.adInfo.pid = adInfo.pid;
                logItem.adInfo.adxpid = adInfo.adxpid;
                logItem.adInfo.adxuid = adInfo.adxuid;
                logItem.adInfo.bannerId = adInfo.bannerId;
                logItem.adInfo.cid = adInfo.cid;
                logItem.adInfo.mid = adInfo.mid;
                logItem.adInfo.cpid = adInfo.cpid;
                logItem.adInfo.offerPrice = adInfo.offerPrice;
                logItem.adInfo.areaId = adInfo.areaId;
            }
        }

        bool TanxBiddingHandler::filter(const BiddingFilterCallback& filterCb){
            if(bidRequest.is_ping()!=0){
                return bidFailedReturn();
            }
            //从BID Request中获取请求的广告位信息,目前只取第一个
            if(bidRequest.adzinfo_size()<=0)
                return bidFailedReturn();
            const BidRequest_AdzInfo& adzInfo = bidRequest.adzinfo(0);
            const std::string& pid = adzInfo.pid();
            AdSelectCondition queryCondition;
            queryCondition.adxid = ADX_TANX;
            queryCondition.adxpid = pid;
            queryCondition.ip = bidRequest.ip();
            if(bidRequest.has_mobile()){
                queryCondition.mobileDevice = getDeviceType(bidRequest.mobile().device().platform());
                queryCondition.flowType = SOLUTION_FLOWTYPE_MOBILE;
            }else{
                queryCondition.pcOS = getOSTypeFromUA(bidRequest.user_agent());
                queryCondition.pcBrowserStr = getBrowserTypeFromUA(bidRequest.user_agent());
                queryCondition.flowType = SOLUTION_FLOWTYPE_PC;
            }
            if(!filterCb(this,queryCondition)){
                adInfo.pid = queryCondition.mttyPid;
                return bidFailedReturn();
            }
            adInfo.pid = queryCondition.mttyPid;
            adInfo.adxpid = queryCondition.adxpid;
            return isBidAccepted = true;
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
            std::string adxAdvIdStr = !banner["adx_advid"].IsNull()?banner["adx_advid"].GetString():"";
            int adxAdvId = extractRealValue(adxAdvIdStr.data(),ADX_TANX);
            std::string adxIndustryTypeStr = !banner["adx_industrytype"].IsNull()?banner["adx_industrytype"].GetString():"";
            int adxIndustryType = extractRealValue(adxIndustryTypeStr.data(),ADX_TANX);
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
            adResult->set_adzinfo_id(adzInfo.id());
            adResult->set_ad_bid_count_idx(0);
//            adResult->add_category();
            adResult->add_creative_type(banner["bannertype"].GetInt());
            adResult->add_category(adxIndustryType);
            //缓存最终广告结果
            adInfo.sid = finalSolution["sid"].GetInt64();
            adInfo.advId = advId;
            adInfo.adxid = ADX_TANX;
            adInfo.adxuid = bidRequest.tid();
            adInfo.bannerId = banner["bid"].GetInt();
            adInfo.cid = adplace["cid"].GetInt();
            adInfo.mid = adplace["mid"].GetInt();
            adInfo.cpid = adInfo.advId;
            adInfo.offerPrice = maxCpmPrice;
            const std::string& userIp = bidRequest.ip();
            IpManager& ipManager = IpManager::getInstance();
            adInfo.areaId = ipManager.getAreaCodeStrByIp(userIp.data());

            char pjson[2048]={'\0'};
            std::string strBannerJson = banner["json"].GetString();
            strncat(pjson,strBannerJson.data(),sizeof(pjson));
            tripslash2(pjson);
            rapidjson::Document bannerJson;
            parseJson(pjson,bannerJson);
            std::string destUrl = bannerJson["mtls"][0]["p1"].GetString();

            adResult->add_destination_url(destUrl);
            adResult->add_click_through_url(destUrl);
            adResult->set_creative_id(std::to_string(adInfo.bannerId));
            adResult->add_advertiser_ids(adxAdvId);//adx_advid
            adResult->set_html_snippet(tanxHtmlSnippet());
            adResult->set_feedback_address(feedbackUrl);
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