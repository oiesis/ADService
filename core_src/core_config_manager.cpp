//
// Created by guoze.lin on 16/3/2.
//

#include "core_config_manager.h"
#include "common/atomic.h"
#include "constants.h"
#include "muduo/base/Logging.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <exception>
#include <functional>

#define MAKE_CONFIG_PARSER(CLASS) (std::bind(&CLASS::parse,std::placeholders::_1,std::placeholders::_2))
#define MAKE_CONFIG_DELETER(CLASS) (std::bind(&CLASS::destruct,std::placeholders::_1))
#define MAKE_CONFIG(CONFIG_FILE,CLASS) ConfigValue(CONFIG_FILE,0,NULL,MAKE_CONFIG_PARSER(CLASS),NULL,MAKE_CONFIG_DELETER(CLASS))


namespace adservice{
    namespace server{

        using namespace muduo;
        using namespace adservice::utility::json;
        using namespace std::placeholders;

        typedef typename ConfigManager::ConfigMap::iterator Iter;

        void* monitorConfigFiles(void* param){
            ConfigManager& configManager = ConfigManager::getInstance();
            while(configManager.isRuning()){
                sleep(1);
                try {
                    configManager.load();
                }catch(std::exception& e){
                    LOG_ERROR<<e.what();
                }
            }
        }

        void ConfigManager::load(){
            struct stat filestat;
            for(Iter iter = configMap.begin();iter!=configMap.end();iter++){
                ConfigManager::ConfigValue& configValue = iter->second;
                const std::string filePath = configValue.filePath;
                long version = configValue.version;
                void* data = configValue.data;
                if(stat(filePath.c_str(),&filestat)==-1){
                    LOG_ERROR<<"ConfigManager:can not stat file,"<<filePath<<",errno:"<<errno;
                    continue;
                }
                time_t lastModified = filestat.st_mtime;
                if(lastModified>version){ // 修改时间比当前版本更晚,更新
//                    MessageWraper mw;
                    rapidjson::Document mw;
                    bool bSuccess = parseJsonFile(filePath.c_str(),mw);
                    if(!bSuccess){
                        LOG_ERROR<<"ConfigManager:parse config file error,"<<filePath;
                        continue;
                    }
                    ConfigObjectParser& parser = configValue.parser;
                    if(!parser){
                        LOG_ERROR<<"ConfigManager:parser of "<<iter->first<<" not assigned";
                        continue;
                    }
                    data = parser(mw,NULL);
                    configValue.version = lastModified;
                    ConfigChangeCallback& onChange = configValue.onChange;
                    if(onChange){
                        try {
                            if(configValue.data!=NULL)
                                onChange(data,configValue.data);
                        }catch(std::exception& e){
                            LOG_ERROR<<"exception occur on config change,"<<filePath<<","<<e.what();
                        }
                    }
                    if(configValue.data!=NULL){
                        configValue.dataDestructor(configValue.data);
                    }
                    configValue.data = data;
                }
            }
        }

        void ConfigManager::start() {
            if(!ATOM_CAS(&started,0,1))
                return;
            // 注册配置解析器
            configMap[CONFIG_SERVICE]   =   MAKE_CONFIG(CONFIG_SERVICE_PATH,ServerConfig);
            configMap[CONFIG_CLICK]     =   MAKE_CONFIG(CONFIG_CLICK_PATH,ClickConfig);
            configMap[CONFIG_LOG]       =   MAKE_CONFIG(CONFIG_LOG_PATH,LogConfig);
            configMap[CONFIG_TRACK_LOG] =   MAKE_CONFIG(CONFIG_TRACK_LOG_PATH,LogConfig);
            configMap[CONFIG_ADSELECT]  =   MAKE_CONFIG(CONFIG_ADSELECT_PATH,ADSelectConfig);
            configMap[CONFIG_DEBUG]     =   MAKE_CONFIG(CONFIG_DEBUG_PATH,DebugConfig);
            configMap[CONFIG_AEROSPIKE] =   MAKE_CONFIG(CONFIG_AEROSPIKE_PATH, AerospikeConfig);
            // 加载注册配置
            load();
            // 开启配置检测线程
            run = true;
            if(pthread_create(&monitorThread,NULL,&monitorConfigFiles,NULL)) {
                LOG_ERROR << "create remote config monitor error";
                return;
            }
        }

        void* ConfigManager::get(const std::string &configKey) {
            Iter iter;
            if((iter = configMap.find(configKey))==configMap.end()){
                throw ConfigException("ConfigManager no such config key");
            }
            ConfigValue& configValue = iter->second;
            if(configValue.data==NULL){
                //tofix:concurrent access
                const std::string& filePath = configValue.filePath;
//                MessageWraper mw;
                rapidjson::Document mw;
                bool bSuccess = parseJsonFile(filePath.c_str(),mw);
                if(!bSuccess){
                    throw ConfigException(std::string("ConfigManager:parse config file ")+filePath);
                }
                ConfigObjectParser& parser = configValue.parser;
                if(!parser){
                    throw ConfigException(std::string("ConfigManager:parser not assigned for key ")+iter->first);
                }
                configValue.data = parser(mw,configValue.data);
            }
            return configValue.data;
        }

        void ConfigManager::stop() {
            if(!ATOM_CAS(&started,1,0))
                return;
            if(run){
                run = false;
                pthread_join(monitorThread,NULL);
                configMap.clear();
            }
        }

        void ConfigManager::registerOnChange(const std::string& configKey,const ConfigChangeCallback& cb){
            Iter iter;
            if((iter = configMap.find(configKey))==configMap.end()){
                throw ConfigException("ConfigManager no such config key");
            }
            ConfigValue& configValue = iter->second;
            configValue.onChange = cb;
        }


        void ConfigManager::registerOnChange(const std::string& configKey,const ConfigChangeCallback&& cb){
            Iter iter;
            if((iter = configMap.find(configKey))==configMap.end()){
                throw ConfigException("ConfigManager no such config key");
            }
            ConfigValue& configValue = iter->second;
            configValue.onChange = cb;
        }

    }
}