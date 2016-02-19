//
// Created by guoze.lin on 16/1/20.
//

#ifndef ADCORE_LOG_H
#define ADCORE_LOG_H

#include <sys/types.h>
#include <cstdlib>
#include <memory>
#include "protocol/log/log.h"

#ifdef linux
#include <ext/vstring.h>
#include <ext/vstring_fwd.h>

namespace adservice{
	namespace types{
		// c++11 small string optimization
		// https://gcc.gnu.org/ml/gcc-patches/2014-11/msg01785.html
		typedef __gnu_cxx::__sso_string string;
	}
}
#else
#include "muduo/base/Types.h"
namespace adservice{
	namespace types{
		typedef muduo::string string;
	}
}

#endif

#ifndef char_t
typedef char char_t;
#endif

#ifndef uchar_t
typedef u_char uchar_t;
#endif

#ifndef uint8_t
typedef u_int8_t uint8_t;
#endif

#ifndef uint16_t
typedef u_int16_t uint16_t;
#endif

#ifndef uint32_t
typedef u_int32_t uint32_t;
#endif

#ifndef uint64_t
typedef u_int64_t uint64_t;
#endif


#define IN
#define OUT
#define INOUT

namespace adservice {
	namespace types {


//		/** IPV4 地址组 */
//		typedef struct IP32_t {
//			int32_t addr;
//			struct IP32_t *next;
//		} IPV4_ADDR, *IPV4_POINTER;
//
//		/** IPV6 地址组 */
//		typedef struct IP128_t {
//			union {
//				uint64_t dword[2];
//				uint32_t word[4];
//				unsigned char b[16];
//			} addr;
//			struct IP128_t *next;
//		} IPV6_ADDR, *IPV6_POINTER;
//
//		typedef IPV4_POINTER IPPROXY_POINTER;
//
//		/** 用户使用的地址集合 */
//		typedef struct IPInfo {
//			/** ipv4 地址指针 */
//			IPV4_POINTER ipv4;
//			/** ipv6 地址指针 */
//			IPV6_POINTER ipv6;
//			/** 用户使用代理地址指针 */
//			IPPROXY_POINTER proxy;
//		} IPInfo, *PIPInfo;
//
//		typedef struct UserInfo {
//			/** 年龄 */
//			uint8_t age;
//			/** 性别 0:未知 1:男 2:女 */
//			uint8_t sex;
//			/** 兴趣编码 */
//			uint64_t interest;
//		} UserInfo;
//
//		typedef struct GeoInfo {
//			/** 纬度 */
//			char *latitude;
//			/** 经度 */
//			char *longtitude;
//			/** 国家编码 */
//			uint8_t country;
//			/** 省份编码 */
//			uint8_t province;
//			/** 城市编码 */
//			uint8_t city;
//			/** 区域编码 */
//			uint8_t district;
//			/** 街道编码 */
//			uint8_t street;
//		} GeoInfo;
//
//
//		typedef enum LogPhaseType : char {
//			BID, SHOW, VIEW, CLICK, TRACK, MAPPING
//		} LogPhaseType;
//
//		//广告信息
//		typedef struct AdInfo {
//			/** 广告主Id */
//			int advId;
//			/** 推广计划Id */
//			int cpid;
//			/** 推广单元Id */
//			int sid;
//			/** 创意Id */
//			int bid;
//			/** 点击Id */
//			int clickId;
//			/** 广告交换商Id */
//			int adxid;
//			/** 网站Id */
//			int mid;
//			/** 频道Id */
//			int cid;
//			/** 广告位Id */
//			int pid;
//			/** 广告的落地页面,仅在click模块有用 */
//			char *landingUrl;
//			/** 成本 */
//			int costPrice;
//			/** 投标价 */
//			int bidPrice;
//		} AdInfo;
//
//		typedef struct LogItem {
//			/** 处理请求时的Unix时间戳 */
//			long timeStamp;
//			/** 用户时区 */
//			uint8_t timeZone;
//			/** 日志产生的阶段 */
//			LogPhaseType logPhase;
//			/** 当前请求的http status */
//			uint16_t reqStatus;
//			/** 请求方法,true:Get false:Post*/
//			bool reqMethod;
//			/** 用户的ip信息 */
//			IPInfo ipInfo;
//			/** 请求来源 */
//			char *referer;
//			/** 请求的主机 */
//			char *host;
//			/** 请求的路径 */
//			char *path;
//			/** 用户ID,不同的阶段有不同的含义 */
//			char *userId;
//			/** 用户浏览器代理 */
//			char *userAgent;
//			/** 第三方平台提供的用户信息 */
//			UserInfo userInfo;
//			/** 第三方平台提供的地理信息 */
//			GeoInfo geoInfo;
//			/** 用户访问的页面的信息 */
//			char *pageInfo;
//			/** 前端脚本挖掘到的信息 */
//			char *jsInfo;
//			/** 第三方平台提供的用户设备信息 */
//			char *deviceInfo;
//			/** 跟踪Id */
//			int traceId;
//			/**  广告信息 */
//			AdInfo adInfo;
//		} LogItem, *PLogItem;
//
//		static inline bool isIpNull(IPV4_POINTER ipv4) {
//			return ipv4 == NULL;
//		}
//
//		static inline bool isIpNull(IPV6_POINTER ipv6) {
//			return ipv6 == NULL;
//		}
//
//		static inline IPV4_POINTER nextIpV4(IPV4_POINTER ipv4) {
//			return ipv4->next;
//		}
//
//		static inline IPV6_POINTER nextIpV6(IPV6_POINTER ipv6) {
//			return ipv6->next;
//		}
	}
}

#endif
