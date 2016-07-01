//
// Created by guoze.lin on 16/5/20.
//

#include <iostream>
#include <map>
#include <string>
#include <muduo/net/Buffer.h>
#include <muduo/net/http/HttpRequest.h>
#include <muduo/net/http/HttpResponse.h>
#include <muduo/base/Logging.h>
#include "common/constants.h"
#include "core_src/core_config_manager.h"
#include "core_src/adselect/core_adselect_manager.h"
#include "core_src/adselect/ad_select_logic.h"
#include "core_src/logic/bid_query_task.h"
#include "core_src/logic/show_query_task.h"
#include "core_src/logic/click_query_task.h"
#include "core_src/logic/trace_query_task.h"
#include "core_src/logpusher/log_pusher.h"
#include "core_src/core_threadlocal_manager.h"
#include "core_src/core_ip_manager.h"
#include "utility/utility.h"
#include "common/spinlock.h"
#include "protocol/baidu/baidu_price.h"
#include "protocol/tanx/tanx_price.h"
#include "protocol/youku/youku_price.h"
#include "protocol/tencent_gdt/tencent_gdt_price.h"
#ifdef USE_ENCODING_GZIP
#include "muduo/net/ZlibStream.h"
#endif
#include "AdGlobal.h"
#include "AdServer.h"
#include "AdNetTask.h"
#include "AdUtil.h"
#include "AdControlTask.h"

using namespace std;
using namespace std::placeholders;
using namespace muduo;
using namespace muduo::net;
using namespace adservice;
using namespace adservice::corelogic;
using namespace adservice::server;
using namespace adservice::utility::cypher;
using namespace adservice::utility::url;
using namespace adservice::utility::serialize;
using namespace adservice::utility::file;
using namespace adservice::utility::json;
using namespace adservice::adselect;
using namespace adservice::log;


ConfigManager* configManager;
LogPusherPtr serviceLogger;
LogPusherPtr trackLogger;
AdControl* ctControl;


void buildErrorResponse(AdSession* pSession){
    HttpResponse response(false);
    response.setStatusCode(HttpResponse::k500ServerError);
    Buffer buf;
    response.appendToBuffer(&buf);
    std::string strResponse = buf.retrieveAllAsString();
    pSession->AdSessionWrite(strResponse.data(),strResponse.length());
}


void buildSuccessResponse(AdSession* pSession){
    HttpResponse response(false);
    response.setStatusCode(HttpResponse::k200Ok);
    Buffer buf;
    response.appendToBuffer(&buf);
    std::string strResponse = buf.retrieveAllAsString();
    pSession->AdSessionWrite(strResponse.data(),strResponse.length());
    pSession->AdSessionIdle();
}


class HttpQueryTask : public AdNetTaskInterface{
private:
public:
    int Init(AdTask & ctTask)
    {
        return AD_SUCCESS;
    };

    int DeInit()
    {
        return AD_SUCCESS;
    };

    int Run(AdCache * pCache, AdSession * pSession, AdTask & ctTask)
    {
        if(pCache==NULL) {
            return AD_FAILURE;
        }
        try {
            const char *bufferData = pCache->Get();
            HttpRequest request;
            HttpResponse response(false);
            if(!parseHttpRequest(request, bufferData, pCache->Size())){
                DebugMessageWithTime("parse http request failed");
                buildErrorResponse(pSession);
                return AD_FAILURE;
            }
            //lock session
            pSession->AdSessionActive();

            if (request.path() == "/v" || request.path() == "/s") {
                HandleShowQueryTask showTask(request,response);
                showTask.setLogger(serviceLogger);
                showTask();
            } else if(request.path().find("bid")!=std::string::npos){
                HandleBidQueryTask bidTask(request,response);
                bidTask.setLogger(serviceLogger);
                bidTask();
            } else if (request.path() == "/c") {
                HandleClickQueryTask clickTask(request,response);
                clickTask.setLogger(serviceLogger);
                clickTask();
            } else if(request.path() == "/t"){
                HandleTraceTask traceTask(request,response);
                traceTask.setLogger(trackLogger);
                traceTask();
            }else if (request.path() == "/jt.html") {
                buildSuccessResponse(pSession);
                return AD_SUCCESS;
            } else {
                response.setStatusCode(HttpResponse::k404NotFound);
                response.setStatusMessage("Not Found");
                response.setCloseConnection(true);
            }
            Buffer buf;
            response.appendToBuffer(&buf);
            std::string strResponse = buf.retrieveAllAsString();
            pSession->AdSessionWrite(strResponse.data(),strResponse.length());
            pSession->AdSessionIdle();
        }catch(std::exception& e){
            DebugMessageWithTime("HttpQueryTask::Run EXCEPTION,",e.what());
            buildErrorResponse(pSession);
        }
        return AD_SUCCESS;
    };

