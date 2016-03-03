//
// Created by guoze.lin on 16/2/26.
//

#include "kafka_log_producer.h"
#include "muduo/base/Logging.h"
#include "log_pusher.h"
#include "constants.h"
#include <exception>

namespace adservice{
    namespace log{

        using namespace muduo;

        void LogDeliverReportCb::dr_cb(RdKafka::Message &message){
            if(message.err()!=ERR_NO_ERROR && !needRecover){ //kafka 发送发生错误
                DebugMessage("error occured in kafka,err",message.errstr()," errCode:",message.err());
                LogPusherPtr logPusher = LogPusher::getLogger(CLICK_SERVICE_LOGGER);
                logPusher->setWorkMode(true);
                const char* payload = (const char*)message.payload();
#if defined(USE_ALIYUN_LOG)
                log::Message msg(message.topic_name(),"",std::string(payload,payload+message.len()));
#else
                log::Message msg(message.topic_name(),std::string(payload,payload+message.len()));
#endif
                logPusher->startRemoteMonitor(msg);
                needRecover = true;
            }else if(message.err()==ERR_NO_ERROR && needRecover){ //kafka 错误恢复
                DebugMessage("kafka error recover,continue to work");
                needRecover = false;
                LogPusherPtr logPusher = LogPusher::getLogger(CLICK_SERVICE_LOGGER);
                logPusher->setWorkMode(false);
            }
        }

        void LogEventCb::event_cb (Event &event){
            switch(event.type()){
                case Event::EVENT_ERROR:
                    LOG_ERROR<<"error occured in kafka,broker:"<<event.broker_name()<<" "<<event.str()<<",errcode:"<<event.err();
                    break;
                case Event::EVENT_LOG:
                    LOG_INFO<<"event log in kafka,"<<event.str();
                    break;
                case Event::EVENT_THROTTLE:
                    LOG_INFO<<"event trottle from kafka broker,broker:"<<event.broker_name()<<" "<<event.str();
                    break;
            }
        }

        void KafkaLogProducer::configure(){
            std::string brokers = DEFAULT_KAFKA_BROKER;
            RdKafka::Conf *conf = RdKafka::Conf::create(RdKafka::Conf::CONF_GLOBAL);
            RdKafka::Conf *tconf = RdKafka::Conf::create(RdKafka::Conf::CONF_TOPIC);
            std::string errstr;
            RdKafka::Conf::ConfResult res;
            if((res=conf->set("metadata.broker.list", brokers, errstr))!=RdKafka::Conf::CONF_OK ||
                    (res = conf->set("event_cb", &eventCb, errstr))!= RdKafka::Conf::CONF_OK ||
                    (res=conf->set("dr_cb", &drCb, errstr))!= RdKafka::Conf::CONF_OK ||
                    (res=conf->set("queue.buffering.max.messages","100000",errstr))!=RdKafka::Conf::CONF_OK||
                    (res=conf->set("message.send.max.retries","3",errstr))!= RdKafka::Conf::CONF_OK ||
                    (res=conf->set("retry.backoff.ms","500",errstr))!=RdKafka::Conf::CONF_OK){
                LOG_ERROR<<"error occured when configuring kafka log producer,"<<errstr;
                throw LogClientException(errstr,-1);
            }
            producer = RdKafka::Producer::create(conf, errstr);
            if (!producer) {
                LOG_ERROR<<"error occured when configuring kafka log producer" << errstr;
                throw LogClientException(errstr,-1);
            }
            topic = RdKafka::Topic::create(producer, topicName, tconf, errstr);
            if (!topic) {
                LOG_ERROR<<"error occured when configuring kafka log producer" << errstr;
                throw LogClientException(errstr,-1);
            }
        }

        SendResult KafkaLogProducer::send(Message& msg){
#if defined(USE_ALIYUN_LOG)
            const char* bytes = msg.getBody().c_str();
            int len = msg.getBody().length();
#else
            const char* bytes = msg.bytes.c_str();
            int len = msg.bytes.length();
#endif
            RdKafka::ErrorCode resp=producer->produce(topic, RdKafka::Topic::PARTITION_UA,
			  RdKafka::Producer::RK_MSG_COPY, (void*)bytes,len,
			  &DEFAULT_KAFKA_KEY, NULL);
            if(resp==RdKafka::ErrorCode::ERR_NO_ERROR){
                return SendResult::SEND_OK;
            }else{
                LOG_ERROR<<"kafka produce log error,error code:"<<resp;
                return SendResult::SEND_ERROR;
            }
        }

        SendResult KafkaLogProducer::send(Message&& msg){
#if defined(USE_ALIYUN_LOG)
            const char* bytes = msg.getBody().c_str();
            int len = msg.getBody().length();
#else
            const char* bytes = msg.bytes.c_str();
            int len = msg.bytes.length();
#endif
            RdKafka::ErrorCode resp=producer->produce(topic, RdKafka::Topic::PARTITION_UA,
                                                      RdKafka::Producer::RK_MSG_COPY,
                                                      (void*)bytes,len,
                                                      &DEFAULT_KAFKA_KEY, NULL);
            if(resp==RdKafka::ErrorCode::ERR_NO_ERROR){
                return SendResult::SEND_OK;
            }else{
                LOG_ERROR<<"kafka produce log error,error code:"<<resp;
                return SendResult::SEND_ERROR;
            }
        }



        void* kafkaTimer(void* param){
            KafkaTimerParam* _param = (KafkaTimerParam*)param;
            RdKafka::Producer* p = _param->producer;
            while(_param->run){
                p->poll(1000);
            }
            return NULL;
        }

        void KafkaLogProducer::start(){
            timerParam.producer = producer;
            timerParam.run = true;
            if(pthread_create(&kafkaTimerThread,NULL,&kafkaTimer,&timerParam)){
                LOG_ERROR<<"create kafka log timer error";
                return;
            }
            pthread_detach(kafkaTimerThread);
        }

        void KafkaLogProducer::shutdown(){
            timerParam.run = false;
            RdKafka::wait_destroyed(5000);
        }

    }
}
