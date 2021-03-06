#ifndef __CORE_MODEL_SOURCE_RECORD_H__
#define __CORE_MODEL_SOURCE_RECORD_H__

#include "utility/url.h"

#include <aerospike/as_record.h>


namespace adservice {
namespace core {
namespace model {

class SourceRecord {
public:
	SourceRecord() = default;

	SourceRecord(utility::url::ParamMap & paramMap, protocol::log::LogItem & log);

	~SourceRecord();

	void record(const as_record & record);
	as_record * record();

	time_t time() const;
	std::string advId() const;
	std::string sid() const;
	std::string adxId() const;
	std::string mtUid() const;
	std::string pid() const;
	std::string requestId() const;
	std::string createId() const;
	std::string geoId() const;
	std::string refererUrl() const;
	std::string bidPrice() const;

private:
	time_t time_{ 0 };				// 最后更新时间
	std::string advId_;			// 广告主ID
	std::string sid_;			// 推广单ID
	std::string adxId_;			// Adx平台ID
	std::string mtUid_;			// 麦田用户ID
	std::string pid_;			// 广告位ID
	std::string requestId_;		// 请求ID
	std::string createId_;		// 创意ID
	std::string geoId_;			// 地域ID
	std::string refererUrl_;	// 来源页
	std::string bidPrice_;		// 出价价格

	as_record * record_{ nullptr };
};

}	// namespace model
}	// namespace core
}	// namespace adservice

#endif	// __CORE_MODEL_SOURCE_RECORD_H__
