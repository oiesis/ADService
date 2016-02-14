//
// Created by guoze.lin on 16/2/14.
//

#include "log_pusher.h"
#include "muduo/base/logging.h"

namespace adservice{
    namespace log{

        struct LogPushClickTask{
            LogPushClickTask(Producer* p,std::shared_ptr<LogItem> l):producer(p),log(l){}
            operator()(){
                Message msg("mtty_click","tagA","");
                try{
                    SendResultONS sendResult = producer->send(msg);
                }catch(ONSClientException& e){
                    LOG_ERROR << "ONSClient error:" << e.GetMsg() << " errorcode:" << e.GetError()<<std::endl;
                }
            }
            Producer* producer;
            std::shared_ptr<LogItem> log;
        };

        void LogPuser::loadLoggerFactoryProperty(const char* file){
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

        void LogPusher::push(MttyMessage& message){
            executor.run(std::bind(LogPushClickTask(producer,message.log)));
        }

        void LogPusher::push(MttyMessage&& message){
            executor.run(std::bind(LogPushClickTask(producer,message.log)));
        }

    }
}