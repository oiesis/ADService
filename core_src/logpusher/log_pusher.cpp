//
// Created by guoze.lin on 16/2/14.
//

#include <cstdlib>
#include <sys/stat.h>
#include <sys/types.h>
#include "log_pusher.h"
#include "muduo/base/Logging.h"
#include "utility/utility.h"
#include "core_threadlocal_manager.h"
#include "atomic.h"

namespace adservice{
    namespace log{

        using namespace adservice::utility::escape;

        struct spinlock LogPusher::lock={0};
        std::map<std::string,LogPusherPtr> LogPusher::logMap;

        LogProducer* LogProducerFactory::createProducer(LogProducerType type,const std::string& loggerName) {
            switch(type){
                case LOG_ALIYUN:
                    DebugMessage("using aliyun log");
                    return new AliyunLogProducer(loggerName);
                    break;
                case LOG_KAFKA:
                    DebugMessage("using kafka log");
                    return new KafkaLogProducer(loggerName);
                    break;
                default:
                    DebugMessage("using local log");
                    return NULL;
            }
        }


        struct LogPushTask{
            LogPushTask(LogProducer* p,std::shared_ptr<adservice::types::string>& l):producer(p),log(l){}
            LogPushTask(LogProducer* p,std::shared_ptr<adservice::types::string>&& l):producer(p),log(l){}
            void operator()(){
#if defined(USE_ALIYUN_LOG)
                std::string ali_escapeString = encode4ali(*(log.get()));
                ons::Message msg(DEFAULT_TOPIC,"click",ali_escapeString);
#else
                Message msg("",*(log.get()));
#endif
                try{
#if defined(USE_ALIYUN_LOG)
                    SendResultONS sendResult = producer->send(msg); //尽管不加宏也可以,但并不希望加一些冗余的类型转换开销
                    DebugMessage("sendResult:",sendResult.getMessageId());
#else
                    SendResult sendResult = producer->send(msg);
                    if(sendResult==SendResult::SEND_ERROR){
                        LogPusherPtr logger = LogPusher::getLogger(MTTY_SERVICE_LOGGER);
                        logger->setWorkMode(true);
                        logger->startRemoteMonitor(msg);
                    }
#endif
                }catch(LogClientException& e){
                    LOG_ERROR << "LogClient error:" << e.GetMsg() << " errorcode:" << e.GetError();
                    LogPusherPtr logger = LogPusher::getLogger(MTTY_SERVICE_LOGGER);
                    logger->setWorkMode(true);
                    logger->startRemoteMonitor(msg);
                }catch(std::exception& e){
                    LOG_ERROR << "error occured in LogPushClickTask,err:"<<e.what();
                }
            }
            LogProducer* producer;
            std::shared_ptr<adservice::types::string> log;
        };

        struct LogPushLocalThreadData{
            FILE* fp;
            long lastTime;
            int logCnt;
            LogPushLocalThreadData():fp(NULL),lastTime(0),logCnt(0){}
            ~LogPushLocalThreadData(){
                if(fp!=NULL){
                    fclose(fp);
                }
            }
            static void destructor(void* ptr){
                if(ptr){
                    delete ((LogPushLocalThreadData*)ptr);
                }
            }
        };

#define HOUR_SECOND     3600
        struct LogPushLocalTask{
            LogPushLocalTask(std::shared_ptr<adservice::types::string>& l):log(l){}
            LogPushLocalTask(std::shared_ptr<adservice::types::string>&& l):log(l){}
            void operator()(){
                pthread_t thread = pthread_self();
                LogPushLocalThreadData* data = (LogPushLocalThreadData*)ThreadLocalManager::getInstance().get(thread);
                if(data==NULL){
                    data = new LogPushLocalThreadData;
                    ThreadLocalManager::getInstance().put(thread,data,&LogPushLocalThreadData::destructor);
                }
                FILE* fp = data->fp;
                long curTime = utility::time::getCurrentTimeStamp();
                bool expired = false;
                if(fp==NULL||(expired=data->lastTime<curTime-HOUR_SECOND)){
                    if(expired){
                        fclose(fp);
                        fp = NULL;
                        DebugMessageWithTime("hourly log cnt:",data->logCnt," of thread ",(long)thread);
                        data->logCnt=0;
                    }
                    if(access("logs",F_OK)==-1){
                        if(mkdir("logs",S_IRWXU|S_IRWXG)<0){
                            LOG_ERROR << "dir log can not be created!";
                            return;
                        }
                    }
                    time_t currentTime;
                    ::time(&currentTime);
                    tm* ltime = localtime(&currentTime);
                    char dirname[50];
                    sprintf(dirname,"logs/%d%02d%02d%02d",1900+ltime->tm_year,ltime->tm_mon+1,ltime->tm_mday,ltime->tm_hour);
                    if(access(dirname,F_OK)==-1){
                        if(mkdir(dirname,S_IRWXU|S_IRWXG)<0){
                            LOG_ERROR << "dir "<<dirname<<" can not be created!";
                        }
                    }
                    char filename[50];
                    sprintf(filename,"%s/click.%d.log",dirname,thread);
                    fp = fopen(filename,"wb+");
                    if(fp==NULL){
                        LOG_ERROR << "file " << filename<<" can not be opened!";
                        return;
                    }
                    data->lastTime = utility::time::getTodayStartTime()+ltime->tm_hour*HOUR_SECOND;
                    data->fp = fp;
                }
                char flag[20]={'\0'};
                sprintf(flag,"mt%d^",log->length());
                fwrite(flag,strlen(flag),1,fp);
                fwrite(log->c_str(),log->length(),1,fp);
                data->logCnt++;
            }
            std::shared_ptr<adservice::types::string> log;
        };


