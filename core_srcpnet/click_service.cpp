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
        using namespace adservice::server;


        class HandleClickQueryTask{
        public:
            explicit HandleClickQueryTask(const TcpConnectionPtr& _conn,const muduo::string& query):conn(_conn),data(query.c_str()){
            }
            void operator()(){
                try {
                    protocol::click::ClickRequest clickRequest;
                    getAvroObject(clickRequest, (const uint8_t *) data.c_str(), data.length());
                    if (clickRequest.cookiesId.empty()) {
                        CypherResult128 cookiesResult;
                        makeCookies(cookiesResult);
                        clickRequest.cookiesId = (char *) cookiesResult.bytes;
                    }
                    protocol::click::ClickResponse clickResponse;
                    clickResponse.cookiesId = clickRequest.cookiesId;
                    std::string clickResponseAvroData;
                    writeAvroObject(clickResponse, clickResponseAvroData);
                    // 根据clickRequest生成日志对象
                    protocol::log::LogItem log;
                    log.userId = clickRequest.cookiesId;
                    log.userInfo.age = clickRequest.age;
                    log.userInfo.interest = clickRequest.age;
                    log.adInfo = *(reinterpret_cast<protocol::log::AdInfo *>(&clickRequest.adInfo));
                    log.geoInfo = *(reinterpret_cast<protocol::log::GeoInfo *>(&clickRequest.geoInfo));
                    std::shared_ptr<std::string> logString = std::make_shared<std::string>();
                    writeAvroObject(log, *(logString.get()));
                    // 将日志对象推送到阿里云队列
                    ClickModule clickModule = ClickService::getInstance();
                    if (clickModule.use_count() > 0)
                        clickModule->getLogger()->push(logString);
                    //返回请求
                    Buffer buf;
                    HttpResponse resp(true);
                    resp.setStatusCode(HttpResponse::k200Ok);
//                    resp.setStatusCode(HttpResponse::k302);
                    resp.setStatusMessage("OK");
                    resp.setContentType("text/html");
                    resp.addHeader("Server", "Mtty");
                    resp.addHeader("Location","http://www.mtty.com");
//                    resp.setBody(clickResponseAvroData.c_str());
                    resp.appendToBuffer(&buf);
                    conn->send(&buf); //这里将异步调用IO线程,进行数据回写
                    conn->shutdown(); //假定都是短链接
                }catch(std::exception& e){
                    LOG_ERROR<<"error occured in HandleClickQueryTask:"<<e.what();
                    HttpResponse resp(true);
                    resp.setStatusCode(HttpResponse::k500ServerErr);
                    resp.setStatusMessage("error");
                    resp.setContentType("text/html");
                    resp.addHeader("Server", "Mtty");
                    Buffer buf;
                    resp.appendToBuffer(&buf);
                    conn->send(&buf);
                    conn->shutdown();
                }
            }
        private:
            std::string data;
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
            if (req.path() == "/c") {
                const muduo::string& data = req.query();
                DebugMessage("in request c,query=",req.query().c_str());
                executor.run(std::bind(HandleClickQueryTask(conn,data)));
            }
            else
            {
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
