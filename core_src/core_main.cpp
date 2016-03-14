//
// Created by guoze.lin on 16/1/20.
//

#include "core.h"
#include <iostream>


int launch_service(){
    //是否使用高级服务端模型,默认不使用
#ifndef ADVANCED_SERVER_MODEL
    using namespace adservice::server;
    ADService::initClassVar();
    ADServicePtr service = ADService::getInstance();
    service->start();
#else
    // 使用erlang actor模式
#endif
    return 0;
}
