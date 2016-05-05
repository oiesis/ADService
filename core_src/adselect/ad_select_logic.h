//
// Created by guoze.lin on 16/5/4.
//

#ifndef ADCORE_AD_SELECT_LOGIC_H
#define ADCORE_AD_SELECT_LOGIC_H


#include "ad_select_result.h"
#include "core_adselect_manager.h"


namespace adservice{
    namespace adselect{

        class AdSelectLogic{
        public:
            AdSelectLogic(AdSelectManager* selectClient):esResp(rapidjson::kObjectType),adselect(selectClient){
            }

            /**
             * 根据广告位进行的广告选择逻辑
             */
            bool selectByPid(int seqId,const std::string& queryPid,bool isAdxPid);

            inline const SelectResult& getResult() const{
                return selectResult;
            }
        private:
            rapidjson::Document esResp;
            AdSelectManager* adselect;
            SelectResult selectResult;
        };

    }
}

#endif //ADCORE_AD_SELECT_LOGIC_H
