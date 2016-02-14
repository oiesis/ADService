//
// Created by guoze.lin on 16/2/14.
//

#ifndef ADCORE_LOGPUSHER_H
#define ADCORE_LOGPUSHER_H

#include "common/types.h"
#include "utility/utility.h"
#include "alibaba/ONSFactory.h"
#include "alibaba/ONSClientException.h"

namespace adservice{
    namespace log{

        using namespace adservice::server;
        using namespace ons;

        static constexpr int LOG_QUEUE_SIZE = 1024*1024;
        static const int LOGGER_THREAD_NUM = 100;

        static const char* DEFAULT_PRODUCER_ID = "PID_mtty001";
        static const char* DEFAULT_ACCESS_KEY = "5jaQzkjjARFVFUrE";
        static const char* DEFAULT_SECRET_KEY = "SbFRrY6y1cnSKcdC0QpK1Vkv0QMmTw";
        static const char* DEFAULT_TOPIC = "mtty_click";

        class LogPusher{

        public:
            LogPusher(const char* logger = "log_default"):loggerName(logger),
                                                          executor("log_default",false,LOGGER_THREAD_NUM,LOG_QUEUE_SIZE){
                factoryInfo.setFactoryProperty(ONSFactoryProperty::ProducerId, DEFAULT_PRODUCER_ID);
                factoryInfo.setFactoryProperty(ONSFactoryProperty::PublishTopics, DEFAULT_TOPIC);
                factoryInfo.setFactoryProperty(ONSFactoryProperty::MsgContent, "input msg content");
                factoryInfo.setFactoryProperty(ONSFactoryProperty::AccessKey, DEFAULT_ACCESS_KEY);
                factoryInfo.setFactoryProperty(ONSFactoryProperty::SecretKey, DEFAULT_SECRET_KEY );
                producer = ONSFactory::getInstance()->createProducer(factoryInfo);
            }

            ~LogPusher(){
                producer->shutdown();
            }

            void loadLoggerFactoryProperty(const char* file);

            void reinitProducer(){
                if(producer!=NULL){
                    producer->shutdown();
                }
                producer = ONSFactory::getInstance()->createProducer(factoryInfo);
            }

            void start(){
                producer->start();
                executor.start();
            }
            void stop(){
                executor.stop();
            }
            void push(MttyMessage& message);
            void push(MttyMessage&& message);
        private:
            std::string loggerName;
            adservice::server::Executor executor;
            ONSFactoryProperty factoryInfo;
            Producer* producer;
        };
    }
}

#endif //ADCORE_LOGPUSHER_H
