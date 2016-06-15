//
// Created by guoze.lin on 16/5/3.
//

#ifndef ADCORE_BID_QUERY_TASK_H
#define ADCORE_BID_QUERY_TASK_H

#include "abstract_query_task.h"
#include "protocol/base/abstract_bidding_handler.h"
#include "core_ip_manager.h"
#include <initializer_list>

namespace adservice{
    namespace corelogic{

        using namespace protocol::bidding;

        static const int BID_MAX_MODULES = 10;

        struct ModuleIndex{
            int64_t moduleHash;
            int64_t adxId;
            ModuleIndex(){}
            ModuleIndex(int64_t h,int64_t id){
                moduleHash = h;
                adxId  = id;
            }
            ModuleIndex(const std::initializer_list<int64_t>& list){
                const int64_t* b = list.begin();
                moduleHash = b[0];
                adxId = b[1];
            }
        };


        /**
         * 处理竞价模块逻辑的类
         */
        class HandleBidQueryTask : public AbstractQueryTask{
        public:
            static int initialized;
            static int moduleCnt;
            static struct ModuleIndex moduleAdx[BID_MAX_MODULES];
            static int moduleIdx[BID_MAX_MODULES];
            static void init();
            static int getAdxId(const std::string& path);
            static AbstractBiddingHandler* getBiddingHandler(int adxId);
        public:
            explicit HandleBidQueryTask(const HttpRequest& request,HttpResponse& response):AbstractQueryTask(request,response){
                adxId = getAdxId(request.path());
                biddingHandler = NULL;
            }

            void updateBiddingHandler();

            protocol::log::LogPhaseType currentPhase(){
                return protocol::log::LogPhaseType::BID;
            }

            // 期望http 请求状态
            HttpResponse::HttpStatusCode expectedReqStatus(){
                return HttpResponse::k200Ok;
            }

            void getPostParam(ParamMap& paramMap);

            void customLogic(ParamMap& paramMap,protocol::log::LogItem& log,HttpResponse& response);

            void onError(std::exception& e,HttpResponse& resp){
                LOG_ERROR<<"error occured in HandleBidQueryTask:"<<e.what()<<",query:"<<data;
            }
        private:
            int adxId;
            AbstractBiddingHandler* biddingHandler;
        };

    }
}

#endif //ADCORE_BID_QUERY_TASK_H
