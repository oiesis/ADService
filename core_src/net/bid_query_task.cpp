//
// Created by guoze.lin on 16/5/3.
//

#include "bid_query_task.h"
#include "protocol/"

namespace adservice{
    namespace corelogic{

        int HandleBidQueryTask::initialized = 0;
        int HandleBidQueryTask::moduleCnt = 0;
        int HandleBidQueryTask::moduleIdx[BID_MAX_MODULES];
        ModuleIndex HandleBidQueryTask::moduleAdx[BID_MAX_MODULES];

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
                default:
                    return NULL;
            }
        }

        void HandleBidQueryTask::updateBiddingHandler() {
            if(biddingHandler==NULL){
                pthread_t thread = pthread_self();
                BidThreadLocal* data = (BidThreadLocal*)ThreadLocalManager::getInstance().get(thread);
                if(data==NULL){
                    data = new BidThreadLocal;
                    ThreadLocalManager::getInstance().put(thread,data,&BidThreadLocal::destructor);
                }
                BiddingHandlerMap::iterator iter;
                if((iter=data->biddingHandlers.find(adxId))==data->biddingHandlers.end()){
                    biddingHandler = getBiddingHandler(adxId);
                    data->insert(std::make_pair(adxId,biddingHandler));
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
            biddingHandler->parseRequestData(data);
        }

        void HandleBidQueryTask::customLogic(ParamMap& paramMap,protocol::log::LogItem& log,HttpResponse& resp){
            updateBiddingHandler();
            if(biddingHandler==NULL){
                log.reqStatus = 500;
            }else{
                biddingHandler->match(resp);
                biddingHandler->fillLogItem(log);
            }
        }

    }
}