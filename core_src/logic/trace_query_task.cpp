#include "trace_query_task.h"

#include <aerospike/aerospike_key.h>

#include <boost/format.hpp>

#include "utility/aero_spike.h"
#include "model/source_record.h"


namespace adservice {
	namespace corelogic {

		namespace {

			const int TRACE_ID_ARRIVE =						5;

// 版本号
			const std::string URL_VERSION =					"v";
// 设备类型：1 PC端  2 移动端
			const std::string URL_DEVICE_TYPE =				"k";
/*
 * 请求类型：
 * 6 记录一次PV请求
 * 7 记录一次注册请求
 * 8 记录一次订单请求
 * 1001 记录一次自定义行为
 * 12 记录一次下载请求<mobile>
 * 13 记录一次安装请求<mobile>
 * 14 记录一次激活请求<mobile>
 */
			const std::string URL_REQUEST_TYPE =			"y";
// source_id
			const std::string URL_SOURCE_ID =				"g";
// 用户ID/订单ID/设备ID
			const std::string URL_USER_OR_ORDER_ID =		"t1";
// 用户名/商品名／设备ID类型 IDFA/IMEI/AndroidID
			const std::string URL_USER_OR_PRODUCT_NAME =	"t2";
// 商品ID
			const std::string URL_PRODUCT_ID =				"t3";
// 商品价格
			const std::string URL_PRODUCT_PRICE =			"t4";
// 商品数量
			const std::string URL_PRODUCT_QUANTITY =		"t5";
// 订单总金额
			const std::string URL_ORDER_PRICE =				"t6";
// 商品图片地址
			const std::string URL_PRODUCT_IMAGE_URL =		"t7";
// 商品URL地址
			const std::string URL_PRODUCT_URL =				"t8";

			const std::string URL_TAG9 =					"t9";

			const std::string URL_TAG10 =					"t10";

// sourceid
			bool getSourceId(const std::string & sourceIdIndex, std::string & sourceId)
			{
				if (!utility::AeroSpike::instance && !utility::AeroSpike::instance.connect()) {
					auto & error = utility::AeroSpike::instance.error();
					LOG_ERROR << "connect error, code:" << error.code << ", msg:" << error.message;
					return false;
				}

				as_error error;
				as_record * record = nullptr;

				/* 根据用户id和广告主id获取source_id */
				as_key key;
				as_key_init(&key, utility::AeroSpike::instance.nameSpace().c_str(), "source_id_index", sourceIdIndex.c_str());
				if (aerospike_key_get(utility::AeroSpike::instance.connection(), &error, nullptr, &key, &record) != AEROSPIKE_OK) {
					LOG_ERROR << "get index error, code:" << error.code << ", msg:" << error.message;
					as_key_destroy(&key);
					as_record_destroy(record);
					return false;
				}

				sourceId = as_record_get_str(record, "source_id");

				as_key_destroy(&key);
				as_record_destroy(record);
				return true;
			}

// sourceidindex
			bool getRecord(const std::string & sourceId, core::model::SourceRecord & sourceRecord)
			{
				if (!utility::AeroSpike::instance && !utility::AeroSpike::instance.connect()) {
					auto & error = utility::AeroSpike::instance.error();
					LOG_ERROR << "connect error, code:" << error.code << ", msg:" << error.message;
					return false;
				}

				/* 获取source_id的内容 */
				as_error error;
				as_record * record = nullptr;

				as_key key;
				as_key_init(&key, utility::AeroSpike::instance.nameSpace().c_str(), "source_id", sourceId.c_str());
				if (aerospike_key_get(utility::AeroSpike::instance.connection(), &error, nullptr, &key, &record) != AEROSPIKE_OK) {
					LOG_ERROR << "get error, code:" << error.code << ", msg:" << error.message;
					as_key_destroy(&key);
					as_record_destroy(record);
					return false;
				}

				sourceRecord.record(*record);

				return true;
			}

			void fillLog(protocol::log::LogItem & log,
						 ParamMap & paramMap,
						 const std::string & version,
						 const std::string & device,
						 const std::string & sourceId)
			{
				log.traceInfo.version = version;
				log.traceInfo.deviceType = device;
				log.traceInfo.sourceid = sourceId;
				log.traceInfo.tag1 = paramMap[URL_USER_OR_ORDER_ID];
				log.traceInfo.tag2 = paramMap[URL_USER_OR_PRODUCT_NAME];
				log.traceInfo.tag3 = paramMap[URL_PRODUCT_ID];
				log.traceInfo.tag4 = paramMap[URL_PRODUCT_PRICE];
				log.traceInfo.tag5 = paramMap[URL_PRODUCT_QUANTITY];
				log.traceInfo.tag6 = paramMap[URL_ORDER_PRICE];
				log.traceInfo.tag7 = paramMap[URL_PRODUCT_IMAGE_URL];
				log.traceInfo.tag8 = paramMap[URL_PRODUCT_URL];
				log.traceInfo.tag9 = paramMap[URL_TAG9];
				log.traceInfo.tag10 = paramMap[URL_TAG10];
			}