    //判断是否收到完整的Http包,如果完整返回length
    int PacketLen(AdCache* pCache)
    {
        char * pBody;
        char * pData;
        int iBody;
        int ret;
        pData =pCache->Get();
        string sValue;
        //check http head
        ret = AdHttp::GetPostUri(*pCache, sValue);
        if (AD_SUCCESS == ret)
        {
            iBody = AdHttp::GetLen(*pCache);
            if(iBody <= 0)
                return AD_FAILURE;

            pBody = AdHttp::GetBody(*pCache);
            if (NULL == pBody)
            {
                return AD_FAILURE;;
            }

            return pBody - pData + iBody;
        }
        return AD_FAILURE;
    };
};

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
    // 简单粗暴地重启服务
    exit(0);
}

void onDebugConfigChange(const std::string& type,void* newData,void* oldData){
    DebugMessageWithTime("DebugConfig modified");
    DebugConfig* oldDebugConfig = (DebugConfig*)oldData;
    DebugConfig* newDebugConfig = (DebugConfig*)newData;

    if(newDebugConfig->dynamicLogLevel!=oldDebugConfig->dynamicLogLevel){
        DebugMessageWithTime("logging level changed,old level:",oldDebugConfig->dynamicLogLevel,",new level:",newDebugConfig->dynamicLogLevel);
        setLogLevel(newDebugConfig->dynamicLogLevel);
    }
    if(newDebugConfig->debugIp!=oldDebugConfig->debugIp){
        DebugMessageWithTime("debug user ip changed,old ip:",oldDebugConfig->debugIp,",new debug ip:",newDebugConfig->debugIp);
    }
    // verbose version,if different print debug info
    if(newDebugConfig->verboseVersion!=oldDebugConfig->verboseVersion){
        DebugMessageWithTime("Verbose Debug Info:");
        DebugMessageWithTime("current debug user IP:",newDebugConfig->debugIp);
    }
}


int main(int argc,char** argv){

    ConfigManager::init();
    configManager = &(ConfigManager::getInstance());
    ServerConfig* serverConfig = (ServerConfig*)configManager->get(CONFIG_SERVICE);
    int httpThreads = serverConfig->coreHttpThreads;
    int port = serverConfig->corePort;

    LogConfig* logConfig = (LogConfig*)configManager->get(CONFIG_LOG);
    serviceLogger = adservice::log::LogPusher::getLogger(MTTY_SERVICE_LOGGER,
                                                         CONFIG_LOG,
                                                         logConfig->loggerThreads,
                                                         !logConfig->logRemote);
    serviceLogger->start();
    if(serverConfig->runTrack){
        LogConfig *trackLogConfig = (LogConfig *) configManager->get(CONFIG_TRACK_LOG);
        trackLogger = adservice::log::LogPusher::getLogger(MTTY_TRACK_LOGGER,
                                                           CONFIG_TRACK_LOG,
                                                           trackLogConfig->loggerThreads,
                                                           !trackLogConfig->logRemote);
        trackLogger->start();
    }

    HandleShowQueryTask::loadTemplates();
    HandleBidQueryTask::init();
    IpManager::init();

    AdServer cServer;
    cout<<"***********Server Init**************\n"<<endl;
    int ret = cServer.AdServerInit(port, httpThreads, 1);
    if(ret != AD_SUCCESS)
    {
        AD_ERROR("AdServer init error\n");
        return ret;
    }

    HttpQueryTask httpTask;
    httpTask.SetName("AdService");
    //create net (server,  work,  write) task
    ret = AdControlNetCreateTask( static_cast<AdNetBase *>(&cServer),
                                  static_cast<AdNetTaskInterface*>(&httpTask),  httpThreads, 1);
    if(ret != AD_SUCCESS)
    {
        AD_ERROR("AdServer Create error\n");
        return ret;
    }

    configManager->registerOnChange(CONFIG_SERVICE,std::bind(&onConfigChange,CONFIG_SERVICE,_1,_2));
    configManager->registerOnChange(CONFIG_LOG,std::bind(&onConfigChange,CONFIG_LOG,_1,_2));
    configManager->registerOnChange(CONFIG_TRACK_LOG,std::bind(&onConfigChange,CONFIG_TRACK_LOG,_1,_2));
    configManager->registerOnChange(CONFIG_ADSELECT,std::bind(&onConfigChange,CONFIG_ADSELECT,_1,_2));
    configManager->registerOnChange(CONFIG_DEBUG,std::bind(&onDebugConfigChange,CONFIG_DEBUG,_1,_2));

    ctControl = &(AdControl::Instance());

    ret = ctControl->OutputInit(1988);
    if(ret != AD_SUCCESS)
    {
        AD_ERROR("Control Output init error\n");
        return ret;
    }
    ctControl->TaskDetect(300);

    serviceLogger->stop();
    if(trackLogger.use_count()>0){
        trackLogger->stop();
    }
    AdSelectManager::release();
    ThreadLocalManager::getInstance().destroy();
    ConfigManager::exit();
    IpManager::destroy();
    return 0;
}
