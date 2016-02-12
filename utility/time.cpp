//
// Created by guoze.lin on 16/1/25.
//

#include "utility.h"

namespace adservice{
    namespace utility{
        namespace time{

            const int MTTY_START_YEAR = 2015;
            const int MTTY_START_MON = 10;
            const int MTTY_START_DAY = 1;
            const int MTTY_START_HOUR = 11;
            const int MTTY_START_MIN = 11;
            const int MTTY_START_SEC = 11;

            /**
             * 假定麦田服务开始的基准时间
             * 假定为2015-10-01 11:11:11开始
             */
            int64_t mttyTimeBegin() {
                struct tm beginTime = {0};
                beginTime.tm_year = MTTY_START_YEAR - 1900;
                beginTime.tm_mon = MTTY_START_MON - 1;
                beginTime.tm_mday = MTTY_START_DAY;
                beginTime.tm_hour = MTTY_START_HOUR;
                beginTime.tm_min = MTTY_START_MIN;
                beginTime.tm_sec = MTTY_START_SEC;
                return (long) mktime(&beginTime);
            }

        }
    }
}
