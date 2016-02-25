//
// Created by guoze.lin on 16/2/24.
//

#ifndef ADCORE_URL_H
#define ADCORE_URL_H

#include <stddef.h>
#include <ctime>
#include <random>
#include <map>
#include <vector>
#include <iostream>
#include <strings.h>
#include <cstring>
#include <sstream>
#include <tuple>
#include "common/types.h"

namespace adservice{
    namespace utility{

        namespace url{


            std::string urlEncode(const std::string& input);

            std::string urlDecode(const std::string &input);

            /**
             * 快速url decode
             */
            void urlDecode_f(const adservice::types::string &input,adservice::types::string &output,char* buffer);

            typedef std::map<adservice::types::string,adservice::types::string> ParamMap;

            /**
             * 从buffer中的url query中获取参数,其中url query应该是被url encode的,格式为xxx=xx&xx=xxx
             */
            void getParam(ParamMap &m,const char* buffer,char seperator='&');

            void getParam(ParamMap& m,const adservice::types::string& input);

            /**
             * 从cookies中获取所有参数
             */
            void getCookiesParam(ParamMap &m,const char* buffer);

            /**
             * 从cookies串中提取目标参数
             */
            adservice::types::string extractCookiesParam(const adservice::types::string& key,const adservice::types::string& input);
        }

    }
}


#endif //ADCORE_URL_H
