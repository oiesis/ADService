//
// Created by guoze.lin on 16/5/5.
//

#ifndef ADCORE_AD_SELECT_RESULT_H
#define ADCORE_AD_SELECT_RESULT_H

#include "utility/json.h"
#include <string>
#include <sstream>
#include <tuple>

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

        /**
         * 在查询过程中的预设广告位信息
         */
        struct PreSetAdplaceInfo{
            std::vector<std::tuple<int,int>> sizeArray;
            int flowType;
            PreSetAdplaceInfo(){
                flowType  = 0;
            }
        };

        struct AdSelectCondition{
            //adx平台广告位ID
            std::string adxpid;
            //mtty内部广告位ID
            std::string mttyPid;
            //需要匹配的ip地址
            std::string ip;
            //需要匹配的分时编码
            std::string dHour;
            //需要匹配的移动设备过滤条件字符串表示
            std::string mobileDeviceStr;
            //需要匹配的pc端浏览器类型过滤条件
            std::string pcBrowserStr;
            //需要匹配的优先条件,比如优酷的dealid
            std::string priorKey;
            //需要匹配的adxid
            int adxid;
            //需要匹配的地域编码
            int dGeo;
            //需要匹配的banner的宽
            int width;
            //需要匹配的banner的高
            int height;
            //需要匹配的媒体类型,编码为mtty定义的媒体类型
            int mediaType;
            //需要匹配的广告位类型,编码为mtty定义的广告位类型
            int adplaceType;
            //需要匹配的流量类型,编码为mtty定义的流量类型
            int flowType;
            //需要匹配的屏数
            int displayNumber;
            //需要匹配的pc端操作系统类型
            int pcOS;
            //需要匹配的移动端设备类型
            int mobileDevice;
            //需要匹配的网络类型
            int mobileNetwork;
            //需要匹配的运营商类型
            int mobileNetWorkProvider;
            // 预设的广告位信息,比如从ADX流量获取的信息填充到这里,可以省略在ES中对广告位的查询
            PreSetAdplaceInfo* pAdplaceInfo;

            AdSelectCondition(){
                dGeo = 0;
                width = 0;
                height = 0;
                mediaType = 0;
                adplaceType = 0;
                flowType = 0;
                displayNumber = 0;
                pcOS = 0;
                mobileDevice = 0;
                mobileNetwork = 0;
                mobileNetWorkProvider = 0;
                pAdplaceInfo = NULL;
            }

            std::string toString(){
                std::stringstream ss;
                ss<<"adxpid:"<<adxpid<<",";
                ss<<"mttypid:"<<mttyPid<<",";
                ss<<"ip:"<<ip<<",";
                ss<<"dHour:"<<dHour<<",";
                ss<<"adxid:"<<adxid<<",";
                ss<<"dGeo:"<<dGeo<<",";
                ss<<"width:"<<width<<",";
                ss<<"height:"<<height<<",";
                ss<<"mediaType:"<<mediaType<<",";
                ss<<"adplaceType:"<<adplaceType<<",";
                ss<<"flowType:"<<flowType<<",";
                ss<<"displayNumber:"<<displayNumber<<",";
                ss<<"pcBrowser:"<<pcBrowserStr<<",";
                ss<<"pcOs:"<<pcOS<<",";
                ss<<"mobileDevice:"<<mobileDevice<<",";
                ss<<"mobileNetwork:"<<mobileNetwork<<",";
                ss<<"mobileNetworkProvider:"<<mobileNetWorkProvider<<",";
                ss<<"priorKey:"<<priorKey;
                return ss.str();
            }

        };
    }
}


#endif //ADCORE_AD_SELECT_RESULT_H
