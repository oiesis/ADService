//
// Created by guoze.lin on 16/4/29.
//

#include "click_query_task.h"


namespace adservice{
    namespace corelogic{

        void HandleClickQueryTask::customLogic(ParamMap& paramMap,protocol::log::LogItem& log,HttpResponse& resp){
            if(!log.adInfo.landingUrl.empty()) {
                resp.setStatusCode(HttpResponse::k302Redirect);
                resp.addHeader("Location", log.adInfo.landingUrl);
                resp.setStatusMessage("OK");
            }else{
                resp.setStatusCode(HttpResponse::k400BadRequest);
                resp.setStatusMessage("Error,empty landing url");
            }
        }

    }
}