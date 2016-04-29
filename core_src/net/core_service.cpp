//
// Created by guoze.lin on 16/2/2.
//

#include <signal.h>
#include "core_service.h"
#include "utility/utility.h"
#include "protocol/log/log.h"
#include "common/types.h"
#include "core_config_manager.h"
#include "click_query_task.h"
#include "show_query_task.h"
#include <exception>

extern adservice::corelogic::CoreModule g_coreService;

namespace adservice{

    namespace corelogic{

        using namespace std::placeholders;
        using namespace muduo;
        using namespace muduo::net;
        using namespace adservice::utility::serialize;
        using namespace adservice::utility::cypher;
        using namespace adservice::utility::url;
        using namespace adservice::utility::hash;
        using namespace adservice::utility::file;
        using namespace adservice::adselect;
        using namespace adservice::server;
        using namespace adservice::types;
        using namespace rapidjson;

        void handle_sigsegv(int signal){
            DebugMessage("segment fault detected");
            if(g_coreService.use_count()==0)
                exit(0);
            g_coreService->setNeedRestart();
            g_coreService->stop();
        }

        void signal_sigsegv(){
            struct sigaction sa;
            sa.sa_handler = handle_sigsegv;
            sa.sa_flags = SA_SIGINFO;
            sigaction(SIGSEGV, &sa, 0);
        }

        CoreModule CoreService::getInstance(){
            return g_coreService;
        }

        void CoreService::start(){
            executor.start();
            serviceLogger->start();
            server->start();
            loop.loop();
        }

        void setLogLevel(int logLevel){
            switch(logLevel){
                case 1:
                    muduo::Logger::setLogLevel(muduo::Logger::LogLevel::DEBUG);
                    break;
                case 2:
                    muduo::Logger::setLogLevel(muduo::Logger::LogLevel::INFO);
                    break;
                case 3:
                    muduo::Logger::setLogLevel(muduo::Logger::LogLevel::WARN);
                    break;
                case 4:
                    muduo::Logger::setLogLevel(muduo::Logger::LogLevel::ERROR);
                    break;
                case 5:
                    muduo::Logger::setLogLevel(muduo::Logger::LogLevel::FATAL);
                    break;
                default:
                    break;
            }
        }

        void onConfigChange(const std::string& type,void* newData,void* oldData){
            DebugMessageWithTime("config ",type," modified");
            if(g_coreService.use_count()==0)
                return;
            // 简单粗暴地重启服务
            g_coreService->setNeedRestart();
            g_coreService->stop();
        }

        void CoreService::init() {
            ConfigManager& configManager = ConfigManager::getInstance();
            ServerConfig* serverConfig = (ServerConfig*)configManager.get(CONFIG_SERVICE);
            int httpThreads = serverConfig->coreHttpThreads;
            int port = serverConfig->corePort;
            setLogLevel(serverConfig->loggingLevel);
            //点击逻辑初始化相关
            if(serverConfig->runClick) {
                ClickConfig* clickConfig = (ClickConfig*)configManager.get(CONFIG_CLICK);
                httpThreads = std::max(httpThreads,clickConfig->httpthreads);
                configManager.registerOnChange(CONFIG_CLICK,std::bind(&onConfigChange,CONFIG_CLICK,_1,_2));
            }
            // 初始化 logger
            LogConfig* logConfig = (LogConfig*)configManager.get(CONFIG_LOG);
            serviceLogger = adservice::log::LogPusher::getLogger(MTTY_SERVICE_LOGGER,
                                                               logConfig->loggerThreads,
                                                               !logConfig->logRemote);
            // 初始化http server
            muduo::net::InetAddress addr(static_cast<uint16_t>(port));
            server = std::make_shared<CoreHttpServer>(&loop,addr,"mtty::core_service");
            server->setHttpCallback(std::bind(&CoreService::onRequest,this,_1,_2,_3));
            server->setThreadNum(httpThreads);
            configManager.registerOnChange(CONFIG_SERVICE,std::bind(&onConfigChange,CONFIG_SERVICE,_1,_2));
            configManager.registerOnChange(CONFIG_LOG,std::bind(&onConfigChange,CONFIG_LOG,_1,_2));
            configManager.registerOnChange(CONFIG_ADSELECT,std::bind(&onConfigChange,CONFIG_ADSELECT,_1,_2));
        }


        void doClick(CoreService* service,const TcpConnectionPtr& conn,const HttpRequest& req,bool isClose){
            try {
                //DebugMessage("in request c,query=", req.query());
                service->getExecutor().run(std::bind(HandleClickQueryTask(conn,req)));
            }catch(std::exception &e){
                LOG_ERROR<<"error occured in ClickService::onRequest"<<e.what();
            }
        }

        void doShow(CoreService* service,const TcpConnectionPtr& conn,const HttpRequest& req,bool isClose){
            try{
                //DebugMessage("in request v,query=",req.query());
                service->getExecutor().run(std::bind(HandleShowQueryTask(conn,req)));
            }catch(std::exception &e){
                LOG_ERROR<<"error occured in ClickService::onRequest"<<e.what();
            }
        }

        void CoreService::onRequest(const TcpConnectionPtr& conn,const HttpRequest& req, bool isClose) {
            //todo:改成table dispatcher
            if(req.path().length()>URL_LONG_REQUEST_THRESH){
                DebugMessage("Received Long Request,",req.path().length(),",input:",req.path());
            }
            if (req.path() == "/v" || req.path() == "/s") { //show
                doShow(this,conn,req,isClose);
            } else if(req.path() == "/c"){ //click
                doClick(this,conn,req,isClose);
            } else if(req.path() == "/jt.html"){ //http健康检查
                HttpResponse resp(false);
                resp.setStatusCode(HttpResponse::k200Ok);
                Buffer buf;
                resp.appendToBuffer(&buf);
                conn->send(&buf);
#ifdef USE_SHORT_CONN
                conn->shutdown();
#endif
            }
            else // 404
            {
                DebugMessage("req.path() not match target!",req.path());
                HttpResponse resp(isClose);
                resp.setStatusCode(HttpResponse::k404NotFound);
                resp.setStatusMessage("Not Found");
                Buffer buf;
                resp.appendToBuffer(&buf);
                conn->send(&buf);
#ifdef USE_SHORT_CONN
                conn->shutdown();
#endif
            }

        }

    }

}