			std::string aliLog(const protocol::log::LogItem & log,
							   const core::model::SourceRecord & sourceRecord,
							   ParamMap & paramMap,
							   const std::string & ownerId,
							   const std::string & sourceId,
							   const std::string & requestTypeStr)
			{
				boost::format formatter("http://mtty.cn-beijing.log.aliyuncs.com/logstores/mt-log/track.gif?APIVersion=0.6.0\
&v=%1%\
&d=%2%\
&k=%3%\
&y=%4%\
&g=%5%\
&t1=%6%\
&t2=%7%\
&t3=%8%\
&t4=%9%\
&t5=%10%\
&t6=%11%\
&t7=%12%\
&t8=%13%\
&t9=%14%\
&t10=%15%\
&a=%16%\
&b=%17%\
&c=%18%\
&e=%19%\
&r=%20%\
&s=%21%\
&x=%22%\
&u=%23%");

				formatter % log.traceInfo.version
				% ownerId
				% log.traceInfo.deviceType
				% (log.traceId == TRACE_ID_ARRIVE ? std::to_string(log.traceId) : requestTypeStr)
				% sourceId
				% log.traceInfo.tag1
				% log.traceInfo.tag2
				% log.traceInfo.tag3
				% log.traceInfo.tag4
				% log.traceInfo.tag5
				% log.traceInfo.tag6
				% log.traceInfo.tag7
				% log.traceInfo.tag8
				% log.traceInfo.tag9
				% log.traceInfo.tag10
				% sourceRecord.geoId()
				% sourceRecord.bidPrice()
				% sourceRecord.createId()
				% sourceRecord.sid()
				% sourceRecord.requestId()
				% sourceRecord.pid()
				% sourceRecord.adxId()
				% log.userId;

				return formatter.str();
			}

		}	// anonymous namespace


		HandleTraceTask::HandleTraceTask(const HttpRequest & request, HttpResponse & response)
				: AbstractQueryTask(request, response)
		{
		}

		protocol::log::LogPhaseType HandleTraceTask::currentPhase()
		{
			return protocol::log::LogPhaseType::TRACK;
		}

		HttpResponse::HttpStatusCode HandleTraceTask::expectedReqStatus()
		{
			return HttpResponse::k302Redirect;
		}

		void HandleTraceTask::customLogic(ParamMap & paramMap, protocol::log::LogItem & log, HttpResponse & resp)
		{
			std::string userId = log.userId,
					ownerId = paramMap[URL_ADOWNER_ID],
					requestTypeStr = paramMap[URL_REQUEST_TYPE],
					version = paramMap[URL_VERSION],
					device = paramMap[URL_DEVICE_TYPE];
			if (userId.empty() || ownerId.empty() || requestTypeStr.empty() || version.empty() ||  device.empty()) {
				LOG_ERROR << "参数错误，缺少必须参数：用户id＝" << userId << "请求类型＝" << requestTypeStr
				<< "，版本＝" << version << "，广告主ID＝" << ownerId << "，设备＝" << device << "！" ;
				return;
			}

			std::string sourceIdIndex = userId + ownerId;

			std::string sourceId;
			core::model::SourceRecord sourceRecord;
			if (getSourceId(sourceIdIndex, sourceId)) {
				// 判断是否是一次到达
				if (getRecord(sourceId, sourceRecord)) {
					if (::time(nullptr) - sourceRecord.time() <= 10) {
						log.traceId = TRACE_ID_ARRIVE;
					}
				}
			}

			// 记录TraceInfo日志
			fillLog(log, paramMap, version, device, sourceId);

			std::string aliLogUrl = aliLog(log, sourceRecord, paramMap, ownerId, sourceId, requestTypeStr);

			// 跳转至阿里云日志服务
			resp.setStatusCode(HttpResponse::k302Redirect);
			resp.addHeader("Location", aliLogUrl);
			resp.setStatusMessage("OK");
		}

		void HandleTraceTask::onError(std::exception & e, HttpResponse & resp)
		{
			LOG_ERROR << "error occured in HandleTraceTask:" << e.what();
		}

	}	// namespace corelogic
}	// namespace adservice
