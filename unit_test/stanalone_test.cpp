//
// Created by guoze.lin on 16/3/18.
//
#define UNIT_TEST
#include <string>
#include <iostream>
#include <map>
#include <vector>
#include "common/functions.h"
#include "utility/cypher.h"
#include "utility/mttytime.h"
#include "utility/json.h"
#include "utility/url.h"
#include "functions.h"
#include "platform.h"
#include "common/constants.h"

using namespace adservice::utility::json;
using namespace adservice::utility::url;


void paramTest(){
   const char* urlQuery="x=1&r=0ab740980000570470f710a20381bc5a&d=9&t=mm_56988911_9996733_33396840&e=mm_56988911_9996733_33396840&c=2476&f=http%3A%2F%2Fshow.mtty.com%2Fv%3Fp%3DAQq3QJgAAFcEcPcQogOBvFrWzmzeIed4TA%253D%253D%26a%3D0086-ffff-ffff%26b%3D30%26c%3D2476%26d%3D9%26e%3D307%26r%3D0ab740980000570470f710a20381bc5a%26s%3Dmm_56988911_9996733_33396840%26x%3D1%26tm%3D1459908855%26l%3Dhttp%253a%252f%252fclick.tanx.com%252fct%253ftanx_k%253d185%2526tanx_e%253dEsIwo4CZh8cNsMiJfeWrPF2mVfYHg13C5cWr8QsGxbApgnUw6x2sIcVTX6G4YhoOC0T3KGt5oIKHwUSKL%25252fbzByaGrxxkXlChGBW0itW0722UJnaJJ8e1s3br2sJGPAVwovAgk4YgQDFNnsZnatUxR8fKtJ5Tv8%25252buFn%25252fge5HOSb8aQ2cO91sWdA%25253d%25253d%2526tanx_u%253d&h=000&a=0086-ffff-ffff&url=http%3A%2F%2Fbdtg.9377a.com%2Fsousuotg.php%3Fid%3D11857%26uid%3D_2476";
   ParamMap paramMap;
   getParam(paramMap,urlQuery);
    std::cout<<"creativeId:"<<std::stol(paramMap[URL_CREATIVE_ID])<<std::endl;
}

int main(int argc,char** argv){
    paramTest();
    return 0;
}