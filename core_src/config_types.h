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
            bool checkIdleConnection;
            bool isDaemon;
            int corePort;
            int coreHttpThreads;
            int loggingLevel;
            int idleConnectionTimeout;
            static void* parse(const rapidjson::Document& mw,void* data){
                data = data==NULL? (new ServerConfig):data;
                ServerConfig* c = (ServerConfig*)data;
                c->runClick = getField(mw,"load_click",false);
                c->runShow = getField(mw,"load_show",false);
                c->runBid = getField(mw,"load_bid",false);
                c->runView = getField(mw,"load_view",false);
                c->runTrack = getField(mw,"load_track",false);
                c->corePort = getField(mw,"core_port",80);
                c->coreHttpThreads = getField(mw,"core_http_threads",24);
                c->isDaemon = getField(mw,"isDaemon",true);
                c->loggingLevel = getField(mw,"logging_level",4);
                c->checkIdleConnection = getField(mw,"check_idle_connection",false);
                c->idleConnectionTimeout = getField(mw,"idle_connection_timeout",HTTP_IDLE_MAX_SECOND);
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
            static void* parse(const rapidjson::Document& mw,void* data){
                data = data==NULL?(new ClickConfig):data;
                ClickConfig* c = (ClickConfig*)data;
                c->clickPort = getField(mw,"click_port",8808);
                c->httpthreads = getField(mw,"click_http_threads",24);
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
            std::string kafkaMQMaxSize;
            std::string aliyunProducerId;
            std::string aliyunTopic;
            std::string aliyunAccessKey;
            std::string aliyunSecretKey;
            int loggerThreads;
            bool logRemote;
            static void* parse(const rapidjson::Document& mw,void* data){
                data = data==NULL?(new LogConfig):data;
                LogConfig* c = (LogConfig*)data;
                c->loggerThreads = getField(mw,"logger_threads",10);
                c->logRemote = getField(mw,"log_remote",false);
                c->kafkaBroker = getField(mw,"kafka_broker",DEFAULT_KAFKA_BROKER);
                c->kafkaTopic = getField(mw,"kafka_topic",DEFAULT_KAFKA_TOPIC);
                c->kafkaKey = getField(mw,"kafka_key",DEFAULT_KAFKA_KEY);
                c->kafkaMQMaxSize = getField(mw,"kafka_mqmaxsize",DEFAULT_KAFKA_MQSIZE_STR);
                c->aliyunProducerId = getField(mw,"aliyun_producer_id",DEFAULT_ALIYUN_PRODUCER_ID);
                c->aliyunTopic = getField(mw,"aliyun_topic",DEFAULT_ALIYUN_TOPIC);
                c->aliyunAccessKey = getField(mw,"aliyun_access_key",DEFAULT_ALIYUN_ACCESS_KEY);
                c->aliyunSecretKey = getField(mw,"aliyun_secret_key",DEFAULT_ALIYUN_SECRET_KEY);
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
            static void* parse(const rapidjson::Document& mw,void* data){
                data = data==NULL? (new ADSelectConfig):data;
                ADSelectConfig* c = (ADSelectConfig*)data;
                c->entryNode = getField(mw,"entry_node",DEFAULT_ADSELECT_NODE);
                c->entryPort = getField(mw,"entry_port",DEFAULT_ADSELECT_PORT);
                c->authorization = getField(mw,"auth",DEFAULT_AUTHORIZATION);
                return data;
            }
            static void destruct(void* data){
                delete ((ADSelectConfig*)data);
            }
        };

        struct DebugConfig{
            int dynamicLogLevel;
            int verboseVersion;
            int fd;
            std::string debugIp;
            std::string debugUseragent;
            std::string debugCookies;
            static void* parse(const rapidjson::Document& mw,void* data){
                data = data==NULL? (new DebugConfig):data;
                DebugConfig* c = (DebugConfig*)data;
                c->dynamicLogLevel = getField(mw,"dynamic_log_level",3);
                c->verboseVersion = getField(mw,"verbose_version",0);
                c->fd = getField(mw,"debug_fd",-1);
                c->debugIp = getField(mw,"debug_ip","");
                c->debugUseragent = getField(mw,"debug_ua","");
                c->debugCookies = getField(mw,"debug_cookies","");
                return data;
            }
            static void destruct(void* data){
                delete ((DebugConfig*)data);
            }
        };

        struct AerospikeConfig {
            struct Connection {
                std::string host;
                int port;
            };

            std::vector<Connection> connections;
            std::string nameSpace;

            static void * parse(const rapidjson::Document & mw, void * data)
            {
                data = data == nullptr ? (new AerospikeConfig) : data;
                auto * c = (AerospikeConfig *)data;

                c->nameSpace = getField(mw, "nameSpace", "mt");

                auto connIt = mw.FindMember("connections");
                if (connIt != mw.MemberEnd()) {
                    if (connIt->value.IsArray()) {
                        for (auto it = connIt->value.Begin(); it != connIt->value.End(); ++it) {
                            auto hostIt = it->FindMember("host");
                            auto portIt = it->FindMember("port");
                            if (hostIt != it->MemberEnd() && hostIt->value.IsString() &&
                                portIt != it->MemberEnd() && portIt->value.IsNumber() && portIt->value.IsInt()) {
                                c->connections.emplace_back(Connection { .host = hostIt->value.GetString(), .port = portIt->value.GetInt() });
                            }
                        }
                    }
                }

                return data;
            }

            static void destruct(void * data)
            {
                delete ((AerospikeConfig *)data);
            }
        };
    }
}

#endif //ADCORE_CONFIG_TYPES_H
