//
// Created by guoze.lin on 16/3/3.
//

#ifndef ADCORE_CONFIG_TYPES_H
#define ADCORE_CONFIG_TYPES_H

#include "utility.h"
#include "constants.h"
#include <cstdlib>

namespace adservice{
    namespace server{

        using namespace adservice::utility::json;

        //全局服务配置类型
        struct ServerConfig{
            bool runClick;
            bool runShow;
            bool runBid;
            bool runView;
            bool runTrack;
            bool isDaemon;
            int corePort;
            int coreHttpThreads;
            int loggingLevel;
            static void* parse(const MessageWraper& mw,void* data){
                data = data==NULL? (new ServerConfig):data;
                ServerConfig* c = (ServerConfig*)data;
                c->runClick = mw.getBoolean("load_click", false);
                c->runShow = mw.getBoolean("load_show",false);
                c->runBid = mw.getBoolean("load_bid",false);
                c->runView = mw.getBoolean("load_view",false);
                c->runTrack = mw.getBoolean("load_track",false);
                c->corePort = mw.getInt("core_port",80);
                c->coreHttpThreads = mw.getInt("core_http_threads",24);
                c->isDaemon = mw.getBoolean("isDaemon",true);
                c->loggingLevel = mw.getInt("logging_level",4);
                return data;
            }
            static void destruct(void* data){
                delete ((ServerConfig*)data);
            }
        };

        //点击模块配置类型
        struct ClickConfig{
            int clickPort;
            int httpthreads;
            int clickLoggerThreads;
            bool clickLogRemote;
            static void* parse(const MessageWraper& mw,void* data){
                data = data==NULL?(new ClickConfig):data;
                ClickConfig* c = (ClickConfig*)data;
                c->clickPort = mw.getInt("click_port",8808);
                c->httpthreads = mw.getInt("click_http_threads",24);
                c->clickLoggerThreads = mw.getInt("click_logger_threads",10);
                c->clickLogRemote = mw.getBoolean("click_log_remote",false);
                return data;
            }
            static void destruct(void* data){
                delete ((ClickConfig*)data);
            }
        };

        //日志配置类型
        struct LogConfig{
            std::string kafkaBroker;
            std::string kafkaTopic;
            std::string kafkaKey;
            std::string aliyunProducerId;
            std::string aliyunTopic;
            std::string aliyunAccessKey;
            std::string aliyunSecretKey;
            static void* parse(const MessageWraper& mw,void* data){
                data = data==NULL?(new LogConfig):data;
                LogConfig* c = (LogConfig*)data;
                c->kafkaBroker = mw.getString("kafka_broker",DEFAULT_KAFKA_BROKER);
                c->kafkaTopic = mw.getString("kafka_topic",DEFAULT_KAFKA_TOPIC);
                c->kafkaKey = mw.getString("kafka_key",DEFAULT_KAFKA_KEY);
                c->aliyunProducerId = mw.getString("aliyun_producer_id",DEFAULT_ALIYUN_PRODUCER_ID);
                c->aliyunTopic = mw.getString("aliyun_topic",DEFAULT_ALIYUN_TOPIC);
                c->aliyunAccessKey = mw.getString("aliyun_access_key",DEFAULT_ALIYUN_ACCESS_KEY);
                c->aliyunSecretKey = mw.getString("aliyun_secret_key",DEFAULT_ALIYUN_SECRET_KEY);
                return data;
            }
            static void destruct(void* data){
                delete ((LogConfig*)data);
            }
        };

        struct ADSelectConfig{
            std::string entryNode;
            int entryPort;
            std::string authorization;
            static void* parse(const MessageWraper& mw,void* data){
                data = data==NULL? (new ADSelectConfig):data;
                ADSelectConfig* c = (ADSelectConfig*)data;
                c->entryNode = mw.getString("entry_node",DEFAULT_ADSELECT_NODE);
                c->entryPort = mw.getInt("entry_port",DEFAULT_ADSELECT_PORT);
                c->authorization = mw.getString("auth",DEFAULT_AUTHORIZATION);
                return data;
            }
            static void destruct(void* data){
                delete ((ADSelectConfig*)data);
            }
        };


    }
}

#endif //ADCORE_CONFIG_TYPES_H
