//
// Created by guoze.lin on 16/5/3.
//

#include "tanx_bidding_handler.h"
#include "utility.h"

namespace protocol {
    namespace bidding {

        using namespace protocol::Tanx;
        using namespace adservice::utility::serialize;
        using namespace adservice::utility::json;

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

        std::string TanxBiddingHandler::tanxHtmlSnippet(){
            char extShowBuf[1024];
            std::string bid = bidRequest.bid();
            bool isMobile = bidRequest.has_mobile();
            const BidRequest_AdzInfo& adzInfo = bidRequest.adzinfo(0);
            std::string bannerSize = adzInfo.size();
            int width,height;
            extractSize(bannerSize,width,height);
            if(isMobile){ //mobile
                int api = adzInfo.api_size()>0?adzInfo.api(0):0;
                if(api!=3&&api!=5){
                    snprintf(extShowBuf,sizeof(extShowBuf),"of=2&l=%s&",AD_TX_CLICK_MACRO);
                }else{
                    DebugMessage("TanxBiddingHandler::generateHttpSnippet mobile api not supported,api:",api);
                    return "";
                }
            }else{ //pc
                snprintf(extShowBuf,sizeof(extShowBuf),"of=0&p=%s&l=%s&",AD_TX_PRICE_MACRO,AD_TX_CLICK_MACRO);
            }
            return generateHtmlSnippet(bid,width,height,extShowBuf);
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
            if(isBidAccepted){
                if(bidRequest.has_mobile()){
                    const BidRequest_Mobile& mobile = bidRequest.mobile();
                    if(mobile.has_device()){
                        const BidRequest_Mobile_Device& device = mobile.device();
                        logItem.deviceInfo = device.DebugString();
                    }
                }
                logItem.adInfo.sid = adInfo.sid;
                logItem.adInfo.advId = adInfo.advId;
                logItem.adInfo.adxid = adInfo.adxid;
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
            const BidRequest_AdzInfo& adzInfo = bidRequest.adzinfo(0);
            const std::string& pid = adzInfo.pid();
            AdSelectCondition queryCondition;
            queryCondition.adxpid = pid;
            queryCondition.ip = bidRequest.ip();
            if(!filterCb(this,queryCondition)){
                return bidFailedReturn();
            }
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
            std::string industryType = banner["industrytype"].GetString();
            adResult->add_category(std::stoi(industryType));
            //缓存最终广告结果
            adInfo.pid = std::to_string(adplace["pid"].GetInt64());
            adInfo.sid = finalSolution["sid"].GetInt64();
            adInfo.advId = advId;
            adInfo.adxid = ADX_TANX;
            const char* strAdxPid = adplace["adxpid"].GetString();
            adInfo.adxpid = strAdxPid;
            adInfo.adxuid = bidRequest.tid();
            adInfo.bannerId = banner["bid"].GetInt();
            adInfo.cid = adplace["cid"].GetInt();
            adInfo.mid = adplace["mid"].GetInt();
            adInfo.cpid = adInfo.advId;
            adInfo.offerPrice = maxCpmPrice;
            adInfo.areaId = "0086-ffff-ffff";

            char pjson[2048]={'\0'};
            std::string strBannerJson = banner["json"].GetString();
            strncat(pjson,strBannerJson.data(),sizeof(pjson));
            tripslash2(pjson);
            rapidjson::Document bannerJson;
            parseJson(pjson,bannerJson);
            std::string destUrl = bannerJson["mtls"][0]["p1"].GetString();

            adResult->add_destination_url(destUrl);
            adResult->add_click_through_url(destUrl);
            adResult->set_creative_id(std::to_string(adInfo.cid));
            adResult->add_advertiser_ids(advId);
            adResult->set_html_snippet(tanxHtmlSnippet());
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