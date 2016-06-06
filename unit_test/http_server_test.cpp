//
// Created by guoze.lin on 16/5/19.
//
#include <muduo/net/http/HttpServer.h>
#include <muduo/net/http/HttpRequest.h>
#include <muduo/net/http/HttpResponse.h>
#include <muduo/net/EventLoop.h>
#include <muduo/base/Logging.h>
#include <iostream>
#include <map>
#include <unistd.h>
#include <signal.h>
#include "common/constants.h"
#include "core_src/core_config_manager.h"
#include "core_src/adselect/core_adselect_manager.h"
#include "core_src/adselect/ad_select_logic.h"
#include "core_src/logic/bid_query_task.h"
#include "core_src/logic/show_query_task.h"
#include "core_src/logic/click_query_task.h"
#include "core_src/logpusher/log_pusher.h"
#include "core_src/core_threadlocal_manager.h"
#include "utility/utility.h"
#include "common/spinlock.h"
#include "protocol/baidu/baidu_price.h"
#include "protocol/tanx/tanx_price.h"
#include "protocol/youku/youku_price.h"
#include "protocol/tencent_gdt/tencent_gdt_price.h"
#ifdef USE_ENCODING_GZIP
#include "muduo/net/ZlibStream.h"
#endif

using namespace muduo;
using namespace muduo::net;
using namespace adservice;
using namespace adservice::server;
using namespace adservice::corelogic;
using namespace adservice::utility::cypher;
using namespace adservice::utility::url;
using namespace adservice::utility::serialize;
using namespace adservice::utility::file;
using namespace adservice::utility::json;
using namespace adservice::adselect;
using namespace adservice::log;


ConfigManager* configManager;
LogPusherPtr serviceLogger;


void onRequest(const HttpRequest& req, HttpResponse* resp)
{
    try{
        if (req.path() == "/v"||req.path()=="/s")
        {
            HandleShowQueryTask showTask(req,*resp);
            showTask();
        }
        else if (req.path() == "/c")
        {
            HandleClickQueryTask clickTask(req,*resp);
            clickTask();
        }else if (req.path().find("bid")!=std::string::npos){
            HandleBidQueryTask bidTask(req,*resp);
            bidTask();
        }else if (req.path() == "/jt.html")
        {
            resp->setStatusCode(HttpResponse::k200Ok);
        }
        else
        {
            resp->setStatusCode(HttpResponse::k404NotFound);
            resp->setStatusMessage("Not Found");
            resp->setCloseConnection(true);
        }
    }catch(std::exception& e){
        resp->setStatusCode(HttpResponse::k500ServerError);
        resp->setStatusMessage("error");
        resp->setContentType("text/html");
        resp->setCloseConnection(true);
    }
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

EventLoop* mainLoop;

/**
 * 当外部kill进程时,进行处理
 */
void handle_sigterm(int sig){
    mainLoop->quit();
}

/**
 * 屏蔽不关心的信号
 */
void signal_ignore() {
    struct sigaction sa;
    sa.sa_handler = SIG_IGN;
    sigaction(SIGPIPE, &sa, 0);
}

void signal_kill(){
    struct sigaction sa;
    sa.sa_handler = handle_sigterm;
    sigaction(SIGTERM,&sa,0);
    sigaction(SIGINT,&sa,0);
    sigaction(SIGHUP,&sa,0);
}
/**
 * 进行相关信号的注册
 */
void dosignals() {
    signal_ignore();
    signal_kill();
}


int main(int argc, char* argv[])
{
    ConfigManager::init();
    configManager = &(ConfigManager::getInstance());
    ServerConfig* serverConfig = (ServerConfig*)configManager->get(CONFIG_SERVICE);
    int httpThreads = serverConfig->coreHttpThreads;
    int port = serverConfig->corePort;
    setLogLevel(serverConfig->loggingLevel);

    LogConfig* logConfig = (LogConfig*)configManager->get(CONFIG_LOG);
    serviceLogger = adservice::log::LogPusher::getLogger(MTTY_SERVICE_LOGGER,
                                                         logConfig->loggerThreads,
                                                         !logConfig->logRemote);
    serviceLogger->start();

    HandleShowQueryTask::loadTemplates();
    HandleBidQueryTask::init();
    EventLoop loop;
    mainLoop = &loop;
    HttpServer server(&loop, InetAddress(port), "dummy");
    server.setHttpCallback(onRequest);
    server.setThreadNum(httpThreads);
    server.start();
    dosignals();
    loop.loop();
    serviceLogger->stop();
    AdSelectManager::release();
    ThreadLocalManager::getInstance().destroy();
    ConfigManager::exit();
    return 0;
}
