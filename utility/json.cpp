//
// Created by guoze.lin on 16/2/2.
//

#include <sstream>
#include <fstream>
#include <exception>
#include "json.h"

namespace adservice{
    namespace utility{
        namespace json{

            void tripslash2(char* str){
                char* p1 = str,*p2=p1;
                while(*p2!='\0'){
                    if(*p2=='\\'&&p2[1]=='\"'){
                        p2++;
                    }
                    *p1++ = *p2++;
                }
                *p1='\0';
            }

            std::string toString(double value){
                char t[100];
                char* middle=t+49,*p1=middle,*p2=p1;
                char sign = value>0.0?' ':'-';
                value = fabs(value);
                long one = (long)value;
                double two = value-one;
                while(one>0){
                    p1--;
                    *p1=(one%10)+0x30;
                    one/=10;
                }
                if(p1==middle){
                    p1--;*p1='0';
                }
                *middle='.';
                while(two>1e-10){
                    p2++;
                    two*=10;
                    int tmp = (int)two;
                    *p2=tmp+0x30;
                    two-=tmp;
                }
                if(p2==middle){
                    p2++;*p2='0';
                }
                p2++;
                *p2 = '\0';
                if(sign=='-'){
                    p1--;*p1=sign;
                }
                return std::string(p1,p2);
            }


            template<typename T>
            std::string toString(T value){
                char t[50];
                char* end=t+49,*p=end;
                char sign = value>0?' ':'-';
                value=abs(value);
                while(value>0){
                    p--;
                    *p = (value%10)+0x30;
                    value/=10;
                }
                *end='\0';
                if(p==end){
                    p--;*p='0';
                }
                if(sign=='-'){
                    p--;*p=sign;
                }
                return std::string(p,end);
            }

            void addKVPair(char* &buffer,const char* key,const char* value,bool ignoreQuote=false){
                const char* p = key;
                *buffer++='"';
                while(*p!='\0'){
                    *buffer++=*p++;
                }
                *buffer++='"';*buffer++=':';
                if(!ignoreQuote)
                    *buffer++='"';
                p=value;
                while(*p!='\0'){
                    *buffer++=*p++;
                }
                if(!ignoreQuote)
                    *buffer++='"';
            }

            std::string toJson(MessageWraper& obj){
                char buffer[1024];
                char* p = buffer;
                *p='{';p++;
                MessageWraper::CIter iter;
                MessageWraper::CObjIter objIter;
                bool begin = true;
                for(iter = obj.messages.begin();iter!=obj.messages.end();iter++){
                    if(!begin){
                        *p++=',';
                    }else{
                        begin = false;
                    }
                    JSONObjectType type = obj.typeMap[iter->first];
                    if ((objIter=obj.innerObjects.find(iter->first)) != obj.innerObjects.end()) {
                        addKVPair(p, iter->first.c_str(), toJson(*(objIter->second)).c_str(),true);
                    } else {
                        addKVPair(p, iter->first.c_str(), iter->second.c_str(),type==OBJECT||type==ARRAY);
                    }
                    p++;
                }
                *p++='}';
                *p='\0';
                return std::string(buffer);
            }

            std::string toJson(rapidjson::Document& doc){
                try {
                    rapidjson::StringBuffer buffer;
                    buffer.Clear();
                    rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
                    doc.Accept(writer);
                    return std::string(buffer.GetString());
                }catch(std::exception& e){
                    DebugMessageWithTime("toJson exception:",e.what());
                    return "";
                }
            }

            const MessageWraper& MessageWraper::getObject(const std::string& key) const{
                CObjIter objIter = innerObjects.find(key);
                if(objIter!=innerObjects.end()){
                    return *(objIter->second);
                }
                throw JSONException("key-obj not exist",-1);
            }

            MessageWraper& MessageWraper::getObject(const std::string& key){
                CObjIter objIter = innerObjects.find(key);
                if(objIter!=innerObjects.end()){
                    return *(objIter->second);
                }
                CIter iter = messages.find(key);
                if(iter!=messages.end()){
                    MessageWraper* newObj = new MessageWraper;
                    parseJson(iter->second.c_str(),*newObj);
                    innerObjects[key] = newObj;
                    typeMap[key] = JSONObjectType::OBJECT;
                    return *newObj;
                }else{
                    throw JSONException("key not exist",-1);
                }
            }

            bool parseJson(const char* json,MessageWraper& mw){
                rapidjson::Reader reader;
                JsonParseHandler handler;
                rapidjson::StringStream ss(json);
                JsonParseHandler::MessageMap& messageMap = mw.getMessages();
                if(reader.Parse(ss,handler)){
                    messageMap.swap(handler.getMessageMap());
                    return true;
                }else{
                    rapidjson::ParseErrorCode e = reader.GetParseErrorCode();
                    size_t o = reader.GetErrorOffset();
                    std::cerr << "Error: " << rapidjson::GetParseError_En(e) << std::endl;;
                    std::string slice = (strlen(json)>o+10)?std::string(json).substr(o,10):std::string(json);
                    std::cerr << " at offset " << o << " near '" << slice << "...'" << std::endl;
                    return false;
                }
            }

            bool parseJson(const char* json,rapidjson::Document& doc){
                doc.Parse<0>(json);
                if(doc.HasParseError()){
                    rapidjson::ParseErrorCode e = doc.GetParseError();
                    size_t o = doc.GetErrorOffset();
                    std::cerr << "Error: " << rapidjson::GetParseError_En(e) << std::endl;;
                    std::string slice = (strlen(json)>o+10)?std::string(json).substr(o,10):std::string(json);
                    std::cerr << " at offset " << o << " near '" << slice << "...'" << std::endl;
                    return false;
                }
                return true;
            }

            bool parseJsonFile(const char* filePath, MessageWraper& mw){
                std::fstream fs(filePath,std::ios_base::in);
                if(!fs.good()){
                    std::cerr<<" can't open json file:"<<filePath<<std::endl;
                    return false;
                }
                std::stringstream ss;
                do{
                    std::string str;
                    std::getline(fs,str,'\n');
                    ss << str;
                }while(!fs.eof());
                fs.close();
                return parseJson(ss.str().c_str(),mw);
            }

            bool parseJsonFile(const char* filePath, rapidjson::Document & doc){
                std::fstream fs(filePath,std::ios_base::in);
                if(!fs.good()){
                    std::cerr<<" can't open json file:"<<filePath<<std::endl;
                    return false;
                }
                std::stringstream ss;
                do{
                    std::string str;
                    std::getline(fs,str,'\n');
                    ss << str;
                }while(!fs.eof());
                fs.close();
                return parseJson(ss.str().c_str(),doc);
            }

        }
    }
}
