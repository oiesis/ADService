//
// Created by guoze.lin on 16/3/2.
//

#ifndef ADCORE_CORE_CONFIG_MANAGER_H
#define ADCORE_CORE_CONFIG_MANAGER_H

#include "functions.h"
#include <map>
#include <string>
#include <tuple>
#include <functional>
#include <unistd.h>
#include "config_types.h"

namespace adservice{
    namespace server{

        using namespace adservice::utility::json;

        typedef std::function<void* (const rapidjson::Document&,void*)> ConfigObjectParser;
        typedef std::function<void (void*,void*)> ConfigChangeCallback;
        typedef std::function<void (void*)> ConfigObjectDestructor;

        class ConfigManager{
        public:
            typedef struct ConfigValue{
                std::string filePath;
                long version;
                void* data;
                ConfigObjectParser parser;
                ConfigChangeCallback onChange;
                ConfigObjectDestructor dataDestructor;
                ConfigValue(){}
                ConfigValue(const std::string& f,long v,void* d,const ConfigObjectParser& p,const ConfigChangeCallback& c,const ConfigObjectDestructor& deleter):
                        filePath(f),version(v),data(d),parser(p),onChange(c),dataDestructor(deleter){}
                ConfigValue(const std::string&& f,long v,void* d,const ConfigObjectParser&& p,const ConfigChangeCallback&& c,const ConfigObjectDestructor&& deleter):
                        filePath(f),version(v),data(d),parser(p),onChange(c),dataDestructor(deleter){}
                ~ConfigValue(){
                    if(data!=NULL){
                        DebugMessage("ConfigValue destruct");
                        dataDestructor(data);
                    }
                }
            } ConfigValue;

            typedef std::map<std::string,ConfigValue> ConfigMap;
        public:
            static ConfigManager& getInstance(){
                static ConfigManager instance;
                return instance;
            }
            static void init(){
                ConfigManager& instance = getInstance();
                if(!instance.started)
                    instance.start();
            }
            static void exit(){
                getInstance().stop();
            }
        public:
            /**
             * 获取相关配置,返回对应的配置对象指针
             */
            void* get(const std::string& configKey);

            void start();

            void stop();

            bool isRuning(){return run;}

            void load();

            ConfigMap& getConfigMap(){ return configMap;}

            /**
             * 注册配置变更时的回调
             */
            void registerOnChange(const std::string& key,const ConfigChangeCallback& cb);

            /**
             * 注册配置变更时的回调
             */
            void registerOnChange(const std::string& key,const ConfigChangeCallback&& cb);

            ~ConfigManager(){
                DebugMessage("ConfigManager gone");
                stop();
            }
        private:
            ConfigManager():started(0),run(false){}
            int started;
            bool run;
            pthread_t monitorThread;
        private:
            ConfigMap configMap;
        };

        class ConfigException : public std::exception{
        public:
        public:
            ConfigException() _GLIBCXX_USE_NOEXCEPT {}
            ConfigException(const std::string& str) _GLIBCXX_USE_NOEXCEPT :message(str){}
            const char* what() const _GLIBCXX_USE_NOEXCEPT {return message.c_str();}
        private:
            std::string message;
        };
    }
}

#endif //ADCORE_CORE_CONFIG_MANAGER_H
