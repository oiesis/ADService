//
// Created by guoze.lin on 16/5/3.
//

#ifndef ADCORE_ABSTRACT_BIDDING_HANDLER_H
#define ADCORE_ABSTRACT_BIDDING_HANDLER_H

#include <functional>
#include <string>
#include "protocol/log/log.h"
#include "muduo/net/http/HttpResponse.h"
#include "common/types.h"
#include "adselect/ad_select_result.h"
#include "constants.h"
#include "functions.h"

namespace protocol{
    namespace bidding{

        using namespace muduo::net;
        using namespace adservice::adselect;

        class AbstractBiddingHandler;

        typedef std::function<bool(AbstractBiddingHandler*,AdSelectCondition&)> BiddingFilterCallback;

        /**
         * BiddingHandler主要处理协议层数据转换,屏蔽各个平台之间输入输出的差异
         * 实际上应该叫做BiddingDataAdapter
         */
        class AbstractBiddingHandler{
        public:

            AbstractBiddingHandler():isBidAccepted(false){}

            /**
             * 从Adx Bid Post请求数据中获取具体的请求信息
             */
            virtual bool parseRequestData(const std::string& data) = 0;

            /**
             * 根据Bid 的相关信息对日志进行信息填充
             */
            virtual void fillLogItem(protocol::log::LogItem& logItem){}

            /**
             * 根据ADX的请求进行竞价匹配,决定是否接受这个流量,同时设置isBidAccepted
             * @return: true接受流量,false不接受流量
             */
            virtual bool filter(const BiddingFilterCallback& filterCb){isBidAccepted=false;return false;}

            /**
             * 将匹配结果转换为具体平台的格式的结果
             */
            virtual void buildBidResult(const AdSelectCondition& selectCondition,const SelectResult& result) = 0;

            /**
             * 当接受流量时装配合适的输出
             */
            virtual void match(INOUT HttpResponse& response) = 0;

            /**
             * 不接受ADX的流量请求
             */
            virtual void reject(INOUT HttpResponse& response) = 0;

            /**
             * 产生htmlsnippet
             */
            virtual std::string generateHtmlSnippet(const std::string &bid, int width, int height, const char *extShowBuf,
                                                    const char *cookieMappingUrl = "");

            virtual std::string generateScript(const std::string &bid,int width,int height,const char* scriptUrl,const char* clickMacro,const char* extParam);

            inline bool bidFailedReturn(){
                return (isBidAccepted = false);
            }

            inline bool bidSuccessReturn(){
                return (isBidAccepted = true);
            }
        protected:
            void getShowPara(const std::string& bid,char* showParamBuf,int showBufSize);
            void getClickPara(const std::string& bid,char* clickParamBuf,int clickBufSize,const std::string& ref,const std::string& landingurl);
            int extractRealValue(const std::string& input,int adx);
        protected:
            //最近一次匹配的结果
            bool isBidAccepted;
            protocol::log::AdInfo adInfo;
        };

    }
}

#endif //ADCORE_ABSTRACT_BIDDING_HANDLER_H
