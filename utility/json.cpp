//
// Created by guoze.lin on 16/2/2.
//

#include "utility.h"
#include <sstream>
#include <fstream>

namespace adservice{
    namespace utility{
        namespace json{

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
                    std::cerr << " at offset " << o << " near '" << std::string(json).substr(o, 10) << "...'" << std::endl;
                    return false;
                }
            }

            bool parseJson(const char* json,rapidjson::Document& doc){
                doc.Parse<0>(json);
                if(doc.HasParseError()){
                    rapidjson::ParseErrorCode e = doc.GetParseError();
                    size_t o = doc.GetErrorOffset();
                    std::cerr << "Error: " << rapidjson::GetParseError_En(e) << std::endl;;
                    std::cerr << " at offset " << o << " near '" << std::string(json).substr(o, 10) << "...'" << std::endl;
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
		        DebugMessage("read json file ok");
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
