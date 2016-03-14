//
// Created by guoze.lin on 16/2/15.
//

#ifndef ADCORE_CONSTANTS_H
#define ADCORE_CONSTANTS_H

/**日志类别相关常量**/
// 点击日志生成器名
#define CLICK_SERVICE_LOGGER    "ClickServiceLog"

/**日志引擎相关默认参数*/
#define DEFAULT_KAFKA_BROKER    "192.168.31.147"
#define DEFAULT_KAFKA_TOPIC     "mt-new-click"
#define DEFAULT_KAFKA_KEY       "click"

#define DEFAULT_ALIYUN_PRODUCER_ID      "PID_mtty001"
#define DEFAULT_ALIYUN_TOPIC            "adlog"
#define DEFAULT_ALIYUN_ACCESS_KEY       "5jaQzkjjARFVFUrE"
#define DEFAULT_ALIYUN_SECRET_KEY       "SbFRrY6y1cnSKcdC0QpK1Vkv0QMmTw"

/**Http 参数相关常量**/
// cookies userId key
#define COOKIES_MTTY_ID         "m1"
// 服务根域名
#define COOKIES_MTTY_DOMAIN     "mtty.com"

/**时间相关常量**/
#define DAY_SECOND              86440

/**配置类别常量**/
// 服务器相关配置
#define CONFIG_SERVICE          "ServerConfig"
// 点击模块相关配置
#define CONFIG_CLICK            "ClickConfig"
// 日志相关的配置键
#define CONFIG_LOG              "LogConfig"

/**配置文件路径常量**/
// 服务配置文件
#define CONFIG_SERVICE_PATH     "conf/service.conf"
// 点击模块配置文件
#define CONFIG_CLICK_PATH       "conf/click.conf"
// 相关日志配置文件
#define CONFIG_LOG_PATH         "conf/log.conf"

/**核心逻辑模块常量**/
// 点击逻辑服务屏蔽字
#define MASK_CLICK              0X00000001
// 曝光逻辑服务屏蔽字
#define MASK_SHOW               0X00000002
// 竞价逻辑服务屏蔽字
#define MASK_BID                0X00000004
// 曝光统计模块服务屏蔽字
#define MASK_VIEW               0X00000008
// 跟踪逻辑模块服务屏蔽字
#define MASK_TRACK              0X00000010

#endif //ADCORE_CONSTANTS_H
