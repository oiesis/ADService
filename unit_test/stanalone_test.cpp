//
// Created by guoze.lin on 16/3/18.
//
#define UNIT_TEST
#include <string>
#include <iostream>
#include <iomanip>
#include <map>
#include <vector>
#include "common/functions.h"
#include "common/constants.h"
#include "utility/url.h"

using namespace adservice::utility::url;
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

int main(int argc,char** argv){
    std::string output="{\n"
            "   \"hits\": {\n"
            "      \"total\": 1,\n"
            "      \"hits\": [\n"
            "         {\n"
            "            \"_source\": {\n"
            "               \"banner_type\": 2,\n"
            "               \"bgid\": 28,\n"
            "               \"bid\": 59,\n"
            "               \"ctr\": 0,\n"
            "               \"height\": 50,\n"
            "               \"html\": \"{mt_ad_pid:\\\\'%s\\\\',mt_ad_width:\\\\'300\\\\',mt_ad_height:\\\\'50\\\\',mt_ad_impid:\\\\'%s\\\\',mt_ad_advid:\\\\'23\\\\',mt_ad_unid:\\\\'%s\\\\',mt_ad_plid:\\\\'%s\\\\',mt_ad_gpid:\\\\'%s\\\\',mt_ad_cid:\\\\'59\\\\',mt_ad_arid:\\\\'%s\\\\',mt_ad_ctype:\\\\'2\\\\',mt_ad_xcurl:\\\\'%s\\\\',mt_ad_of:\\\\'0\\\\',mt_ad_tview:\\\\'\\\\',mt_ad_mtls: [{p0: \\\\'http://material.mtty.com/201601/08/0Oa4KL.swf\\\\',p1: \\\\'http://bd.5399.com/htmlcode/534726.html\\\\',p2: \\\\'000\\\\',p3: \\\\'300\\\\',p4: \\\\'50\\\\',p5:\\\\'\\\\',p6:\\\\'\\\\',p7:\\\\'\\\\',p8:\\\\'\\\\',p9:\\\\'\\\\'}]}\",\n"
            "               \"offerprice\": 0,\n"
            "               \"width\": 300\n"
            "            }\n"
            "         }\n"
            "      ]\n"
            "   }\n"
            "}";
    paramTest();
    return 0;
}