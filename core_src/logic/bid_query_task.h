//
// Created by guoze.lin on 16/5/3.
//

#ifndef ADCORE_BID_QUERY_TASK_H
#define ADCORE_BID_QUERY_TASK_H

#include "abstract_query_task.h"
#include "protocol/baidu/baidu_bidding_handler.h"
#include "protocol/tanx/tanx_bidding_handler.h"
#include "protocol/youku/youku_bidding_handler.h"
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

        typedef std::map<int,AbstractBiddingHandler*> BiddingHandlerMap;

        struct BidThreadLocal{
            BiddingHandlerMap biddingHandlers;
            BidThreadLocal(){}
            ~BidThreadLocal(){
                if(!biddingHandlers.empty()){
                    typedef BiddingHandlerMap::iterator Iter;
                    for(Iter iter=biddingHandlers.begin();iter!=biddingHandlers.end();iter++){
                        if(iter->second)
                            delete iter->second;
                    }
                }
            }
            static void destructor(void* ptr){
                if(ptr){
                    delete ((BidThreadLocal*)ptr);
                }
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
            explicit HandleBidQueryTask(){}
            explicit HandleBidQueryTask(const TcpConnectionPtr& _conn,const HttpRequest& request):AbstractQueryTask(_conn,request){
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
