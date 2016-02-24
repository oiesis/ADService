//
// Created by guoze.lin on 16/2/2.
//

#include "click_service.h"
#include "utility/utility.h"
#include "protocol/click/click.h"
#include "protocol/log/log.h"
#include "common/types.h"
#include <exception>

extern adservice::click::ClickModule g_clickService;

namespace adservice{

    namespace click{

        using namespace std::placeholders;
        using namespace muduo;
        using namespace muduo::net;
        using namespace adservice::utility::serialize;
        using namespace adservice::utility::cypher;
        using namespace adservice::utility::url;
        using namespace adservice::server;
        using namespace adservice::types;

        void checkParseClickResponse(protocol::click::ClickResponse& resp,adservice::types::string& data){
            getAvroObject(resp, (const uint8_t *) data.c_str(), data.length());
        }

        void logicWithAvro(const string& data,protocol::click::ClickRequest &clickRequest,adservice::types::string& clickResponseAvroData){
            getAvroObject(clickRequest, (const uint8_t *) data.c_str(), data.length());
            if (clickRequest.cookiesId.empty()) {
                CypherResult128 cookiesResult;
                makeCookies(cookiesResult);
                clickRequest.cookiesId = (char *) cookiesResult.char_bytes;
            }
            DebugMessage("click request,cookiesId:",clickRequest.cookiesId);
            DebugMessage("after parse clickrequest");
            protocol::click::ClickResponse clickResponse;
            clickResponse.cookiesId = clickRequest.cookiesId;
            writeAvroObject(clickResponse, clickResponseAvroData);
            DebugMessage("after generate clickresponse");
            checkParseClickResponse(clickResponse,clickResponseAvroData);
        }

        void parseObjectToLogItem(protocol::click::ClickRequest& clickRequest,protocol::log::LogItem& log){
            log.userId = clickRequest.cookiesId;
            log.userInfo.age = clickRequest.age;
            log.userInfo.interest = clickRequest.age;
            log.adInfo = *(reinterpret_cast<protocol::log::AdInfo *>(&clickRequest.adInfo));
            log.geoInfo = *(reinterpret_cast<protocol::log::GeoInfo *>(&clickRequest.geoInfo));
        }

        void parseObjectToLogItem(ParamMap &paramMap,protocol::log::LogItem &log){ //考虑将临时对象作为static const对象来处理
            char buf[1024];
            adservice::types::string output;
            if(paramMap.find("url")!=paramMap.end()){ //落地页
                adservice::types::string& url = paramMap["url"];
                urlDecode_f(url,output,buf);
                log.adInfo.landingUrl = buf;
            }
            if(paramMap.find("f")!=paramMap.end()){ //ref
                adservice::types::string& f = paramMap["f"];
                urlDecode_f(f,output,buf);
                log.referer = f;
            }
            if(paramMap.find("s")!=paramMap.end()){ //广告位Id
                adservice::types::string& s = paramMap["s"];
                log.adInfo.pid = std::stoi(s);
            }
            if(paramMap.find("r")!=paramMap.end()){ //曝光Id
                adservice::types::string& r = paramMap["r"];
                log.adInfo.imp_id = r;
            }
            if(paramMap.find("d")!=paramMap.end()){ //广告主Id
                adservice::types::string& d = paramMap["d"];
                log.adInfo.advId = std::stoi(d);
            }
            if(paramMap.find("t")!=paramMap.end()){ // 推广计划Id
                adservice::types::string& t = paramMap["t"];
                log.adInfo.cpid = std::stoi(t);
            }
            if(paramMap.find("e")!=paramMap.end()){ // 推广单元Id
                adservice::types::string& e = paramMap["e"];
                log.adInfo.sid = std::stoi(e);
            }
            if(paramMap.find("c")!=paramMap.end()){ // 创意Id
                adservice::types::string& c = paramMap["c"];
                log.adInfo.creativeId = std::stoi(c);
            }
            if(paramMap.find("h")!=paramMap.end()){ // clickId
                adservice::types::string& h = paramMap["h"];
                log.adInfo.clickId = h;
            }
            if(paramMap.find("a")!=paramMap.end()){ //areaId
                adservice::types::string& a = paramMap["a"];
                log.adInfo.areaId = a;
            }
        }

