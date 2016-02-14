//
// Created by guoze.lin on 16/2/2.
//

#include "click_service.h"
#include "utility/utility.h"
#include "protocol/click/click.h"
#include "common/types.h"

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
            explicit HandleClickQueryTask(ClickService* service,const string& query):clickservice(service),data(query){
            }
            operator()(){
                protocol::click::ClickRequest clickRequest;
                getAvroObject(clickRequest,data.c_str(),data.length());
                if(clickRequest.cookiesId.empty()){
                    CypherResult128 cookiesResult;
                    makeCookies(cookiesResult);
                    clickRequest.cookiesId = cookiesResult.bytes;
                }
                // 根据clickRequest生成日志对象
                std::shared_ptr<LogItem> log = std::make_shared<LogItem>();
                //todo:fixme
                // 将日志对象推送到阿里云队列
                clickservice->getLogger().push(MttyMessage(MttyMessage::TYPE_LOG,log));
            }
        private:
            string data;
            ClickService* clickservice;
        };

        void ClickService::start(){
            executor.start();
            clickLogger.start();
            server->start();
            loop.loop();
        }

        void ClickService::init(int port,int threads){
            muduo::net::InetAddress addr(static_cast<uint16_t>(port));
            server = std::make_shared<HttpServer>(&loop,addr,"mtty::click_service");
            server->setHttpCallback(std::bind(&ClickService::onRequest,this,_1,_2));
            server->setThreadNum(threads);
        }

        void ClickService::onRequest(const HttpRequest& req, HttpResponse* resp) {
            DebugMessage("Headers ", req.methodString(), " ",req.path());
            if (req.path() == "/c") {
                resp->setStatusCode(HttpResponse::k200Ok);
                resp->setStatusMessage("OK");
                resp->setContentType("text/html");
                resp->addHeader("Server", "Mtty");
                string now = Timestamp::now().toFormattedString();
                const string& data = req.query();
                executor.run(std::bind(HandleClickQueryTask(this,data)));
                resp->setCloseConnection(true);
            }
            else
            {
                resp->setStatusCode(HttpResponse::k404NotFound);
                resp->setStatusMessage("Not Found");
                resp->setCloseConnection(true);
            }

        }

    }

}
