//
// Created by guoze.lin on 16/5/3.
//

#include "bid_query_task.h"
#include "adselect/core_adselect_manager.h"
#include "adselect/ad_select_logic.h"
#include "protocol/baidu/baidu_bidding_handler.h"
#include "protocol/tanx/tanx_bidding_handler.h"
#include "protocol/youku/youku_bidding_handler.h"
#include "protocol/tencent_gdt/gdt_bidding_handler.h"

namespace adservice{
    namespace corelogic{

        int HandleBidQueryTask::initialized = 0;
        int HandleBidQueryTask::moduleCnt = 0;
        int HandleBidQueryTask::moduleIdx[BID_MAX_MODULES];
        struct ModuleIndex HandleBidQueryTask::moduleAdx[BID_MAX_MODULES];

#define ADD_MODULE_ENTRY(name,adxid) {moduleIdx[moduleCnt] = moduleCnt; \
        moduleAdx[moduleCnt++] = {fnv_hash(name,strlen(name)),adxid}; \
}
        /**
         * 初始化不同ADX的请求模块索引表
         */
        void HandleBidQueryTask::init() {
            if(initialized==1||!ATOM_CAS(&initialized,0,1))
                return;
            moduleCnt=0;

            //百度ADX
            ADD_MODULE_ENTRY(BID_QUERY_PATH_BAIDU,ADX_BAIDU);
            //淘宝ADX
            ADD_MODULE_ENTRY(BID_QUERY_PATH_TANX,ADX_TANX);
            //优酷ADX
            ADD_MODULE_ENTRY(BID_QUERY_PATH_YOUKU,ADX_YOUKU);
            //腾讯ADX
            ADD_MODULE_ENTRY(BID_QUERY_PATH_GDT,ADX_TENCENT_GDT);

            std::sort<int*>(moduleIdx,moduleIdx+moduleCnt,[moduleAdx](const int& a,const int& b)->bool{
                return moduleAdx[a].moduleHash<moduleAdx[b].moduleHash;
            });
        }

        /**
         * 根据模块路径获取对应的Adx
         */
        int HandleBidQueryTask::getAdxId(const std::string& path){
            int64_t key = fnv_hash(path.c_str(),path.length());
            int l=0,h=moduleCnt-1;
            while(l<=h){
                int mid = l+((h-l)>>1);
                if(key <= moduleAdx[moduleIdx[mid]].moduleHash)
                    h = mid-1;
                else
                    l = mid+1;
            }
            if(l<0||l>=moduleCnt||moduleAdx[moduleIdx[l]].moduleHash!=key){
                return 0;
            }
            return moduleAdx[moduleIdx[l]].adxId;
        }

        AbstractBiddingHandler* HandleBidQueryTask::getBiddingHandler(int adxId){
            switch(adxId){
                case ADX_TANX:
                    return new TanxBiddingHandler();
                case ADX_YOUKU:
                    return new YoukuBiddingHandler();
                case ADX_BAIDU:
                    return new BaiduBiddingHandler();
                case ADX_TENCENT_GDT:
                    return new GdtBiddingHandler();
                default:
                    return NULL;
            }
        }

        void HandleBidQueryTask::updateBiddingHandler() {
            if(biddingHandler==NULL){
                BidThreadLocal& bidData = threadData->bidData;
                BiddingHandlerMap::iterator iter;
                if((iter=bidData.biddingHandlers.find(adxId))==bidData.biddingHandlers.end()){
                    biddingHandler = getBiddingHandler(adxId);
                    bidData.biddingHandlers.insert(std::make_pair(adxId,biddingHandler));
                }else{
                    biddingHandler = iter->second;
                }
            }
        }

        void HandleBidQueryTask::getPostParam(ParamMap& paramMap){
            updateBiddingHandler();
            if(biddingHandler==NULL) {
                DebugMessageWithTime("Bidding Handler Not Found,adxId:",adxId);
                return;
            }
            if(!biddingHandler->parseRequestData(data)){
                DebugMessageWithTime("Parse Bidding Request Failed,adxId",adxId);
            }
        }

        void HandleBidQueryTask::customLogic(ParamMap& paramMap,protocol::log::LogItem& log,HttpResponse& resp){
            updateBiddingHandler();
            if(biddingHandler==NULL){
                log.reqStatus = 500;
            }else{
                TaskThreadLocal* localData = threadData;
                bool bidResult = biddingHandler->filter([localData](AbstractBiddingHandler* adapter,const AdSelectCondition& condition)->bool{
                    //连接ADSelect
                    AdSelectManager& adselect = AdSelectManager::getInstance();
                    int seqId = 0;
#ifndef NOUSE_QUERY_EXECUTOR_QUEUE
                    CoreModule coreModule = CoreService::getInstance();
                    if(coreModule.use_count()>0){
                        seqId = coreModule->getExecutor().getThreadSeqId();
                    }
#else
                    seqId = localData->seqId;
#endif
                    AdSelectLogic adSelectLogic(&adselect);
                    if(!adSelectLogic.selectByPid(seqId,condition.pid,true)){
                        return false;
                    }
                    adapter->buildBidResult(adSelectLogic.getResult());
                   return true;
                });
                if(bidResult){
                    biddingHandler->match(resp);
                }else{
                    biddingHandler->reject(resp);
                }
                biddingHandler->fillLogItem(log);
            }
        }

    }
}