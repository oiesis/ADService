//
// Created by guoze.lin on 16/3/3.
//

#ifndef ADCORE_CONFIG_TYPES_H
#define ADCORE_CONFIG_TYPES_H

#include "utility.h"
#include <cstdlib>

namespace adservice{
    namespace server{

        using namespace adservice::utility::json;

        struct ServerConfig{
            bool runClick;
            bool isDaemon;
            int loggingLevel;
            static void* parse(const MessageWraper& mw,void* data){
                data = data==NULL? (new ServerConfig):data;
                ServerConfig* c = (ServerConfig*)data;
                c->runClick = mw.getBoolean("load_click", false);
                c->isDaemon = mw.getBoolean("isDaemon",true);
                c->loggingLevel = mw.getInt("logging_level",4);
                return data;
            }
            static void destruct(void* data){
                delete ((ServerConfig*)data);
            }
        };

        struct ClickConfig{
            int clickPort;
            int clickThreads;
            int clickLoggerThreads;
            bool clickLogRemote;
            static void* parse(const MessageWraper& mw,void* data){
                data = data==NULL?(new ClickConfig):data;
                ClickConfig* c = (ClickConfig*)data;
                c->clickPort = mw.getInt("click_port",8808);
                c->clickThreads = mw.getInt("click_threads",24);
                c->clickLoggerThreads = mw.getInt("click_logger_threads",10);
                c->clickLogRemote = mw.getBoolean("click_log_remote",false);
                return data;
            }
            static void destruct(void* data){
                delete ((ClickConfig*)data);
            }
        };


    }
}

#endif //ADCORE_CONFIG_TYPES_H
