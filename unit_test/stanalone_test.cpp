//
// Created by guoze.lin on 16/3/18.
//
#define UNIT_TEST
#include <string>
#include <iostream>
#include <iomanip>
#include <map>
#include <vector>
#include <exception>
#include "common/functions.h"
#include "common/constants.h"
#include "utility/url.h"
#include "utility/json.h"
#include "utility/hash.h"
#include "utility/mttytime.h"
#include "utility/cypher.h"

using namespace adservice::utility::url;
using namespace adservice::utility::json;
using namespace adservice::utility::hash;
using namespace adservice::utility::time;
using namespace adservice::utility::cypher;
using namespace std;

void paramTest(){
    const char* url = "v=1.0&d=9&k=1&y=7&t1=ererwrwer3&t2=";
    //const char* url = "http://show.mtty.com/v?p=-tsCgWZqNjC6xpRl8VwJxQ==&of=3&a=0086-ffff-ffff&b=20000&c=12551&d=8&e=10000&r=g2iwjo7r6xpag&s=8863364436303842593&x=13&tm=1463538785&w=&gz=1";
    ParamMap paramMap;
    getParamv2(paramMap,url);
    typedef typename ParamMap::iterator Iter;
    for(Iter iter = paramMap.begin();iter!=paramMap.end();iter++){
        cout<<iter->first<<":"<<iter->second<<endl;
    }
    //DebugMessage("l:",paramMap["l"]);
    //char buffer[1024];
    //std::string output;
    //urlDecode_f(paramMap["l"],output,buffer);
    //DebugMessage("after decoded,l:",output);
}


#define MakeStringValue(s) rapidjson::Value().SetString(s.c_str(),s.length())

#define MakeStringConstValue(s) rapidjson::Value().SetString(s)
void jsontest(){
    const char* pjson= "{\"advid\":\"8\",\"cid\":\"2\",\"ctype\":\"1\",\"formatid\":\"5\",\"gpid\":\"\",\"height\":\"\",\"impid\":\"\",\"mtls\":[{\"p0\":\"/201604/28/VXTsDM.swf\",\"p1\":\"http://www.mtty.com\",\"p10\":\"\",\"p11\":\"\",\"p3\":\"160\",\"p4\":\"600\",\"p5\":\"\",\"p6\":\"\",\"p7\":\"\",\"p8\":\"\",\"p9\":\"\"}],\"of\":\"\",\"pid\":\"\",\"plid\":\"\",\"tview\":\"\",\"unid\":\"\",\"width\":\"\",\"xcurl\":\"\"}";
    rapidjson::Document mtAdInfo;
    parseJson(pjson,mtAdInfo);
    ParamMap paramMap;
    paramMap[URL_ADPLACE_ID] = "123";
    paramMap[URL_EXPOSE_ID] = "afasfaf";
    paramMap[URL_ADX_ID] = "21";
    paramMap[URL_EXEC_ID] = "12";
    paramMap[URL_AREA_ID] = "0086-010-11";
    mtAdInfo["pid"] = MakeStringValue(paramMap[URL_ADPLACE_ID]);
    mtAdInfo["impid"] = MakeStringValue(paramMap[URL_EXPOSE_ID]);
    mtAdInfo["unid"] = MakeStringValue(paramMap[URL_ADX_ID]);
    mtAdInfo["plid"] = MakeStringConstValue("");
    mtAdInfo["gpid"] = MakeStringValue(paramMap[URL_EXEC_ID]);
    mtAdInfo.AddMember("arid",MakeStringValue(paramMap[URL_AREA_ID]),mtAdInfo.GetAllocator());
    mtAdInfo["xcurl"] = MakeStringConstValue("");
    std::string jsonResult = toJson(mtAdInfo);
    DebugMessageWithTime(jsonResult);
}

void hash_test(){
    const char* key = "creative_id_164";
    int h = fnv_hash(key,strlen(key)) % 1024;
    DebugMessageWithTime("hash:",h);
}



#include "core_src/core_ip_manager.h"

void ip_area_test(const char* ip){
    using namespace adservice::server;
    IpManager::init("../17monipdb.dat","../city_area_code.txt");
    IpManager& ipManager = IpManager::getInstance();
    long timeBegin = getCurrentTimeStamp();
    int code;
    //for(int i=0;i<1000000;i++) {
        code = ipManager.getAreaByIp(ip);
    //}
    long timeEnd = getCurrentTimeStamp();
    DebugMessageWithTime("begin time:",timeBegin,",end time:",timeEnd,",cost:",timeEnd-timeBegin,",area code:",code);
    IpManager::destroy();
}

void url_encode_test(const char* url){
    char buffer[2048];
    std::string encoded;
    urlEncode_f(url,strlen(url),encoded,buffer);
    DebugMessageWithTime("result:",buffer);
}

#include "core_src/core_config_manager.h"

void configmanager_test(){
    using namespace adservice::server;
    ConfigManager::init();
    ConfigManager& configManager = ConfigManager::getInstance();
    LogConfig* logConfig = (LogConfig*)configManager.get(CONFIG_LOG);
    DebugMessageWithTime("logConfig kafka broker:",logConfig->kafkaBroker);
    DebugMessageWithTime("logConfig kafka topic:",logConfig->kafkaTopic);
    ServerConfig* serviceConfig = (ServerConfig*)configManager.get(CONFIG_SERVICE);
    DebugMessageWithTime("serviceConfig loadClick:",serviceConfig->runClick);
    DebugMessageWithTime("serviceConfig loadBid:",serviceConfig->coreHttpThreads);
    DebugMessageWithTime("serviceConfig loadBid:",serviceConfig->corePort);
    ConfigManager::exit();
}


bool checkUserCookies(const std::string& oldCookies){
    CypherResult128 cypherResult;
    memcpy((void*)cypherResult.bytes,(void*)oldCookies.c_str(),oldCookies.length());
    DecodeResult64 decodeResult64;
    if(!cookiesDecode(cypherResult,decodeResult64)
       || decodeResult64.words[0]<=0
       || decodeResult64.words[0]>getCurrentTimeSinceMtty()){
        return false;
    }
    return true;
}

#include "protocol/guangyin/guangyin_price.h"

void price_test(const char* encrypt){
    int result = guangyin_price_decode(encrypt);
    DebugMessageWithTime("result:",result);
}


int extractRealValue(const std::string& input,int targetAdx){
    const char* pdata = input.data();
    const char* p1 = pdata,*p2 = p1;
    while(*p2!='\0'){
        if(*p2=='|'){
            int adx = atoi(p1);
            p2++;
            p1 = p2;
            while(*p2!='\0'&&*p2!='|')p2++;
            if(adx == targetAdx){
                return atoi(p1);
            }
            if(*p2=='|'){
                p2++;
                p1=p2;
            }
        } else
            p2++;
    }
    return 0;
}

#include "utility/userclient.h"

void testUaParser(char* input){
    std::string result = adservice::utility::userclient::getBrowserTypeFromUA(input);
    DebugMessageWithTime("result:",result);
}

int main(int argc,char** argv){
    try {
        if(argc>=2){
            uint64_t h = std::abs(fnv_hash(argv[1],strlen(argv[1]))%1024);
            DebugMessage("hash:",h);
        }
    }catch(std::exception& e){
        DebugMessageWithTime("exception:",e.what());
    }
    return 0;
}
