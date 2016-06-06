//
// Created by guoze.lin on 16/2/24.
//

#ifndef ADCORE_JSON_H
#define ADCORE_JSON_H

#include "common/types.h"
#include "functions.h"
#include "rapidjson/reader.h"
#include "rapidjson/writer.h"
#include "rapidjson/document.h"
#include "rapidjson/error/en.h"

namespace adservice{
    namespace utility{

        namespace json{

#define MakeStringValue(s) rapidjson::Value().SetString(s.c_str(),s.length())

#define MakeStringConstValue(s) rapidjson::Value().SetString(s)

#define MakeIntValue(s) rapidjson::Value().SetInt(s)

#define MakeDoubleValue(d) rapidjson::Value().SetDouble(d)

#define MakeBooleanValue(b) rapidjson::Value().SetBool(b)

            void tripslash2(char* str);

            class JSONException : public std::exception{
            public:
                JSONException() _GLIBCXX_USE_NOEXCEPT {}
                JSONException(const std::string& str,int error) _GLIBCXX_USE_NOEXCEPT :message(str),errorCode(error){}
                const char* GetMsg() const { return message.c_str();}
                const char* what() const _GLIBCXX_USE_NOEXCEPT {return message.c_str();}
                int GetError() const {return errorCode;}
            private:
                int errorCode;
                std::string message;
            };

            std::string toString(double value);

            inline std::string toString(const std::string& str){
                return std::string(str);
            }

            enum JSONObjectType : char{
                NUMBER,
                STRING,
                OBJECT,
                ARRAY
            };

            /**
             * 默认会将所有类型当成是字符串表示,所有类型提取都采用LazyLoad方式,
             * 如果需要频繁的读取,应该写自己对象的parser,直接用MessageWraper转换为对象实例
             */
            class MessageWraper{
            private:
                typedef typename std::map<std::string,std::string>::const_iterator CIter;
                typedef typename std::map<std::string,MessageWraper*>::const_iterator CObjIter;
                typedef typename std::map<std::string,JSONObjectType>::const_iterator CTypeIter;
            public:
                MessageWraper(){
                }
                MessageWraper(std::vector<std::string> keys){
                    for(auto& s : keys){
                        messages[s] = "";
                    }
                }
                ~MessageWraper(){
                    if(!messages.empty()){
                        messages.clear();
                    }
                    if(!innerObjects.empty()){
                        for(CObjIter iter = innerObjects.cbegin();iter!=innerObjects.cend();iter++){
                           delete iter->second;
                        }
                        innerObjects.clear();
                    }
                    if(!typeMap.empty()){
                        typeMap.clear();
                    }
                }

                void addField(const std::string& key,const std::string& value,JSONObjectType type = JSONObjectType::STRING){
                    messages[key] = value;
                    typeMap[key] = type;
                }

                void bindFieldType(const std::string& key,JSONObjectType type){
                    typeMap[key] = type;
                }

                void bindFieldTypes(const std::map<std::string,JSONObjectType>& tMap){
                    typeMap.insert(tMap.begin(),tMap.end());
                }

                bool isFieldEmpty(const std::string& key) const{
                    CIter iter;
                    if((iter = messages.find(key))!=messages.end()){
                        return iter->second.empty();
                    }
                    return true;
                }

                bool isFieldEmpty(const std::string& key){
                    std::string& values = messages[key];
                    return values.empty();
                }

                std::map<std::string,std::string>& getMessages(){
                    return messages;
                }

                const std::map<std::string,std::string>& getMessages() const{
                    return messages;
                };

                int32_t getInt(const std::string& key,int32_t defaultValue = 0){
                    std::string& values = messages[key];
                    if(values.empty()){
                        return defaultValue;
                    }
                    int32_t ret = std::stoi(values);
                    return ret;
                }

                int32_t getInt(const std::string& key,int32_t defaultValue = 0) const{
                    CIter iter;
                    if((iter = messages.find(key))!=messages.end() && !iter->second.empty()){
                        return std::stoi(iter->second);
                    }
                    return defaultValue;
                }

                double getDouble(const std::string& key,double defaultValue = 0.0){
                    std::string& values = messages[key];
                    if(values.empty()){
                        return defaultValue;
                    }
                    double ret = std::stof(values);
                    return ret;
                }

                double getDouble(const std::string& key,double defaultValue = 0.0) const{
                    CIter iter;
                    if((iter = messages.find(key))!=messages.end() && !iter->second.empty()){
                        return std::stof(iter->second);
                    }
                    return defaultValue;
                }

