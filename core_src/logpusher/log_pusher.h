//
// Created by guoze.lin on 16/2/14.
//

#ifndef ADCORE_LOGPUSHER_H
#define ADCORE_LOGPUSHER_H

#include "common/types.h"
#include "utility/utility.h"
#include "spinlock.h"
#include "constants.h"
#include "core_executor.h"
#include "aliyun_log_producer.h"
#include "kafka_log_producer.h"
#include "muduo/base/tbb/concurrent_hash_map.h"

namespace adservice{
    namespace log{

        using namespace adservice::server;
        using namespace adservice::types;

        static constexpr int LOG_QUEUE_SIZE = 1024*1024;
        static const int LOGGER_THREAD_NUM = 100;


        class LogPusher;
        typedef std::shared_ptr<LogPusher> LogPusherPtr;

        typedef tbb::concurrent_hash_map<std::string,LogPusherPtr> LogPusherMap;
        typedef LogPusherMap::accessor LogPusherMapAccessor;

        class LogPusher{
        public:
            static LogPusherPtr getLogger(const std::string& name,int ifnodefineThreads =10,bool logLocal = false,const std::string& logConfigKey = CONFIG_LOG);

            static void removeLogger(const std::string& name);

        public:
            LogPusher(const char* logger = "log_default",int loggerThreads = LOGGER_THREAD_NUM,bool modeLocal = false,
            const std::string& logConfigKey = CONFIG_LOG):loggerName(logger),
                                                          executor(logger,false,loggerThreads,LOG_QUEUE_SIZE),
                                                          modeLocal(modeLocal),
                                                          loggerConfigKey(logConfigKey)
            {
                if(!modeLocal)
                    initProducer();
                else
                    producer = NULL;
            }
            ~LogPusher(){
                if(producer!=NULL)
                    delete producer;
                DebugMessage("logger ",this->loggerName, " gone");
            }

            void initProducer(){
#if defined (USE_ALIYUN_LOG)
                producer = LogProducerFactory::createProducer(LogProducerType::LOG_ALIYUN,loggerName,loggerConfigKey);
#elif defined (USE_KAFKA_LOG)
                producer = LogProducerFactory::createProducer(LogProducerType::LOG_KAFKA,loggerName,loggerConfigKey);
#else
                producer = LogProducerFactory::createProducer(LogProducerType::LOG_DEFAULT,loggerName,loggerConfigKey);
#endif
            }

            void reinitProducer(){
                if(producer!=NULL){
                    delete producer;
                }
                initProducer();
            }

            void start(){
                if(!modeLocal&&producer!=NULL) {
                    producer->start();
                }
                executor.start();
            }
            void stop(){
                executor.stop();
            }
            bool getWorkMode(){
                return modeLocal;
            }
            void setWorkMode(bool workLocal){
                if(!workLocal && modeLocal==false && producer == NULL){
                    initProducer();
                }
                modeLocal = workLocal;
            }
            void startRemoteMonitor(const Message& msg);

            void push(std::shared_ptr<adservice::types::string>& logstring);
            void push(std::shared_ptr<adservice::types::string>&& logstring);
        private:
        public:
            static struct spinlock lock;
        private:
            static LogPusherMap logMap;
        private:
            std::string loggerName;
            std::string loggerConfigKey;
            adservice::server::Executor executor;
            LogProducer* producer;
            /// 工作模式,本地文件日志 或 远程日志
            bool modeLocal;
        };
    }
}

#endif //ADCORE_LOGPUSHER_H
