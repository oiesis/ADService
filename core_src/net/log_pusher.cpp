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


        struct LogPushClickTask{
            LogPushClickTask(LogProducer* p,std::shared_ptr<adservice::types::string>& l):producer(p),log(l){}
            LogPushClickTask(LogProducer* p,std::shared_ptr<adservice::types::string>&& l):producer(p),log(l){}
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
#endif
                }catch(LogClientException& e){
                    LOG_ERROR << "LogClient error:" << e.GetMsg() << " errorcode:" << e.GetError();
                    LogPusherPtr logger = LogPusher::getLogger(CLICK_SERVICE_LOGGER);
                    logger->setWorkMode(true);
                    logger->startRemoteMonitor(msg);
                }catch(std::exception& e){
                    LOG_ERROR << "error occured in LogPushClickTask,err:"<<e.what();
                }
            }
            LogProducer* producer;
            std::shared_ptr<adservice::types::string> log;
        };

        struct LogPushClickLocalThreadData{
            FILE* fp;
            long lastTime;
            LogPushClickLocalThreadData():fp(NULL),lastTime(0){}
            ~LogPushClickLocalThreadData(){
                if(fp!=NULL){
                    fclose(fp);
                }
            }
            static void destructor(void* ptr){
                if(ptr){
                    delete ((LogPushClickLocalThreadData*)ptr);
                }
            }
        };


        struct LogPushClickLocalTask{
            LogPushClickLocalTask(std::shared_ptr<adservice::types::string>& l):log(l){}
            LogPushClickLocalTask(std::shared_ptr<adservice::types::string>&& l):log(l){}
            void operator()(){
                pthread_t thread = pthread_self();
                LogPushClickLocalThreadData* data = (LogPushClickLocalThreadData*)ThreadLocalManager::getInstance().get(thread);
                if(data==NULL){
                    data = new LogPushClickLocalThreadData;
                    ThreadLocalManager::getInstance().put(thread,data,&LogPushClickLocalThreadData::destructor);
                }
                FILE* fp = data->fp;
                long curTime = utility::time::getCurrentTimeStamp();
                bool expired = false;
                if(fp==NULL||(expired=data->lastTime<curTime-DAY_SECOND)){
                    if(expired){
                        fclose(fp);
                        fp = NULL;
                    }
                    if(access("log",F_OK)==-1){
                        if(mkdir("log",S_IRWXU|S_IRWXG)<0){
                            LOG_ERROR << "dir log can not be created!";
                            return;
                        }
                    }
                    time_t currentTime;
                    ::time(&currentTime);
                    tm* ltime = localtime(&currentTime);
                    char dirname[50];
                    sprintf(dirname,"log/%d%02d%02d",1900+ltime->tm_year,ltime->tm_mon+1,ltime->tm_mday);
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
                    data->lastTime = utility::time::getTodayStartTime();
                    data->fp = fp;
                }
                char flag[20]={'\0'};
                sprintf(flag,"mt%d^",log->length());
                fwrite(flag,strlen(flag),1,fp);
                fwrite(log->c_str(),log->length(),1,fp);
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
                producer->send(Message(DEFAULT_KAFKA_TOPIC,*(logstring.get())));
#else
                executor.run(std::bind(LogPushClickTask(producer, logstring)));
#endif
            }else{
                executor.run(std::bind(LogPushClickLocalTask(logstring)));
            }
        }

        void LogPusher::push(std::shared_ptr<adservice::types::string>&& logstring){
            if(!modeLocal) {
#if defined USE_KAFKA_LOG
                producer->send(Message(DEFAULT_KAFKA_TOPIC,*(logstring.get())));
#else
                executor.run(std::bind(LogPushClickTask(producer, logstring)));
#endif
            }else{
                executor.run(std::bind(LogPushClickLocalTask(logstring)));
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
                    LogPusher::getLogger(CLICK_SERVICE_LOGGER)->setWorkMode(false);
                    break;
#elif defined USE_KAFKA_LOG
                    sleep(30);
                    if(!LogPusher::getLogger(CLICK_SERVICE_LOGGER)->getWorkMode()){
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

        void LogPusher::startRemoteMonitor(log::Message& msg) { //fixme:fix multi log problem by defining param as a class member
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