        void calculateLogSize(const protocol::log::LogItem& log){
            long size = 0;
            size+=log.userId.length();
            size+=log.userAgent.length();
            size+=sizeof(log.traceId);
            size+=sizeof(log.timeStamp);
            size+=sizeof(log.reqStatus);
            size+=sizeof(log.reqMethod);
            size+=log.referer.length();
            size+=log.path.length();
            size+=log.pageInfo.length();
            size+=log.deviceInfo.length();
            size+=log.jsInfo.length();
            size+=sizeof(log.userInfo.age);
            size+=sizeof(log.userInfo.interest);
            size+=sizeof(log.userInfo.sex);
            size+=sizeof(log.adInfo.creativeId);
            size+=sizeof(log.adInfo.sid);
            size+=sizeof(log.adInfo.advId);
            size+=sizeof(log.adInfo.bidPrice);
            size+=sizeof(log.adInfo.cost);
            size+=sizeof(log.adInfo.cpid);
            size+=sizeof(log.adInfo.pid);
            size+=sizeof(log.adInfo.unionId);
            size+=log.adInfo.adxid.length();
            size+=log.adInfo.areaId.length();
            size+=log.adInfo.cid.length();
            size+=log.adInfo.clickId.length();
            size+=log.adInfo.imp_id.length();
            size+=log.adInfo.landingUrl.length();
            size+=log.adInfo.mid.length();
            size+=sizeof(log.geoInfo.city);
            size+=sizeof(log.geoInfo.country);
            size+=sizeof(log.geoInfo.district);
            size+=sizeof(log.geoInfo.province);
            size+=sizeof(log.geoInfo.street);
            size+=log.geoInfo.latitude.length();
            size+=log.geoInfo.longitude.length();
            DebugMessage("log Item,raw size:",size);
        }

        bool operator==(const protocol::log::IPInfo& a,const protocol::log::IPInfo& b){
            return a.ipv4==b.ipv4&&a.proxy==b.proxy;
        }

        bool operator==(const protocol::log::AdInfo& a,const protocol::log::AdInfo& b){
            return a.advId==b.advId && a.adxid == b.adxid && a.areaId == b.areaId && a.bidPrice == b.bidPrice
                    && a.cid == b.cid && a.cost == b.cost && a.clickId == b.clickId && a.cpid == b.cpid
                   && a.creativeId == b.creativeId && a.imp_id == b.imp_id && a.landingUrl == b.landingUrl;

        }

        bool operator==(const protocol::log::GeoInfo& a, const protocol::log::GeoInfo& b){
            return a.city == b.city && a.country == b.country && a.district == b.district
                    && a.latitude == b.latitude && a.longitude == b.longitude;
        }

