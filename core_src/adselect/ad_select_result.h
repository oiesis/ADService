//
// Created by guoze.lin on 16/5/5.
//

#ifndef ADCORE_AD_SELECT_RESULT_H
#define ADCORE_AD_SELECT_RESULT_H

#include "utility/json.h"
#include <string>

namespace adservice{
    namespace adselect{

        struct SelectResult{
            rapidjson::Document* esResp;
            rapidjson::Value* finalSolution;
            rapidjson::Value* banner;
            rapidjson::Value* adplace;
            int bidPrice;
            SelectResult(){
                esResp = NULL;
                finalSolution = NULL;
                banner = NULL;
                adplace = NULL;
                bidPrice = 0;
            }
        };

        struct AdSelectCondition{
            std::string adxpid;
            std::string mttyPid;
            std::string excludeMediaType;
            std::string ip;
            std::string areaCode;
            std::string dHour;
            int width;
            int height;
            int mediaType;
            int adplaceType;
            int flowType;
            int displayNumber;
        };
    }
}


#endif //ADCORE_AD_SELECT_RESULT_H
