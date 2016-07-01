//
// Created by guoze.lin on 16/5/3.
//

#include "abstract_bidding_handler.h"
#include "utility/utility.h"

namespace protocol{
    namespace bidding{

        using namespace adservice::utility::url;

        int AbstractBiddingHandler::extractRealValue(const std::string& input,int targetAdx){
            const char* pdata = input.data();
            const char* p1 = pdata,*p2 = p1;
            while(*p2!='\0'){
                if(*p2=='|'){
                    int adx = atoi(p1);
                    p2++;
                    p1 = p2;
                    while(*p2!='\0'&&*p2!='|')p2++;
                    if(adx == targetAdx){
                        return atoi(p1);
                    }
                    if(*p2=='|'){
                        p2++;
                        p1=p2;
                    }
                } else
                    p2++;
            }
            return 0;
        }

        void AbstractBiddingHandler::getShowPara(const std::string &bid, char *showParamBuf, int showBufSize){
            int len;
            if(showParamBuf!=NULL) {
                len = snprintf(showParamBuf, showBufSize, "a=%s&b=%d&c=%d&d=%d&e=%d&s=%s&x=%d&r=%s&tm=%d", \
                adInfo.areaId.c_str(), adInfo.offerPrice, adInfo.bannerId, adInfo.advId, \
                adInfo.sid, adInfo.adxpid.c_str(), adInfo.adxid, bid.c_str(), time(NULL));
                if (len >= showBufSize) {
                    DebugMessageWithTime("In AbstractBiddingHandler::httpsnippet,showBufSize too small,actual:", len);
                }
            }
        }

        void AbstractBiddingHandler::getClickPara(const std::string &bid, char *clickParamBuf, int clickBufSize,
                                                  const std::string& ref,
                                                  const std::string& landingUrl) {
            if(clickParamBuf!=NULL) {
                char buffer[1024];
                std::string encodedLandingUrl;
                urlEncode_f(landingUrl, encodedLandingUrl, buffer);
                std::string encodedReferer;
                urlEncode_f(ref,encodedReferer,buffer);
                int len = snprintf(clickParamBuf, clickBufSize,
                                             "s=%s&x=%d&r=%s&d=%d&e=%d&c=%d&f=%s&h=000&a=%s&url=%s",
                                             adInfo.pid.data(),
                                             adInfo.adxid,
                                             adInfo.imp_id.data(),
                                             adInfo.advId,
                                             adInfo.sid,
                                             adInfo.cid,
                                             encodedReferer.data(),
                                             adInfo.areaId.data(),
                                             encodedLandingUrl.data()
                );
                if(len>=clickBufSize){
                    DebugMessageWithTime("in AbstractBiddingHandler::getClickPara,clickBufSize too small,actual:",len);
                }
            }
        }

        std::string AbstractBiddingHandler::generateHtmlSnippet(const std::string& bid,int width,int height,const char* extShowBuf,const char* cookieMappingUrl){
            char showBuf[2048];
            char clickBuf[2048];
            char html[4096];
            getShowPara(bid,showBuf,sizeof(showBuf));
            int len = snprintf(html,sizeof(html),SNIPPET_IFRAME,width,height,SNIPPET_SHOW_URL,extShowBuf,showBuf,cookieMappingUrl);
            if(len>=sizeof(html)){
                DebugMessageWithTime("generateHtmlSnippet buffer size not enough,needed:",len);
            }
            return std::string(html,html+len);
        }

        std::string AbstractBiddingHandler::generateScript(const std::string &bid,int width,int height,const char* scriptUrl,const char* clickMacro,const char* extParam){
            char showBuf[2048]={"\0"};
            char clickBuf[2048]={"\0"};
            char script[4096];
            getShowPara(bid,showBuf,sizeof(showBuf));
//            getClickPara(bid,clickBuf,sizeof(clickBuf),)
            int len = snprintf(script,sizeof(script),SNIPPET_SCRIPT,width,height,scriptUrl,showBuf,clickBuf,clickBuf,extParam,clickMacro);
            if(len>=sizeof(script)){
                DebugMessageWithTime("generateScript buffer size not enough,needed:",len);
            }
            return std::string(script,script+len);
        }

    }
}