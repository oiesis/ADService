//
// Created by guoze.lin on 16/2/26.
//

#ifndef ADCORE_ALIYUN_LOG_PRODUCER_H
#define ADCORE_ALIYUN_LOG_PRODUCER_H

#include "log_producer.h"
#include "alibaba/ONSFactory.h"
#include "alibaba/ONSClientException.h"
#include "utility.h"

namespace adservice{
    namespace log{

        using namespace ons;

        static const char* DEFAULT_PRODUCER_ID = "PID_mtty001";
        static const char* DEFAULT_TOPIC = "adlog";
        static const char* DEFAULT_ACCESS_KEY = "5jaQzkjjARFVFUrE";
        static const char* DEFAULT_SECRET_KEY = "SbFRrY6y1cnSKcdC0QpK1Vkv0QMmTw";

        class AliyunLogProducer : public LogProducer{
        public:
            AliyunLogProducer(){
                factoryInfo.setFactoryProperty(ONSFactoryProperty::ProducerId, DEFAULT_PRODUCER_ID);
                factoryInfo.setFactoryProperty(ONSFactoryProperty::PublishTopics, DEFAULT_TOPIC);
                factoryInfo.setFactoryProperty(ONSFactoryProperty::MsgContent, "input msg content");
                factoryInfo.setFactoryProperty(ONSFactoryProperty::AccessKey, DEFAULT_ACCESS_KEY);
                factoryInfo.setFactoryProperty(ONSFactoryProperty::SecretKey, DEFAULT_SECRET_KEY );
                producer = ONSFactory::getInstance()->createProducer(factoryInfo);
            }
            ~AliyunLogProducer(){
                if(producer!=NULL)
                    producer->shutdown();
            }
            SendResult send(log::Message& msg){
#if not defined(USE_ALIYUN_LOG)
                ons::Message m(msg.topics,"",msg.bytes);
                producer->send(m);
                return SendResult::SEND_OK;
#else
                return SendResult(producer->send(msg));
#endif
            }
            SendResult send(log::Message&& msg){
#if not defined(USE_ALIYUN_LOG)
                ons::Message m(msg.topics,"",msg.bytes);
                producer->send(m);
                return SendResult::SEND_OK;
#else
                ons::Message m(msg.getTopic(),msg.getTag(),msg.getBody());
                return SendResult(producer->send(m));
#endif
            }
            void start(){
                producer->start();
            }
            void shutdown(){
                producer->shutdown();
                producer = NULL;
            }
            void loadLoggerFactoryProperty(const char* file){
                using namespace utility::json;
                MessageWraper mw;
                bool bSuccess = parseJsonFile(file,mw);
                if(!bSuccess){
                    DebugMessage("failed to read json file");
                }
                factoryInfo.setFactoryProperty(ONSFactoryProperty::ProducerId, mw.getString("ProducerId",DEFAULT_PRODUCER_ID));
                factoryInfo.setFactoryProperty(ONSFactoryProperty::PublishTopics, mw.getString("PublishTopics",DEFAULT_TOPIC));
                factoryInfo.setFactoryProperty(ONSFactoryProperty::MsgContent, "input msg content");
                factoryInfo.setFactoryProperty(ONSFactoryProperty::AccessKey, mw.getString("AccessKey",DEFAULT_ACCESS_KEY));
                factoryInfo.setFactoryProperty(ONSFactoryProperty::SecretKey, mw.getString("SecretKey",DEFAULT_SECRET_KEY));
            }
        private:
            ONSFactoryProperty factoryInfo;
            ons::Producer* producer;
        };

    }
}

#endif //ADCORE_ALIYUN_LOG_PRODUCER_H
