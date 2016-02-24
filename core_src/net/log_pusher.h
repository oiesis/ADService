//
// Created by guoze.lin on 16/2/14.
//

#ifndef ADCORE_LOGPUSHER_H
#define ADCORE_LOGPUSHER_H

#include "common/types.h"
#include "utility/utility.h"
#include "alibaba/ONSFactory.h"
#include "alibaba/ONSClientException.h"
#include "spinlock.h"
#include "constants.h"
#include "core_executor.h"

namespace adservice{
    namespace log{

        using namespace adservice::server;
        using namespace adservice::types;
        using namespace ons;

        static constexpr int LOG_QUEUE_SIZE = 1024*1024;
        static const int LOGGER_THREAD_NUM = 100;

        static const char* DEFAULT_PRODUCER_ID = "PID_mtty001";
        static const char* DEFAULT_TOPIC = "adlog";
        static const char* DEFAULT_ACCESS_KEY = "5jaQzkjjARFVFUrE";
        static const char* DEFAULT_SECRET_KEY = "SbFRrY6y1cnSKcdC0QpK1Vkv0QMmTw";


        class LogPusher;
        typedef std::shared_ptr<LogPusher> LogPusherPtr;

        class LogPusher{
        public:
            static LogPusherPtr getLogger(const std::string& name,int ifnodefineThreads =10,bool logLocal = false);

            static void removeLogger(const std::string& name);

        public:
            LogPusher(const char* logger = "log_default",int loggerThreads = LOGGER_THREAD_NUM,bool modeLocal = false):loggerName(logger),
                                                          executor(logger,false,loggerThreads,LOG_QUEUE_SIZE),
                                                          modeLocal(modeLocal)
            {
                factoryInfo.setFactoryProperty(ONSFactoryProperty::ProducerId, DEFAULT_PRODUCER_ID);
                factoryInfo.setFactoryProperty(ONSFactoryProperty::PublishTopics, DEFAULT_TOPIC);
                factoryInfo.setFactoryProperty(ONSFactoryProperty::MsgContent, "input msg content");
                factoryInfo.setFactoryProperty(ONSFactoryProperty::AccessKey, DEFAULT_ACCESS_KEY);
                factoryInfo.setFactoryProperty(ONSFactoryProperty::SecretKey, DEFAULT_SECRET_KEY );
                if(!modeLocal)
                    producer = ONSFactory::getInstance()->createProducer(factoryInfo);
                else
                    producer = NULL;
            }
            ~LogPusher(){
                if(producer!=NULL)
                    producer->shutdown();
                DebugMessage("logger ",this->loggerName, " gone");
            }

            void loadLoggerFactoryProperty(const char* file);

            void reinitProducer(){
                if(producer!=NULL){
                    producer->shutdown();
                }
                producer = ONSFactory::getInstance()->createProducer(factoryInfo);
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
            void setWorkMode(bool workLocal){
                if(!workLocal && modeLocal==false && producer == NULL){
                    producer = ONSFactory::getInstance()->createProducer(factoryInfo);
                }
                modeLocal = workLocal;
            }
            void startRemoteMonitor(ons::Message& msg);

            void push(std::shared_ptr<adservice::types::string>& logstring);
            void push(std::shared_ptr<adservice::types::string>&& logstring);
        private:
        public:
            static struct spinlock lock;
        private:
            static std::map<std::string,LogPusherPtr> logMap;
        private:
            std::string loggerName;
            adservice::server::Executor executor;
            ONSFactoryProperty factoryInfo;
            Producer* producer;
            /// 工作模式,本地文件日志 或 远程日志
            bool modeLocal;
        };
    }
}

#endif //ADCORE_LOGPUSHER_H
