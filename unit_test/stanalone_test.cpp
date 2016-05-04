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
    ParamMap paramMap;
    getParam(paramMap,url);
    typedef typename ParamMap::iterator Iter;
    for(Iter iter = paramMap.begin();iter!=paramMap.end();iter++){
        cout<<iter->first<<":"<<iter->second<<endl;
    }
    DebugMessage("l:",paramMap["l"]);
    char buffer[1024];
    std::string output;
    urlDecode_f(paramMap["l"],output,buffer);
    DebugMessage("after decoded,l:",output);
}

void jsontest(){
    const char* input="{\n"
            "   \"took\": 3,\n"
            "   \"hits\": {\n"
            "      \"total\": 1,\n"
            "      \"hits\": [\n"
            "         {\n"
            "            \"_index\": \"solutions\",\n"
            "            \"_type\": \"banner\",\n"
            "            \"_id\": \"5002588\",\n"
            "            \"_score\": 7.1737857,\n"
            "            \"_routing\": \"185\",\n"
            "            \"_parent\": \"185\",\n"
            "            \"_source\": {\n"
            "               \"banner_type\": 1,\n"
            "               \"bgid\": 185,\n"
            "               \"bid\": 2588,\n"
            "               \"ctr\": 0,\n"
            "               \"height\": 90,\n"
            "               \"html\": \"{pid:\\\\'%s\\\\',width:\\\\'960\\\\',height:\\\\'90\\\\',impid:\\\\'%s\\\\',advid:\\\\'40\\\\',unid:\\\\'%s\\\\',plid:\\\\'%s\\\\',gpid:\\\\'%s\\\\',cid:\\\\'2588\\\\',arid:\\\\'%s\\\\',ctype:\\\\'1\\\\',xcurl:\\\\'%s\\\\',of:\\\\'0\\\\',tview:\\\\'\\\\',mtls: [{p0: \\\\'http://material.mtty.com/201603/17/Fi8K7Z.jpg\\\\',p1: \\\\'http://jump.ztcadx.com/diy?target=http%3A%2F%2Fhuishanmy.tmall.com%2F%3Fkid%3D34095_207194_970488_1436040%26shop_id%3D116953304\\\\',p2: \\\\'000\\\\',p3: \\\\'960\\\\',p4: \\\\'90\\\\',p5:\\\\'\\\\',p6:\\\\'\\\\',p7:\\\\'\\\\',p8:\\\\'\\\\',p9:\\\\'\\\\'}]}\",\n"
            "               \"offerprice\": 0,\n"
            "               \"width\": 960\n"
            "            }\n"
            "         }\n"
            "      ]\n"
            "   }\n"
            "}";
    rapidjson::Document doc;
    parseJson(input,doc);
    DebugMessageWithTime("doc.a:",doc["a"].GetString());
}

void hash_test(){
    const char* key = "creative_id_164";
    int h = fnv_hash(key,strlen(key)) % 1024;
    DebugMessageWithTime("hash:",h);
}

int main(int argc,char** argv){
    try {
        char buffer[8196];
        long t = sizeof(buffer);
        DebugMessageWithTime(t);
    }catch(std::exception& e){
        DebugMessageWithTime("exception:",e.what());
    }
    return 0;
}