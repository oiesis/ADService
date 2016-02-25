//
// Created by guoze.lin on 16/2/24.
//

#ifndef ADCORE_JSON_H
#define ADCORE_JSON_H

#include "common/types.h"
#include "functions.h"
#include "rapidjson/reader.h"
#include "rapidjson/document.h"
#include "rapidjson/error/en.h"

namespace adservice{
    namespace utility{

        namespace json{

            class MessageWraper{
            public:
                MessageWraper(){
                }
                MessageWraper(std::vector<std::string> keys){
                    for(auto& s : keys){
                        messages[s] = "";
                    }
                }
                bool isFieldEmpty(const std::string& key){
                    std::string& values = messages[key];
                    return values.empty();
                }

                std::map<std::string,std::string>& getMessages(){
                    return messages;
                }

                int32_t getInt(const std::string& key,int32_t defaultValue = 0){
                    std::string& values = messages[key];
                    if(values.empty()){
                        return defaultValue;
                    }
                    int32_t ret = std::stoi(values);
                    return ret;
                }

                double getDouble(const std::string& key,double defaultValue = 0.0){
                    std::string& values = messages[key];
                    if(values.empty()){
                        return defaultValue;
                    }
                    double ret = std::stof(values);
                    return ret;
                }

                bool getBoolean(const std::string& key,bool defaultValue = false){
                    std::string& values = messages[key];
                    if(values.empty()){
                        return defaultValue;
                    }
                    return strcasecmp("true",values.c_str()) == 0;
                }

                const char* getRawString(const std::string& key){
                    std::string& values = messages[key];
                    return values.c_str();
                }

                const std::string& getString(const std::string& key,const std::string& defaultValue){
                    std::string& values = messages[key];
                    if(values.empty()){
                        return defaultValue;
                    }
                    return values;
                }
            private:
                std::map<std::string,std::string> messages;
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



            bool parseJson(const char* json,MessageWraper& mw);

            bool parseJson(const char* json,rapidjson::Document& doc);

            bool parseJsonFile(const char* filePath, MessageWraper& mw);

            bool parseJsonFile(const char* filePath, rapidjson::Document & doc);

        }

    }
}

#endif //ADCORE_JSON_H
