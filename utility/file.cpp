//
// Created by guoze.lin on 16/3/30.
//

#include "file.h"
#include "functions.h"

namespace adservice{
    namespace utility{
        namespace file{

            void loadFile(char* buffer,const char* filePath){
                {
                    std::fstream fs(filePath,std::ios_base::in);
                    if(!fs.good()){
                        std::cerr<<" can't open json file:"<<filePath<<std::endl;
                        return;
                    }
                    std::stringstream ss;
                    do{
                        std::string str;
                        std::getline(fs,str,'\n');
                        ss << str;
                    }while(!fs.eof());
                    fs.close();
                    std::string str = ss.str();
//                    DebugMessageWithTime("file size:",str.length());
                    memcpy(buffer,str.c_str(),str.length());
                    buffer[str.length()]='\0';
                }
            }
        }
    }
}