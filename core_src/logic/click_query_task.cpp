//
// Created by guoze.lin on 16/4/29.
//

#include "click_query_task.h"
#include "utility/url.h"

namespace adservice{
    namespace corelogic{

        using namespace utility::url;

        void fill(char* &p,const char* src){
            while(*src!='\0'){
                *p++=*src++;
            }
        }

        void handleLandingUrl(protocol::log::LogItem& logItem,ParamMap& paramMap){
            paramMap["pid"] = paramMap[URL_ADPLACE_ID];
            paramMap["bid"] = paramMap[URL_CREATIVE_ID];
            paramMap["sid"] = paramMap[URL_EXEC_ID];
            char buffer[1024];
            char result[1024];
            std::string output;
            urlDecode_f(logItem.adInfo.landingUrl, output, buffer);
            char* p1 = buffer, *p = result;
            while(*p1!='\0'){
                while(*p1!='{' && *p1!='\0')*p++=*p1++;
                if(*p1=='\0')break;
                char* p2 = ++p1;
                while(*p2!='}' && *p2!='\0')p2++;
                if(*p2=='\0')break;
                *p2 = '\0';
                std::string name(p1,p2);
                *p2 = '}';
                fill(p,paramMap[name].c_str());
                p1 = ++p2;
            }
            *p = '\0';
            logItem.adInfo.landingUrl = std::string(result,p);
        }

        void HandleClickQueryTask::customLogic(ParamMap& paramMap,protocol::log::LogItem& log,HttpResponse& resp){
            if(!log.adInfo.landingUrl.empty()) {
                handleLandingUrl(log,paramMap);
                resp.setStatusCode(HttpResponse::k302Redirect);
                resp.addHeader("Location", log.adInfo.landingUrl);
                resp.setStatusMessage("OK");
            }else{
                resp.setStatusCode(HttpResponse::k400BadRequest);
                resp.setStatusMessage("Error,empty landing url");
            }
        }

    }
}