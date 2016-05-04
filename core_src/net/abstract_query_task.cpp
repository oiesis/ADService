//
// Created by guoze.lin on 16/4/29.
//

#include "abstract_query_task.h"
#include "protocol/baidu/baidu_price.h"
#include "protocol/tanx/tanx_price.h"
#include "protocol/youku/youku_price.h"

namespace adservice{
    namespace corelogic{

        /**
             * 对ADX成交价进行解密
             */
        int decodeAdxExchangePrice(int adx,const std::string& input){
            if(input.empty())
                return 0;
            switch(adx){
                case ADX_TANX:
                    return tanx_price_decode(input);
                case ADX_YOUKU:
                    return youku_price_decode(input);
                case ADX_BAIDU:
                    return baidu_price_decode(input);
                default:
                    return 0;
            }
        }

        /**
         * 将请求参数按需求转换为Log对象
         * 鉴于不同的模块的参数有所差异,所以为了提高parse的速度,可以做parser分发,这是后续优化的点
         */
        void parseObjectToLogItem(ParamMap &paramMap,protocol::log::LogItem &log,const char* allQuery = NULL){ //考虑将临时对象作为static const对象来处理
            char buf[1024];
            adservice::types::string output;
            ParamMap::iterator iter;
            try {
                if ((iter=paramMap.find(URL_LANDING_URL)) != paramMap.end()) { //落地页
                    adservice::types::string &url = iter->second;//paramMap[URL_LANDING_URL];
                    urlDecode_f(url, output, buf);
                    log.adInfo.landingUrl = buf;
                }
                if ((iter=paramMap.find(URL_REFERER)) != paramMap.end()) { //ref
                    adservice::types::string &f = iter->second;//paramMap[URL_REFERER];
                    urlDecode_f(f, output, buf);
                    log.referer = f;
                }
                if ((iter=paramMap.find(URL_ADPLACE_ID)) != paramMap.end()) { //广告位Id
                    adservice::types::string &s = iter->second;//paramMap[URL_ADPLACE_ID];
                    log.adInfo.pid = s;
                    log.adInfo.adxpid = s;
                }
                if ((iter=paramMap.find(URL_EXPOSE_ID)) != paramMap.end()) { //曝光Id
                    adservice::types::string &r = iter->second;//paramMap[URL_EXPOSE_ID];
                    log.adInfo.imp_id = r;
                }
                if ((iter=paramMap.find(URL_ADOWNER_ID)) != paramMap.end()) { //广告主Id
                    adservice::types::string &d = iter->second;//paramMap[URL_ADOWNER_ID];
                    log.adInfo.advId = std::stol(d);
                    log.adInfo.cpid = log.adInfo.advId;
                }
//                if ((iter=paramMap.find(URL_ADPLAN_ID)) != paramMap.end()) { // 推广计划Id
//                    adservice::types::string &t = iter->second;//paramMap[URL_ADPLAN_ID];
//                    log.adInfo.cpid = std::stol(t);
//                }
                if ((iter=paramMap.find(URL_EXEC_ID)) != paramMap.end()) { // 投放单元Id
                    adservice::types::string &e = iter->second;//paramMap[URL_EXEC_ID];
                    log.adInfo.sid = std::stol(e);
                }
                if ((iter=paramMap.find(URL_CREATIVE_ID)) != paramMap.end()) { // 创意Id
                    adservice::types::string &c = iter->second;//paramMap[URL_CREATIVE_ID];
                    log.adInfo.bannerId = std::stol(c);
                }
                if ((iter=paramMap.find(URL_ADX_ID)) != paramMap.end()) { // 平台Id,adxId
                    adservice::types::string &x = iter->second;//paramMap[URL_ADX_ID];
                    log.adInfo.adxid = std::stoi(x);
                }
                if ((iter=paramMap.find(URL_CLICK_ID)) != paramMap.end()) { // clickId
                    adservice::types::string &h = iter->second;//paramMap[URL_CLICK_ID];
                    log.adInfo.clickId = h;
                }
                if ((iter=paramMap.find(URL_AREA_ID)) != paramMap.end()) { //areaId
                    adservice::types::string &a = iter->second;//paramMap[URL_AREA_ID];
                    log.adInfo.areaId = a;
                    int country,province,city;
                    extractAreaInfo(a.c_str(),country,province,city);
                    log.geoInfo.country=country;
                    log.geoInfo.province = province;
                    log.geoInfo.city = city;
                }
                if ((iter=paramMap.find(URL_CLICK_X)) != paramMap.end()){ //点击坐标x
                    adservice::types::string &sx = iter->second;//paramMap[URL_CLICK_X];
                    log.clickx = std::stoi(sx);
                }
                if ((iter=paramMap.find(URL_CLICK_Y)) != paramMap.end()){ //点击坐标y
                    adservice::types::string &sy = iter->second;//paramMap[URL_CLICK_Y];
                    log.clicky = std::stoi(sy);
                }
                if ((iter=paramMap.find(URL_EXCHANGE_PRICE)) != paramMap.end()){ //成交价格
                    adservice::types::string &price = iter->second;//paramMap[URL_EXCHANGE_PRICE];
                    //urlDecode_f(price,output,buffer);
                    log.adInfo.cost = decodeAdxExchangePrice(log.adInfo.adxid,price);
                    log.adInfo.bidPrice = (int)(log.adInfo.cost * AD_OWNER_COST_FACTOR);
                }
            }catch(std::exception& e){
                log.reqStatus = 500;
                LOG_ERROR<<"error:"<<e.what()<<",when processing query "<<allQuery;
            }
        }


