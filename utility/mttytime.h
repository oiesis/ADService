//
// Created by guoze.lin on 16/2/24.
//

#ifndef ADCORE_TIME_H
#define ADCORE_TIME_H

#include <stddef.h>
#include <ctime>
#include "common/types.h"

namespace adservice{
    namespace utility{

        namespace time{

            //假定服务器在东八区

            static const int64_t MTTY_SERVICE_TIME_BEGIN = 1443669071L;//mttyTimeBegin();
            static const int32_t timeZone = 8;
            static constexpr int32_t UTC_TIME_DIFF_SEC = timeZone * 3600;

            inline int64_t getMttyTimeBegin(){
                return MTTY_SERVICE_TIME_BEGIN;
            }

            inline int64_t getCurrentTimeStamp(){
                time_t currentTime;
                ::time(&currentTime);
                return (int64_t)currentTime;
            }

            inline int32_t getCurrentTimeSinceMtty(){
                long currentTime = getCurrentTimeStamp();
                return (int32_t)(currentTime - MTTY_SERVICE_TIME_BEGIN);
            }

            /**
             * mtty时间到本地Unix时间戳
             */
            inline int64_t mttyTimeToUnixTimeStamp(int32_t mttyTime){
                return mttyTime+MTTY_SERVICE_TIME_BEGIN;
            }

            /**
             * 本地时间戳到UTC时间戳
             */
            inline int64_t localTimeStamptoUtc(int64_t unixTimeStamp){
                return unixTimeStamp - UTC_TIME_DIFF_SEC;
            }

            inline int64_t getCurrentTimeStampUtc(){
                return localTimeStamptoUtc(getCurrentTimeStamp());
            }

            /**
             * 用于离线计算mtty基准时间
             */
            int64_t mttyTimeBegin();

            /**
             * 获取当前时间偏离当日零点的偏移秒数
             */
            int32_t getTimeSecondOfToday();

            /**
             * 获取当日零点Unix时间戳
             */
            int64_t getTodayStartTime();

        }
    }
}

#endif //ADCORE_TIME_H
