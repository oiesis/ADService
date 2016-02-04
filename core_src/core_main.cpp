//
// Created by guoze.lin on 16/1/20.
//

#include "core.h"
#include <iostream>

void DebugMessage(const char* str){
    std::cout<<str<<std::endl;
}

int __main(int argc,char** argv){
    //是否使用高级服务端模型,默认不使用
#ifndef ADVANCED_SERVER_MODEL
    using namespace adservice::server;
    ADService::initClassVar();
    ServerConfig  config;
    loadServerConfig(config);
    DebugMessage("load config done");
    ADServicePtr service = ADService::getInstance();
    service->initWithConfig(config);
    service->start();
#else
    // 使用erlang actor模式
#endif
    return 0;
}