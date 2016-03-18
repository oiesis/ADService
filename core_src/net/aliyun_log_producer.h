//
// Created by guoze.lin on 16/2/26.
//

#ifndef ADCORE_ALIYUN_LOG_PRODUCER_H
#define ADCORE_ALIYUN_LOG_PRODUCER_H

#include "log_producer.h"
#include "alibaba/ONSFactory.h"
#include "alibaba/ONSClientException.h"
#include "utility.h"
#include "core_config_manager.h"

namespace adservice{
    namespace log{

        using namespace ons;
        using namespace adservice::server;

        class AliyunLogProducer : public LogProducer{
        public:
            AliyunLogProducer(const std::string& name){
                myName = name;
                configure();
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

            void configure(){
                ConfigManager& configManager = ConfigManager::getInstance();
                LogConfig* logConfig = (LogConfig*)configManager.get(CONFIG_LOG);
                factoryInfo.setFactoryProperty(ONSFactoryProperty::ProducerId, logConfig->aliyunProducerId);
                factoryInfo.setFactoryProperty(ONSFactoryProperty::PublishTopics, logConfig->aliyunTopic);
                factoryInfo.setFactoryProperty(ONSFactoryProperty::MsgContent, "input msg content");
                factoryInfo.setFactoryProperty(ONSFactoryProperty::AccessKey, logConfig->aliyunAccessKey);
                factoryInfo.setFactoryProperty(ONSFactoryProperty::SecretKey, logConfig->aliyunSecretKey );
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
                factoryInfo.setFactoryProperty(ONSFactoryProperty::ProducerId, mw.getString("ProducerId",DEFAULT_ALIYUN_PRODUCER_ID));
                factoryInfo.setFactoryProperty(ONSFactoryProperty::PublishTopics, mw.getString("PublishTopics",DEFAULT_ALIYUN_TOPIC));
                factoryInfo.setFactoryProperty(ONSFactoryProperty::MsgContent, "input msg content");
                factoryInfo.setFactoryProperty(ONSFactoryProperty::AccessKey, mw.getString("AccessKey",DEFAULT_ALIYUN_ACCESS_KEY));
                factoryInfo.setFactoryProperty(ONSFactoryProperty::SecretKey, mw.getString("SecretKey",DEFAULT_ALIYUN_SECRET_KEY));
            }
        private:
            std::string myName;
            ONSFactoryProperty factoryInfo;
            ons::Producer* producer;
        };

    }
}

#endif //ADCORE_ALIYUN_LOG_PRODUCER_H
