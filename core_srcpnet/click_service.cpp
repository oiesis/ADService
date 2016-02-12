//
// Created by guoze.lin on 16/2/2.
//

#include "click_service.h"

namespace adservice{

    namespace click{

        using namespace std::placeholders;
        using namespace muduo;
        using namespace muduo::net;

        void ClickService::start(){
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
            std::cout << "Headers " << req.methodString() << " " << req.path() << std::endl;
//            if (true) {
//                const std::map<string, string> &headers = req.headers();
//                for (std::map<string, string>::const_iterator it = headers.begin();
//                     it != headers.end();
//                     ++it) {
//                    std::cout << it->first << ": " << it->second << std::endl;
//                }
//            }

            if (req.path() == "/c") {
                resp->setStatusCode(HttpResponse::k200Ok);
                resp->setStatusMessage("OK");
                resp->setContentType("text/html");
                resp->addHeader("Server", "Mtty");
                string now = Timestamp::now().toFormattedString();
                resp->setBody("<html><head><title>This is title</title></head>"
                                      "<body><h1>Hello</h1>Now is " + now +
                              "</body></html>");
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