        bool operator==(const protocol::log::LogItem& a,const protocol::log::LogItem& b){
            bool equal = true;
            equal = equal && a.timeStamp == b.timeStamp;
            equal = equal && a.ipInfo == b.ipInfo;
            equal = equal && a.adInfo == b.adInfo;
            equal = equal && a.deviceInfo == b.deviceInfo;
            equal = equal && a.geoInfo == b.geoInfo;
            equal = equal && a.host == b.host;
            equal = equal && a.jsInfo == b.jsInfo;
            equal = equal && a.path == b.path;
            equal = equal && a.referer == b.referer;
            equal = equal && a.logType == b.logType;
            equal = equal && a.reqMethod == b.reqMethod;
            equal = equal && a.reqStatus == b.reqStatus;
            equal = equal && a.userId == b.userId;
            return equal;
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
                    if(logItem==parseLog){
                        DebugMessage("parseback check,content equal,escape4ali safe");
                    }
                }else{
                    DebugMessage("decoded string not equal origin,escape4ali not safe");
                }
            }else{
                DebugMessage("decoded string length not equal origin,escape4ali not safe,",input.length()," ",decode_string.length());
            }
        }

        class HandleClickQueryTask{
        public:
            explicit HandleClickQueryTask(const TcpConnectionPtr& _conn,const adservice::types::string& query,
                const adservice::types::string& cookies):conn(_conn),data(query),userCookies(cookies){
            }
            void operator()(){
                try {
                    ParamMap paramMap;
                    getParam(paramMap,data.c_str()+1);
                    protocol::log::LogItem log;
                    parseObjectToLogItem(paramMap,log);
                    log.logType = protocol::log::LogPhaseType::CLICK;
                    log.reqMethod = true; //true for GET,false for POST
                    log.reqStatus = 302;
                    log.timeStamp = utility::time::getCurrentTimeStampUtc();
                    const muduo::net::InetAddress& peerAddr = conn->peerAddress();
                    log.ipInfo.ipv4=peerAddr.ipNetEndian(); //因为现在的服务器是基于ipv4的,所以只需设置ipv4
                    adservice::types::string userId = extractCookiesParam(COOKIES_MTTY_ID,userCookies);
                    if(userId.empty()){
                        CypherResult128 cookiesResult;
                        makeCookies(cookiesResult);
                        log.userId = (char*)cookiesResult.char_bytes;
                    }else{
                        log.userId = userId;
                    }
                    std::shared_ptr<adservice::types::string> logString = std::make_shared<adservice::types::string>();
                    writeAvroObject(log, *(logString.get()));
#ifdef UNIT_TEST
                    checkAliEscapeSafe(log,*(logString.get()));
#endif
                    // 将日志对象推送到阿里云队列
                    ClickModule clickModule = ClickService::getInstance();
                    if (clickModule.use_count() > 0)
                        clickModule->getLogger()->push(logString);
                    //返回请求
                    Buffer buf;
                    HttpResponse resp(true);
                    if(!log.adInfo.landingUrl.empty()) {
                        resp.setStatusCode(HttpResponse::k302Redirect);
                        resp.addHeader("Location", log.adInfo.landingUrl);
                        resp.setStatusMessage("OK");
                    }else{
                        resp.setStatusCode(HttpResponse::k400BadRequest);
                        resp.setStatusMessage("Error,empty landing url");
                    }
                    resp.setContentType("text/html");
                    resp.addHeader("Server", "Mtty");
                    if(userId.empty()) { //传入的cookies中没有userId,cookies 传出
                        char cookiesString[64];
                        sprintf(cookiesString, "%s=%s;Domain=.%s;Max-Age=2617488000;", COOKIES_MTTY_ID,log.userId.c_str(),COOKIES_MTTY_DOMAIN);//必要时加入Domain
                        resp.addHeader("Set-Cookie", cookiesString);
                    }
                    resp.appendToBuffer(&buf);
                    conn->send(&buf); //这里将异步调用IO线程,进行数据回写
                    conn->shutdown(); //假定都是短链接
                }catch(std::exception& e){
                    LOG_ERROR<<"error occured in HandleClickQueryTask:"<<e.what();
                    HttpResponse resp(true);
                    resp.setStatusCode(HttpResponse::k500ServerError);
                    resp.setStatusMessage("error");
                    resp.setContentType("text/html");
                    resp.addHeader("Server", "Mtty");
                    resp.setBody("error occured in click query");
                    Buffer buf;
                    resp.appendToBuffer(&buf);
                    conn->send(&buf);
                    conn->shutdown();
                }
            }
        private:
            adservice::types::string userCookies;
            adservice::types::string data;
            const TcpConnectionPtr& conn;
        };

        ClickModule ClickService::getInstance(){
            return g_clickService;
        }

        void ClickService::start(){
            executor.start();
            clickLogger->start();
            server->start();
            loop.loop();
        }

        void ClickService::init(int port,int threads,bool logRemote,int loggerThreads){
            clickLogger = adservice::log::LogPusher::getLogger(CLICK_SERVICE_LOGGER,loggerThreads,!logRemote);
            muduo::net::InetAddress addr(static_cast<uint16_t>(port));
            server = std::make_shared<CoreHttpServer>(&loop,addr,"mtty::click_service");
            server->setHttpCallback(std::bind(&ClickService::onRequest,this,_1,_2,_3));
            server->setThreadNum(threads);
        }

        void ClickService::onRequest(const TcpConnectionPtr& conn,const HttpRequest& req, bool isClose) {
            if (req.path() == "/test") {
                try {
                    adservice::types::string data = req.query();
                    adservice::types::string cookies = req.getHeader("Cookie");
                    DebugMessage("in request c,query=", req.query());
                    executor.run(std::bind(HandleClickQueryTask(conn, data,cookies)));
                }catch(std::exception &e){
                    LOG_ERROR<<"error occured in ClickService::onRequest"<<e.what();
                }
            }
            else
            {
                DebugMessage("req.path() not math target!");
                HttpResponse resp(isClose);
                resp.setStatusCode(HttpResponse::k404NotFound);
                resp.setStatusMessage("Not Found");
                resp.setCloseConnection(true);
                Buffer buf;
                resp.appendToBuffer(&buf);
                conn->send(&buf);
                conn->shutdown();
            }

        }

    }

}
