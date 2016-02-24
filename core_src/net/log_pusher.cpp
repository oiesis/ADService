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


        struct LogPushClickTask{
            LogPushClickTask(Producer* p,std::shared_ptr<adservice::types::string>& l):producer(p),log(l){}
            LogPushClickTask(Producer* p,std::shared_ptr<adservice::types::string>&& l):producer(p),log(l){}
            void operator()(){
                std::string ali_escapeString = encode4ali(*(log.get()));
                DebugMessage("encoded log string for aliyun,length:",ali_escapeString.length());
                ons::Message msg(DEFAULT_TOPIC,"tagA",ali_escapeString);
                try{
                    SendResultONS sendResult = producer->send(msg);
                    DebugMessage("sendResult:",sendResult.getMessageId());
                }catch(ONSClientException& e){
                    LOG_ERROR << "ONSClient error:" << e.GetMsg() << " errorcode:" << e.GetError();
                    LogPusherPtr logger = LogPusher::getLogger(CLICK_SERVICE_LOGGER);
                    logger->setWorkMode(true);
                    logger->startRemoteMonitor(msg);
                }catch(std::exception& e){
                    LOG_ERROR << "error occured in LogPushClickTask,err:"<<e.what();
                }
            }
            Producer* producer;
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
                    sprintf(dirname,"log/%d%02d%d",1900+ltime->tm_year,ltime->tm_mon+1,ltime->tm_mday);
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

        void LogPusher::loadLoggerFactoryProperty(const char* file){
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

        void LogPusher::push(std::shared_ptr<adservice::types::string>& logstring){
            if(!modeLocal) {
                executor.run(std::bind(LogPushClickTask(producer, logstring)));
            }else{
                executor.run(std::bind(LogPushClickLocalTask(logstring)));
            }
        }

        void LogPusher::push(std::shared_ptr<adservice::types::string>&& logstring){
            if(!modeLocal) {
                executor.run(std::bind(LogPushClickTask(producer, logstring)));
            }else{
                executor.run(std::bind(LogPushClickLocalTask(logstring)));
            }
        }

        struct RemoteMonitorThreadParam{
            Producer* producer;
            Message msg;
            int started;
            RemoteMonitorThreadParam():started(0){}
            RemoteMonitorThreadParam(Producer* p,const Message& m):producer(p),msg(m),started(0){}
            void init(Producer* p,const Message& m){
                producer = p;
                msg = m;
            }
        };

        void* monitorRemoteLog(void* param){
            RemoteMonitorThreadParam* _param = (RemoteMonitorThreadParam*)param;
            Producer* producer = _param->producer;
            Message& msg = _param->msg;
            while(true) {
                try {
                    SendResultONS result = producer->send(msg);
                    LogPusher::getLogger(CLICK_SERVICE_LOGGER)->setWorkMode(false);
                    break;
                } catch (ONSClientException &e) {
                    LOG_ERROR<<"aliyun ons error still exists,error:"<<e.GetError();
                }
                sleep(2);
            }
            _param->started = 0;
            return NULL;
        }

        void LogPusher::startRemoteMonitor(ons::Message& msg) {
            static RemoteMonitorThreadParam param;
            if(param.started)
                return;
            if(!ATOM_CAS(&param.started,0,1))
                return;
            param.init(producer,msg);
            pthread_t monitorThread;
            if(pthread_create(&monitorThread,NULL,&monitorRemoteLog,&param)){
                LOG_ERROR<<"create remote log monitor error";
            }
            pthread_detach(monitorThread);
        }

    }
}