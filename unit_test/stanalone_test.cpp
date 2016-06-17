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

using namespace adservice::utility::url;
using namespace adservice::utility::json;
using namespace adservice::utility::hash;
using namespace adservice::utility::time;
using namespace std;

void paramTest(){
    const char* url = "http://show.mtty.com/v?p=VwsimgAO6MJ7jEpgW5IA8rszDyLuC4qNGtNxzw&a=0086-ffff-ffff&b=50&c=2546&d=9&e=288&r=3e581c094cf01851&s=9223372032561888060&x=6&tm=1460347546&l=http://click.bes.baidu.com/adx.php?c=cz00NGY1OWJmMDA0MGEzMzFmAHQ9MTQ2MDM0NzU0NgBzZT0xAGJ1PTE4NzA0OTA3AHR1PTkyMjMzNzIwMzI1NjE4ODgwNjAAYWQ9MTQ1ODExOTUyMDUyMDI1NDYAc2l0ZT1odHRwOi8vd3d3Ljh2djguY29tL25ld3MvNzdfMTQuaHRtbAB2PTEAaT05ZDFlOWFjMA&k=dz0zMzYAaD0yODAAY3NpZD0xMjAyNTkwODQzMjE2AHRtPTI2OTA0Njk4NQB0ZD0yMDc5NTQ4AHdpPTE4NzA0OTA3AGZuPTMwMDE0MDg4X2NwcgBmYW49AHVpZD0xODczNzA1NABjaD0wAG9zPTkAYnI9MTAAaXA9MTI0LjEyNi4yMDUuNzgAc3NwPTEAYXBwX2lkPQBhcHBfc2lkPQBzZGtfdmVyc2lvbj0AdHRwPTEAY29tcGxlPTAAc3R5cGU9MABjaG1kPTAAc2NobWQ9MAB4aXA9MTAwLjY1LjQxLjgwAGR0cD0xAGNtYXRjaD0yMDAAZmlyc3RfcmVnaW9uPTEAc2Vjb25kX3JlZ2lvbj0zODIAYWRjbGFzcz0w&url=http%253A%252F%252Fbdtg%2E9377a%2Ecom%252Fsousuotg%2Ephp%253Fid%253D11852%2526uid%253D%257Bmpid%257D_%257Bcid%257D";
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
    for(int i=0;i<1000000;i++) {
        code = ipManager.getAreaByIp(ip);
    }
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

int main(int argc,char** argv){
    try {
        configmanager_test();
    }catch(std::exception& e){
        DebugMessageWithTime("exception:",e.what());
    }
    return 0;
}