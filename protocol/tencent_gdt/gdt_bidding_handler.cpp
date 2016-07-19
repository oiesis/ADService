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
        using namespace adservice::utility::userclient;
        using namespace adservice::server;

        static GdtAdplaceMap gdtAdplaceMap;

        inline int max(const int& a,const int& b){
            return a>b?a:b;
        }


        int getGdtOsType(BidRequest_OperatingSystem os){
            switch(os){
                case BidRequest_OperatingSystem::BidRequest_OperatingSystem_kOSWindows:
                    return SOLUTION_OS_WINDOWS;
                default:
                    return SOLUTION_OS_OTHER;
            }
        }

        int getGdtMobileDeviceType(BidRequest_OperatingSystem os){
            switch(os){
                case BidRequest_OperatingSystem::BidRequest_OperatingSystem_kOSWindows:
                    return SOLUTION_DEVICE_WINDOWSPHONE;
                case BidRequest_OperatingSystem::BidRequest_OperatingSystem_kOSAndroid:
                    return SOLUTION_DEVICE_ANDROID;
                case BidRequest_OperatingSystem::BidRequest_OperatingSystem_kOSIOS:
                    return SOLUTION_DEVICE_IPHONE;
                default:
                    return SOLUTION_DEVICE_OTHER;
            }
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
            queryCondition.adxid = ADX_TENCENT_GDT;
            queryCondition.adxpid = std::to_string(pid);
            queryCondition.ip = bidRequest.ip();
            IpManager& ipManager = IpManager::getInstance();
            queryCondition.dGeo = ipManager.getAreaByIp(queryCondition.ip.data());
            PreSetAdplaceInfo adplaceInfo;
            for(int i=0;i<adzInfo.creative_specs_size();i++){
                int createspecs = adzInfo.creative_specs(i);
                if(gdtAdplaceMap.find(createspecs)) {
                    GdtAdplace &gdtAdplace = gdtAdplaceMap.get(createspecs);
                    adplaceInfo.sizeArray.push_back(std::make_tuple(gdtAdplace.width,gdtAdplace.height));
                    adplaceInfo.flowType = gdtAdplace.flowType;
                }
            }
            if(adplaceInfo.sizeArray.size()==0){
                return bidFailedReturn();
            }
            if(bidRequest.has_device()){ //device
                const BidRequest_Device& device = bidRequest.device();
                BidRequest_DeviceType devType = device.device_type();
                if(devType==BidRequest_DeviceType::BidRequest_DeviceType_kDeviceTypePC){
                    queryCondition.pcOS = getGdtOsType(device.os());
                    queryCondition.pcBrowserStr = getBrowserTypeFromUA(device.user_agent());
                    if(queryCondition.pcOS==SOLUTION_OS_OTHER){
                        queryCondition.pcOS = getOSTypeFromUA(device.user_agent());
                    }
                }else if(devType==BidRequest_DeviceType::BidRequest_DeviceType_kDeviceTypeMobile){
                    adplaceInfo.flowType = SOLUTION_FLOWTYPE_MOBILE;
                    queryCondition.mobileDevice = getGdtMobileDeviceType(device.os());
                }else if(devType == BidRequest_DeviceType::BidRequest_DeviceType_kDeviceTypePad){
                    queryCondition.mobileDevice = device.os()==BidRequest_OperatingSystem_kOSIOS?SOLUTION_DEVICE_IPAD:
                                                  SOLUTION_DEVICE_ANDROIDPAD;
                }else{
                    queryCondition.mobileDevice = SOLUTION_DEVICE_OTHER;
                    queryCondition.pcOS = SOLUTION_OS_OTHER;
                }
            }
            queryCondition.pAdplaceInfo = &adplaceInfo;
            if(!filterCb(this,queryCondition)){
                return bidFailedReturn();
            }

            return isBidAccepted = true;
        }

        void GdtBiddingHandler::buildBidResult(const AdSelectCondition& queryCondition,const SelectResult &result) {
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
            adInfo.pid = queryCondition.mttyPid;
            adInfo.adxpid = queryCondition.adxpid;
            adInfo.advId = finalSolution["advId"].GetInt();
            adInfo.sid = finalSolution["sid"].GetInt64();
            adInfo.adxid = ADX_TENCENT_GDT;
            adInfo.adxuid = bidRequest.user().id();
            adInfo.bannerId = banner["bid"].GetInt();
            adInfo.cpid = adInfo.advId;
            adInfo.offerPrice = maxCpmPrice;

            const std::string& userIp = bidRequest.ip();
            IpManager& ipManager = IpManager::getInstance();
            adInfo.areaId = ipManager.getAreaCodeStrByIp(userIp.data());

            //html snippet相关
            char showParam[2048];
            getShowPara(bidRequest.id(),showParam,sizeof(showParam));
            strncat(showParam,"&of=3",5);
            adResult->set_click_param(showParam);
            adResult->set_impression_param(showParam);
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