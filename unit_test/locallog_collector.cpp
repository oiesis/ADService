//
// Created by guoze.lin on 16/4/11.
//
#include <string>
#include <map>
#include <cstdlib>
#include <sys/stat.h>
#include <functional>
#include <fstream>
#include <dirent.h>
#include <boost/archive/tmpdir.hpp>
#include <boost/serialization/map.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>
#include "functions.h"
#include "utility/utility.h"
#include "core_config_manager.h"
#include "core_src/net/kafka_log_producer.h"

#define PROGRESS_FILE       "progress.bin"
#define COUNTER_FILE        "counter.bin"

using namespace std;
using namespace adservice::utility::serialize;


std::map<std::string,int> progressMap;
typedef std::map<std::string,int>::iterator ProgressMapIter;

std::map<std::string,int> counterMap;
typedef std::map<std::string,int>::iterator CounterMapIter;

typedef std::function<bool (const char*,int)> SyncLogCallback;

bool init(){
    ifstream ifs1(PROGRESS_FILE,ios_base::in);
    if(!ifs1.good()) {
        DebugMessage("init,can't open progress file");
        return false;
    }
    boost::archive::text_iarchive progress(ifs1);
    progress >> progressMap ;
    ifstream ifs2(COUNTER_FILE,ios_base::in);
    if(!ifs2.good()){
        DebugMessage("init,can't open counter file");
        return false;
    }
    boost::archive::text_iarchive counter(ifs2);
    counter >> counterMap;
    ifs1.close();
    ifs2.close();
    return true;
}

void finalize(){
    ofstream ofs1(PROGRESS_FILE);
    if(!ofs1.good())
        throw "finalize,can't open progress file";
    boost::archive::text_oarchive progress(ofs1);
    progress << progressMap;
    ofstream ofs2(COUNTER_FILE);
    if(!ofs2.good())
        throw "finalize,can't open counter file";
    boost::archive::text_oarchive counter(ofs2);
    counter << counterMap;
    ofs1.close();
    ofs2.close();
}


void readFile(const char* filename,const SyncLogCallback& logCallback = SyncLogCallback()){
    std::string file = filename;
    ProgressMapIter iter = progressMap.find(file);
    int fileOffset = 0;
    if(iter == progressMap.end()){
        progressMap[file] = 0;
        fileOffset = 0;
    }else{
        fileOffset = iter->second;
    }
    CounterMapIter counterIter = counterMap.find(file);
    int counter = 0;
    if(counterIter==counterMap.end()){
        counterMap[file] = 0;
        counter = 0;
    }else{
        counter = counterMap[file];
    }
    FILE* fp = fopen(filename,"rb");
    if(!fp){
        DebugMessage("error open file,",filename);
        return;
    }
    fseek(fp,fileOffset,SEEK_SET);
    while(!feof(fp)){
        char lenBuff[56];
        char c;
        char *p = lenBuff;
        int len = 0;
        while((c=fgetc(fp))!='^'&& c!= EOF){
            *p++ = c;
        }
        if(c==EOF){
            break;
        }
        if(lenBuff[0]=='m'&&lenBuff[1]=='t'){
            *p = '\0';
            len = atoi(lenBuff+2);
        }
        fileOffset+=p-lenBuff;
        char logBuffer[2048];
        len=fread(logBuffer,1,len,fp);
        if(logCallback){
           logCallback(logBuffer,len);
        }
        fileOffset+=len;
        counter++;
    }
    fclose(fp);
    //printf("fileOffset:%d,counter:%d\n",fileOffset,counter);
    progressMap[file] = fileOffset;
    counterMap[file] = counter;
}

using namespace adservice::log;
using namespace adservice::server;
KafkaLogProducer* logProducer = NULL;

bool checkDataValid(const char* data,int size){
    try{
        protocol::log::LogItem logItem;
        getAvroObject(logItem, (const uint8_t*)data, size);
        return true;
    }catch(avro::Exception& e){
        DebugMessage("check parsing avro object failed");
    }
    return false;
}

bool pushLog2Kafka(const char* data,int size){
    try {
        if(!checkDataValid(data,size))
            return false;
        adservice::log::Message m("", std::string(data, data + size));
        SendResult sendResult = logProducer->send(m);
        return true;
    }catch(LogClientException& e){
        DebugMessageWithTime("LogClient error:",e.GetMsg()," errorcode:",e.GetError());
    }catch(std::exception& e){
        DebugMessageWithTime("error occured in LogPushClickTask,err:",e.what());
    }
    return false;
}

extern std::string getLogItemString(protocol::log::LogItem& log);

bool verifydata(const char* data,int size){
    try {
        protocol::log::LogItem logItem;
        getAvroObject(logItem, (const uint8_t*)data, size);
        std::string str = getLogItemString(logItem);
        printf("%s\n",str.c_str());
        return true;
    }catch(avro::Exception& e){
        DebugMessageWithTime("error in parsing avro log object",e.what());
    }
    return false;
}


int main(int argc,const char* *argv){
    if(argc != 3){
        printf("usage:locallog_collector [DIR] [OPERATION]\n");
        return 0;
    }
    const char* operation = argv[2];
    SyncLogCallback syncCB;
    if(strcmp(operation,"check")==0){
        syncCB = std::bind(&verifydata,std::placeholders::_1,std::placeholders::_2);
    }else if(strcmp(operation,"sync")==0){
        ConfigManager::init();
        logProducer = new KafkaLogProducer("localsync");
        syncCB = std::bind(&pushLog2Kafka,std::placeholders::_1,std::placeholders::_2);
    }
    try {
        if(!init()){
            finalize();
            init();
        }
        const char *targetDir = argv[1];
        DIR *dir = opendir(targetDir);
        if (!dir) {
            printf("can't open dir");
            return 0;
        }
        struct dirent *dirent = NULL;
        //DebugMessage("collecting dir ", targetDir);
        while ((dirent = readdir(dir)) != NULL) {
            if (strcmp(".", dirent->d_name) == 0 || strcmp("..", dirent->d_name) == 0)
                continue;
            char buffer[1024];
            sprintf(buffer, "%s/%s", targetDir, dirent->d_name);
            //DebugMessage("collecting file ", buffer);
            readFile(buffer,syncCB);
        }
        for (CounterMapIter iter = counterMap.begin(); iter != counterMap.end(); iter++) {
            printf("%s - %d \n", iter->first.c_str(), iter->second);
        }
        finalize();
    }catch(const char* errstr){
        DebugMessage("exception caught:",errstr);
    }catch(...){
        DebugMessage("error occured");
    }
    if(logProducer!=NULL){
        delete logProducer;
        ConfigManager::exit();
    }
    return 0;
}