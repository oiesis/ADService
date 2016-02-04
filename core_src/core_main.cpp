//
// Created by guoze.lin on 16/1/20.
//

#include "core.h"
#include <iostream>

#ifdef UNIT_TEST
void DebugMessage(const char* str){
    std::cout<<str<<std::endl;
}
#endif

int __main(int argc,char** argv){
    //是否使用高级服务端模型,默认不使用
#ifndef ADVANCED_SERVER_MODEL
    using namespace adservice::server;
    DebugMessage("begin");
    ADService::initClassVar();
    DebugMessage("1");
    ServerConfig  config;
    DebugMessage("begin");
    loadServerConfig(config);
    DebugMessage("load config done");
    ADServicePtr service = ADService::getInstance();
    DebugMessage("service instance created");
    service->initWithConfig(config);
    service->start();
#else
    // 使用erlang actor模式
#endif
    return 0;
}