                bool getBoolean(const std::string& key,bool defaultValue = false){
                    std::string& values = messages[key];
                    if(values.empty()){
                        return defaultValue;
                    }
                    return strcasecmp("true",values.c_str()) == 0;
                }

                bool getBoolean(const std::string& key,bool defaultValue = false) const{
                    CIter iter;
                    if((iter = messages.find(key))!=messages.end() && !iter->second.empty()){
                        return strcasecmp("true",iter->second.c_str()) == 0;
                    }
                    return defaultValue;
                }

                const char* getRawString(const std::string& key){
                    std::string& values = messages[key];
                    return values.c_str();
                }

                const char* getRawString(const std::string& key) const{
                    CIter iter;
                    if((iter = messages.find(key))!=messages.end() && !iter->second.empty()){
                        return iter->second.c_str();
                    }
                    return NULL;
                }

                const std::string& getString(const std::string& key,const std::string& defaultValue = ""){
                    std::string& values = messages[key];
                    if(values.empty()){
                        return defaultValue;
                    }
                    return values;
                }

                const std::string& getString(const std::string& key,const std::string& defaultValue = "") const{
                    CIter iter;
                    if((iter = messages.find(key))!=messages.end() && !iter->second.empty()){
                        return iter->second;
                    }
                    return defaultValue;
                }

                bool isEmpty() const{
                    return messages.empty();
                }

                //lazy load,non-threadsafe
                const MessageWraper& getObject(const std::string& key) const;

                MessageWraper& getObject(const std::string& key);

                friend std::string toJson(MessageWraper&);

            private:
                std::map<std::string,std::string> messages;
                std::map<std::string,MessageWraper*> innerObjects;
                std::map<std::string,JSONObjectType> typeMap;
            };


            class JsonParseHandler
                    : public rapidjson::BaseReaderHandler<rapidjson::UTF8<>, JsonParseHandler> {
            public:
                typedef std::map<std::string,std::string> MessageMap;

                JsonParseHandler() : messages_(), state_(kExpectObjectStart), name_() {}

                bool StartObject() {
                    switch (state_) {
                        case kExpectObjectStart:
                            state_ = kExpectNameOrObjectEnd;
                            return true;
                        default:
                            return false;
                    }
                }

                bool String(const char* str, rapidjson::SizeType length, bool) {
                    switch (state_) {
                        case kExpectNameOrObjectEnd:
                            name_ = std::string(str, length);
                            state_ = kExpectValue;
                            return true;
                        case kExpectValue:
                            messages_.insert(MessageMap::value_type(name_, std::string(str, length)));
                            state_ = kExpectNameOrObjectEnd;
                            return true;
                        default:
                            return false;
                    }
                }

                bool EndObject(rapidjson::SizeType) { return state_ == kExpectNameOrObjectEnd; }

                bool Default() { return false; } // All other events are invalid.

                MessageMap& getMessageMap(){
                    return messages_;
                }
            private:
                MessageMap messages_;
                enum State {
                    kExpectObjectStart,
                    kExpectNameOrObjectEnd,
                    kExpectValue
                }state_;
                std::string name_;
            };

            std::string toJson(MessageWraper& obj);

            std::string toJson(rapidjson::Document& doc);

            bool parseJson(const char* json,MessageWraper& mw);

            bool parseJson(const char* json,rapidjson::Document& doc);

            bool parseJsonFile(const char* filePath, MessageWraper& mw);

            bool parseJsonFile(const char* filePath, rapidjson::Document & doc);



            inline int getClassValue(rapidjson::Value& value,int def){
                return value.GetInt();
            }


            inline const std::string getClassValue(rapidjson::Value& value,const std::string& def){
                return std::string(value.GetString());
            }


            inline double getClassValue(rapidjson::Value& value,double def){
                return value.GetDouble();
            }

            inline bool getClassValue(rapidjson::Value& value,bool def){
                return value.GetBool();
            }

            inline std::string getField(rapidjson::Value& document,const std::string& key,const char* def){
                std::string defStr(def);
                return document.HasMember(key.c_str())?getClassValue(document[key.c_str()],defStr):defStr;
            }

            template<typename T>
            inline T getField(rapidjson::Value& document,const std::string& key,const T& def){
                return document.HasMember(key.c_str())?getClassValue(document[key.c_str()],def):def;
            }

        }

    }
}

#endif //ADCORE_JSON_H
