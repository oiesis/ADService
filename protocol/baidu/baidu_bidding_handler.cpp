//
// Created by guoze.lin on 16/5/3.
//

#include "baidu_bidding_handler.h"
#include "utility/utility.h"
#include "core_ip_manager.h"

namespace protocol {
    namespace bidding {

        using namespace protocol::Baidu;
        using namespace adservice::utility::serialize;
        using namespace adservice::server;

#define AD_BD_CLICK_MACRO  "%%CLICK_URL_ESC%%"
#define AD_BD_PRICE_MACRO  "%%PRICE%%"
#define AD_COOKIEMAPPING_BAIDU		""

        inline int max(const int& a,const int& b){
            return a>b?a:b;
        }

        bool BaiduBiddingHandler::parseRequestData(const std::string& data){
            bidRequest.Clear();
            return getProtoBufObject(bidRequest,data);
        }

        std::string BaiduBiddingHandler::baiduHtmlSnippet() {
            char extShowBuf[1024];
            const std::string& bid = bidRequest.id();
            const BidRequest_AdSlot& adSlot = bidRequest.adslot(0);
            int width = adSlot.width();
            int height = adSlot.height();
            int len = snprintf(extShowBuf,sizeof(extShowBuf),"p=%s&l=%s&",AD_BD_PRICE_MACRO,AD_BD_CLICK_MACRO);
            if(len>=sizeof(extShowBuf)){
                DebugMessageWithTime("BaiduBiddingHandler::baiduHtmlSnippet,extShowBuf buffer size not enough,needed:",len);
            }
            return generateHtmlSnippet(bid,width,height,extShowBuf);
        }

        std::string BaiduBiddingHandler::baiduHtmlScript() {
            char extParam[1024];
            const std::string& bid = bidRequest.id();
            const BidRequest_AdSlot& adSlot = bidRequest.adslot(0);
            int width = adSlot.width();
            int height = adSlot.height();
            int len = snprintf(extParam,sizeof(extParam),"p=%s",AD_BD_PRICE_MACRO);
            const std::string sHtml = "";
            return generateScript(bid,width,height,sHtml.data(),AD_BD_CLICK_MACRO,extParam);
        }

        void BaiduBiddingHandler::fillLogItem(protocol::log::LogItem &logItem) {
            logItem.reqStatus = 200;
            logItem.userAgent = bidRequest.user_agent();
            logItem.ipInfo.proxy = bidRequest.ip();
            logItem.adInfo.adxid = ADX_BAIDU;
            if(isBidAccepted){
                if(bidRequest.has_mobile()){
                    const BidRequest_Mobile& mobile = bidRequest.mobile();
                    logItem.deviceInfo = mobile.DebugString();
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

        bool BaiduBiddingHandler::filter(const BiddingFilterCallback& filterCb){
            if(bidRequest.is_ping()||bidRequest.is_test()){
                return bidFailedReturn();
            }
            //从BID Request中获取请求的广告位信息,目前只取第一个
            const BidRequest_AdSlot& adSlot = bidRequest.adslot(0);
            long pid = adSlot.ad_block_key();
            AdSelectCondition queryCondition;
            queryCondition.adxid = ADX_BAIDU;
            queryCondition.adxpid = std::to_string(pid);
            queryCondition.ip = bidRequest.ip();
            if(!filterCb(this,queryCondition)){
                return bidFailedReturn();
            }
            return isBidAccepted = true;
        }

        void BaiduBiddingHandler::buildBidResult(const AdSelectCondition& queryCondition,const SelectResult &result) {
            bidResponse.Clear();
            bidResponse.set_id(bidRequest.id());
            bidResponse.clear_ad();
            BidResponse_Ad* adResult = bidResponse.add_ad();
            rapidjson::Value& finalSolution = *(result.finalSolution);
            rapidjson::Value& adplace = *(result.adplace);
            rapidjson::Value& banner = *(result.banner);
            int advId = finalSolution["advid"].GetInt();
            const BidRequest_AdSlot& adSlot = bidRequest.adslot(0);
            int maxCpmPrice = max(result.bidPrice,adSlot.minimum_cpm());
            adResult->set_max_cpm(maxCpmPrice);
            adResult->set_advertiser_id(advId);
            adResult->set_creative_id(banner["bid"].GetInt());
            adResult->set_height(banner["height"].GetInt());
            adResult->set_width(banner["width"].GetInt());
            adResult->set_sequence_id(adSlot.sequence_id());
            //缓存最终广告结果
            adInfo.advId = advId;
            adInfo.sid = finalSolution["sid"].GetInt64();
            adInfo.adxid = ADX_BAIDU;
            adInfo.adxpid = adplace["adxpid"].GetString();
            adInfo.adxuid = bidRequest.baidu_user_id();
            adInfo.bannerId = banner["bid"].GetInt();
            adInfo.cid = adplace["cid"].GetInt();
            adInfo.mid = adplace["mid"].GetInt();
            adInfo.cpid = adInfo.advId;
            adInfo.offerPrice = maxCpmPrice;
            const std::string& userIp = bidRequest.ip();
            IpManager& ipManager = IpManager::getInstance();
            adInfo.areaId = ipManager.getAreaCodeStrByIp(userIp.data());
            adResult->set_html_snippet(baiduHtmlSnippet());
        }

        void BaiduBiddingHandler::match(HttpResponse &response) {
            std::string result;
            if(!writeProtoBufObject(bidResponse,&result)){
                DebugMessageWithTime("failed to write protobuf object in BaiduBiddingHandler::match");
                reject(response);
                return;
            }
            response.setStatusCode(HttpResponse::k200Ok);
            response.setBody(result);
        }

        void BaiduBiddingHandler::reject(HttpResponse &response) {
            bidResponse.Clear();
            bidResponse.set_id(bidRequest.id());
            std::string result;
            if(!writeProtoBufObject(bidResponse,&result)){
                DebugMessageWithTime("failed to write protobuf object in BaiduBiddingHandler::reject");
                return;
            }
            response.setStatusCode(HttpResponse::k200Ok);
            response.setBody(result);
        }

    }
}