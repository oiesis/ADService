//
// Created by guoze.lin on 16/5/3.
//

#include "abstract_bidding_handler.h"
#include "utility/utility.h"

namespace protocol{
    namespace bidding{

        using namespace adservice::utility::url;

        void AbstractBiddingHandler::httpsnippet(const std::string& bid,char* showParamBuf,int showBufSize,char* clickParamBuf,int clickBufSize){
            int len;
            if(showParamBuf!=NULL) {
                len = snprintf(showParamBuf, showBufSize, "a=%s&b=%d&c=%d&d=%d&e=%d&s=%s&x=%d&r=%s&tm=%d", \
                adInfo.areaId.c_str(), adInfo.offerPrice, adInfo.bannerId, adInfo.advId, \
                adInfo.sid, adInfo.adxpid.c_str(), adInfo.adxid, bid.c_str(), time(NULL));
                if (len >= showBufSize) {
                    DebugMessageWithTime("In AbstractBiddingHandler::httpsnippet,showBufSize too small,actual:", len);
                }
            }
            if(clickParamBuf!=NULL) {
                char buf[2048];
                len = snprintf(buf, sizeof(buf), "%s,%d,%d,%d,%s,%s,%d,%s",
                               bid.c_str(), adInfo.advId, adInfo.sid, adInfo.adxid, adInfo.pid.c_str(),
                               adInfo.areaId.c_str(), adInfo.cid, "");
                if (len >= clickBufSize) {
                    DebugMessageWithTime("In AbstractBiddingHandler::httpsnippet,clickBufSize too small,actual:", len);
                }
                //url encode
                std::string output;
                urlEncode_f(buf,len,output,clickParamBuf);
            }
        }

        std::string AbstractBiddingHandler::generateHtmlSnippet(const std::string& bid,int width,int height,char* extShowBuf,const char* cookieMappingUrl){
            char showBuf[2048];
            char clickBuf[2048];
            char html[4096];
            httpsnippet(bid,showBuf,sizeof(showBuf),clickBuf,sizeof(clickBuf));
            int len = snprintf(html,sizeof(html),SNIPPET_IFRAME,width,height,SNIPPET_SHOW_URL,extShowBuf,showBuf,cookieMappingUrl);
            if(len>=sizeof(html)){
                DebugMessageWithTime("generateHtmlSnippet buffer size not enough,needed:",len);
            }
            return std::string(html,html+len);
        }

        std::string AbstractBiddingHandler::generateScript(const std::string &bid,int width,int height,const char* scriptUrl,const char* clickMacro,const char* extParam){
            char showBuf[2048];
            char clickBuf[2048];
            char script[4096];
            httpsnippet(bid,showBuf,sizeof(showBuf),clickBuf,sizeof(clickBuf));
            int len = snprintf(script,sizeof(script),SNIPPET_SCRIPT,width,height,scriptUrl,showBuf,clickBuf,clickBuf,extParam,clickMacro);
            if(len>=sizeof(script)){
                DebugMessageWithTime("generateScript buffer size not enough,needed:",len);
            }
            return std::string(script,script+len);
        }

    }
}