        LogPusherPtr LogPusher::getLogger(const std::string& name,int ifnodefineThreads,bool logLocal){ //fixme: std::map is not thread-safe,risk still holds
            LogPusherPtr log = logMap[name];
            if(log.use_count()==0){
                spinlock_lock(&lock);
                if((log=logMap[name]).use_count()==0) {
                    log = std::make_shared<LogPusher>(name.c_str(),ifnodefineThreads,logLocal);
                    logMap[name] = log;
                }
                spinlock_unlock(&lock);
            }
            return log;
        }

        void LogPusher::removeLogger(const std::string& name){
            spinlock_lock(&lock);
            logMap.erase(name);
            spinlock_unlock(&lock);
        }

        void LogPusher::push(std::shared_ptr<adservice::types::string>& logstring){
            if(!modeLocal) {
#if defined USE_KAFKA_LOG
                //由于现在使用的kafka client api有自己的消息队列机制,所以不需要走logpusher内部消息队列
                SendResult sendResult = producer->send(Message(DEFAULT_KAFKA_TOPIC,*(logstring.get())));
                if(sendResult==SendResult::SEND_ERROR){
                    this->setWorkMode(true);
                    this->startRemoteMonitor(Message(DEFAULT_KAFKA_TOPIC,*(logstring.get())));
                }
#else
                executor.run(std::bind(LogPushTask(producer, logstring)));
#endif
            }else{
                executor.run(std::bind(LogPushLocalTask(logstring)));
            }
        }

        void LogPusher::push(std::shared_ptr<adservice::types::string>&& logstring){
            if(!modeLocal) {
#if defined USE_KAFKA_LOG
                SendResult sendResult =producer->send(Message(DEFAULT_KAFKA_TOPIC,*(logstring.get())));
                if(sendResult==SendResult::SEND_ERROR){
                    this->setWorkMode(true);
                    this->startRemoteMonitor(Message(DEFAULT_KAFKA_TOPIC,*(logstring.get())));
                }
#else
                executor.run(std::bind(LogPushTask(producer, logstring)));
#endif
            }else{
                executor.run(std::bind(LogPushLocalTask(logstring)));
            }
        }

        struct RemoteMonitorThreadParam{
            LogProducer* producer;
            log::Message msg;
            int started;
            RemoteMonitorThreadParam():started(0){}
            RemoteMonitorThreadParam(LogProducer* p,const log::Message& m):producer(p),msg(m),started(0){}
            void init(LogProducer* p,const log::Message& m){
                producer = p;
                msg = m;
            }
        };

        void* monitorRemoteLog(void* param){
            RemoteMonitorThreadParam* _param = (RemoteMonitorThreadParam*)param;
            LogProducer* producer = _param->producer;
            Message& msg = _param->msg;
            int retryTimes = 0;
            while(true) {
                retryTimes++;
                try {
                    SendResult result = producer->send(msg);
#if defined USE_ALIYUN_LOG
                    LogPusher::getLogger(MTTY_SERVICE_LOGGER)->setWorkMode(false);
                    break;
#elif defined USE_KAFKA_LOG
                    if(result == SendResult::SEND_ERROR){
                        if(retryTimes%30==0)
                            LOG_ERROR<<"log client error still exists";
                    }else{
                        LogPusher::getLogger(MTTY_SERVICE_LOGGER)->setWorkMode(false);
                        DebugMessage("log client error recover,continue with remote logging");
                        break;
                    }
                    sleep(5);
                    if(!LogPusher::getLogger(MTTY_SERVICE_LOGGER)->getWorkMode()){
                        DebugMessage("log client error recover,continue with remote logging");
                        break;
                    }
#endif
                } catch (LogClientException &e) {
                    if(retryTimes%30==0)
                        LOG_ERROR<<"log client error still exists,error:"<<e.GetError();
                }
                sleep(2);
            }
            _param->started = 0;
            return NULL;
        }

        void LogPusher::startRemoteMonitor(const log::Message& msg) { //fixme:fix multi log problem by defining param as a class member
            static RemoteMonitorThreadParam param;
            if(param.started)
                return;
            if(!ATOM_CAS(&param.started,0,1))
                return;
            param.init(producer,msg);
            pthread_t monitorThread;
            if(pthread_create(&monitorThread,NULL,&monitorRemoteLog,&param)){
                LOG_ERROR<<"create remote log monitor error";
                return;
            }
            pthread_detach(monitorThread);
        }


    }
}