        void checkAliEscapeSafe(protocol::log::LogItem &logItem,std::string& input){
            using namespace adservice::utility::escape;
            std::string escape_string = encode4ali(input);
            std::string decode_string = decode4ali(escape_string);
            const char* a = input.c_str();
            const char* b = decode_string.c_str();
            if(input.length()==decode_string.length()){
                bool isSame = true;
                for(int i=0;i<input.length();i++){
                    if(a[i]!=b[i]){
                        isSame = false;
                        break;
                    }
                }
                if(isSame){
                    protocol::log::LogItem parseLog;
                    getAvroObject(parseLog,(uint8_t*)decode_string.c_str(),decode_string.length());
                }else{
                    DebugMessage("decoded string not equal origin,escape4ali not safe");
                }
            }else{
                DebugMessage("decoded string length not equal origin,escape4ali not safe,",input.length()," ",decode_string.length());
            }
        }

        bool checkUserCookies(const adservice::types::string& oldCookies){
            CypherResult128 cypherResult;
            memcpy((void*)cypherResult.bytes,(void*)oldCookies.c_str(),oldCookies.length());
            DecodeResult64 decodeResult64;
            if(!cookiesDecode(cypherResult,decodeResult64)
               || decodeResult64.words[0]<=0
               || decodeResult64.words[0]>time::getCurrentTimeSinceMtty()){
                return false;
            }
            return true;
        }

        void AbstractQueryTask::filterParamMapSafe(ParamMap& paramMap){
            for(ParamMap::iterator iter = paramMap.begin();iter!=paramMap.end();iter++){
                stringSafeInput(iter->second,URL_LONG_INPUT_PARAMETER);
            }
        }

        void AbstractQueryTask::commonLogic(ParamMap& paramMap,protocol::log::LogItem& log,HttpResponse& resp){
            log.userAgent = userAgent;
            log.logType = currentPhase();
            log.reqMethod = reqMethod();
            log.reqStatus = expectedReqStatus();
            log.timeStamp = utility::time::getCurrentTimeStampUtc();
            log.referer = referer;
            log.ipInfo.proxy=userIp;
            if(!isPost) { //对于非POST方法传送的Query
                getParam(paramMap, data.c_str() + 1);
                filterParamMapSafe(paramMap);
                parseObjectToLogItem(paramMap,log,data.c_str()+1);
            }else{ //对于POST方法传送过来的Query
                getPostParam(paramMap);
            }
            adservice::types::string userId = extractCookiesParam(COOKIES_MTTY_ID,userCookies);
            bool needNewCookies = false;
            if(userId.empty()||!checkUserCookies(userId)){
                CypherResult128 cookiesResult;
                makeCookies(cookiesResult);
                log.userId = (char*)cookiesResult.char_bytes;
                needNewCookies = true;
            }else{
                log.userId = userId;
            }
            resp.setContentType("text/html");
#ifdef USE_ENCODING_GZIP
            resp.addHeader("Content-Encoding","gzip");
#endif
            if(needNewCookies) { //传入的cookies中没有userId,cookies 传出
                char cookiesString[64];
                sprintf(cookiesString, "%s=%s;Domain=.%s;Max-Age=2617488000;", COOKIES_MTTY_ID,log.userId.c_str(),COOKIES_MTTY_DOMAIN);//必要时加入Domain
                resp.addHeader("Set-Cookie", cookiesString);
            }
        }

        void AbstractQueryTask::doLog(protocol::log::LogItem& log){
            std::shared_ptr<adservice::types::string> logString = std::make_shared<adservice::types::string>();
            writeAvroObject(log, *(logString.get()));
#if defined(USE_ALIYUN_LOG) && defined(UNIT_TEST)
            checkAliEscapeSafe(log,*(logString.get()));
#endif
            // 将日志对象推送到日志队列
            CoreModule coreModule = CoreService::getInstance();
            if (coreModule.use_count() > 0)
                coreModule->getLogger()->push(logString);
        }

        void AbstractQueryTask::operator()(){
            try{
                ParamMap paramMap;
                protocol::log::LogItem log;
                HttpResponse resp(false);
                resp.setStatusCode(expectedReqStatus());
                commonLogic(paramMap,log,resp);
                customLogic(paramMap,log,resp);
                if(needLog)
                    doLog(log);
                Buffer buf;
                resp.appendToBuffer(&buf);
                conn->send(&buf); //这里将异步调用IO线程,进行数据回写
#ifdef USE_SHORT_CONN
                conn->shutdown(); //假定都是短链接
#endif
            }catch(std::exception& e){
                HttpResponse resp(false);
                resp.setStatusCode(HttpResponse::k500ServerError);
                resp.setStatusMessage("error");
                resp.setContentType("text/html");
                onError(e,resp);
                Buffer buf;
                resp.appendToBuffer(&buf);
                conn->send(&buf);
                conn->shutdown();
            }
        }

    }
}
