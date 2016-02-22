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
            DebugMessage("in check parse back:", resp.cookiesId);
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
            if(paramMap.find("s")!=paramMap.end()){ //广告位Id
                adservice::types::string& s = paramMap["s"];
                log.adInfo.pid = s;
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
                log.adInfo.cpid = t;
            }
            if(paramMap.find("e")!=paramMap.end()){ // 推广单元Id
                adservice::types::string& e = paramMap["e"];
                log.adInfo.sid = e;
            }
            if(paramMap.find("c")!=paramMap.end()){ // 创意Id
                adservice::types::string& c = paramMap["c"];
                log.adInfo.creativeId = c;
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


        class HandleClickQueryTask{
        public:
            explicit HandleClickQueryTask(const TcpConnectionPtr& _conn,const adservice::types::string& query,
                const adservice::types::string& cookies):conn(_conn),data(query),userCookies(cookies){
            }
            void operator()(){
                try {
//                    protocol::click::ClickRequest clickRequest;
//                    adservice::types::string clickResponseAvroData;
//                    logicWithAvro(clickRequest,data,clickResponseAvroData);
                    DebugMessage("cookies:",userCookies);
                    ParamMap paramMap;
                    getParam(paramMap,data.c_str());
                    protocol::log::LogItem log;
                    parseObjectToLogItem(paramMap,log);
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
                    DebugMessage("log generated,log size:",logString->length());
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
                        sprintf(cookiesString, "%s=%s;Max-Age=630720000", COOKIES_MTTY_ID, log.userId.c_str());//必要时加入Domain
                        resp.addHeader("Set-Cookie", cookiesString);
                    }
//                    resp.setBody(clickResponseAvroData.c_str());
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

        void ClickService::init(int port,int threads,bool logRemote){
            clickLogger = adservice::log::LogPusher::getLogger(CLICK_SERVICE_LOGGER);
            clickLogger->setWorkMode(!logRemote);
            muduo::net::InetAddress addr(static_cast<uint16_t>(port));
            server = std::make_shared<CoreHttpServer>(&loop,addr,"mtty::click_service");
            server->setHttpCallback(std::bind(&ClickService::onRequest,this,_1,_2,_3));
            server->setThreadNum(threads);
        }

        void ClickService::onRequest(const TcpConnectionPtr& conn,const HttpRequest& req, bool isClose) {
            DebugMessage("Headers ", req.methodString(), " ",req.path());
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
