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
            rapidjson::Value* finalSolution;
            rapidjson::Value* banner;
            rapidjson::Value* adplace;
            int bidPrice;
            SelectResult(){
                finalSolution = NULL;
                banner = NULL;
                adplace = NULL;
                bidPrice = 0;
            }
        };

        struct AdSelectCondition{
            std::string pid;
            int width;
            int height;
        };
    }
}


#endif //ADCORE_AD_SELECT_RESULT_H
