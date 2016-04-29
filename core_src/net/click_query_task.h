//
// Created by guoze.lin on 16/4/29.
//

#ifndef ADCORE_CLICK_QUERY_TASK_H
#define ADCORE_CLICK_QUERY_TASK_H

#include "abstract_query_task.h"

namespace adservice{
    namespace corelogic{

        /**
         * 处理点击模块逻辑的类
         */
        class HandleClickQueryTask:public AbstractQueryTask{
        public:
            explicit HandleClickQueryTask(const TcpConnectionPtr& _conn,const HttpRequest& request):AbstractQueryTask(_conn,request){
            }

            protocol::log::LogPhaseType currentPhase(){
                return protocol::log::LogPhaseType::CLICK;
            }

            // 期望http 请求状态
            HttpResponse::HttpStatusCode expectedReqStatus(){
                return HttpResponse::k302Redirect;
            }

            void customLogic(ParamMap& paramMap,protocol::log::LogItem& log,HttpResponse& resp);

            void onError(std::exception& e,HttpResponse& resp){
                LOG_ERROR<<"error occured in HandleClickQueryTask:"<<e.what();
            }
        };
    }
}

#endif //ADCORE_CLICK_QUERY_TASK_H
