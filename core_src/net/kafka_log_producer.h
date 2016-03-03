//
// Created by guoze.lin on 16/2/26.
//

#ifndef ADCORE_KAFKA_LOG_PRODUCER_H
#define ADCORE_KAFKA_LOG_PRODUCER_H

#include "log_producer.h"
#include "functions.h"
#include "librdkafka/rdkafkacpp.h"
#include <unistd.h>

namespace adservice{
    namespace log{

        using namespace RdKafka;


        static const std::string DEFAULT_KAFKA_BROKER = "192.168.31.147";
        static const std::string DEFAULT_KAFKA_TOPIC = "mt-new-click";
        static const std::string DEFAULT_KAFKA_KEY = "click";

        struct KafkaTimerParam{
            RdKafka::Producer* producer;
            volatile bool run;
            KafkaTimerParam(){
                producer = NULL;
                run = false;
            }
        };

        class LogDeliverReportCb : public DeliveryReportCb{
        public:
            LogDeliverReportCb():needRecover(false){}
            void dr_cb(RdKafka::Message &message);
        private:
            bool needRecover;
        };

        class LogEventCb : public EventCb{
            void event_cb (Event &event);
        };

        class KafkaLogProducer : public LogProducer{
        public:
            KafkaLogProducer(){
              topicName = DEFAULT_KAFKA_TOPIC;
              configure();
            }
            ~KafkaLogProducer(){
                shutdown();
            }
            void configure();
            void start();
            void shutdown();
            SendResult send(Message& msg);
            SendResult send(Message&& msg);
            void setTopic(const std::string& t){topicName = t;}
            void setDeliverReportCallback(const LogDeliverReportCb&& cb){drCb = cb;}
            void setErrorCallback(const LogEventCb&& cb){eventCb = cb;}
        private:
            RdKafka::Producer* producer;
            Topic* topic;
            std::string topicName;
            LogEventCb eventCb;
            LogDeliverReportCb drCb;
            pthread_t kafkaTimerThread;
            KafkaTimerParam timerParam;
        };

    }
}

#endif //ADCORE_KAFKA_LOG_PRODUCER_H