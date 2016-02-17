/**
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */


#ifndef CLICK_AVRO_CLICK_H_659754449__H_
#define CLICK_AVRO_CLICK_H_659754449__H_


#include <sstream>
#include "boost/any.hpp"
#include "avro/Specific.hh"
#include "avro/Encoder.hh"
#include "avro/Decoder.hh"
#include <ostream>

namespace protocol{
namespace click {
struct GeoInfo {
    std::string latitude;
    std::string longitude;
    int32_t country;
    int32_t province;
    int32_t city;
    int32_t district;
    int32_t street;
    GeoInfo() :
        latitude(std::string()),
        longitude(std::string()),
        country(int32_t()),
        province(int32_t()),
        city(int32_t()),
        district(int32_t()),
        street(int32_t())
        { }
};

struct AdInfo {
    int32_t advId;
    int32_t cpid;
    int32_t sid;
    int32_t bid;
    int32_t clickId;
    int32_t adxid;
    int32_t mid;
    int32_t cid;
    int32_t pid;
    std::string landingUrl;
    int32_t cost;
    int32_t bidPrice;
    AdInfo() :
        advId(int32_t()),
        cpid(int32_t()),
        sid(int32_t()),
        bid(int32_t()),
        clickId(int32_t()),
        adxid(int32_t()),
        mid(int32_t()),
        cid(int32_t()),
        pid(int32_t()),
        landingUrl(std::string()),
        cost(int32_t()),
        bidPrice(int32_t())
        { }
};

struct ClickRequest {
    std::string cookiesId;
    int32_t age;
    int32_t sex;
    GeoInfo geoInfo;
    AdInfo adInfo;
    ClickRequest() :
        cookiesId(std::string()),
        age(int32_t()),
        sex(int32_t()),
        geoInfo(GeoInfo()),
        adInfo(AdInfo())
        { }
};

struct ClickResponse {
    std::string cookiesId;
    int32_t age;
    int32_t sex;
    GeoInfo geoInfo;
    AdInfo adInfo;
    ClickResponse() :
        cookiesId(std::string()),
        age(int32_t()),
        sex(int32_t()),
        geoInfo(GeoInfo()),
        adInfo(AdInfo())
        { }
};

struct __tmp_json_Union__0__ {
private:
    size_t idx_;
    boost::any value_;
public:
    size_t idx() const { return idx_; }
    GeoInfo get_GeoInfo() const;
    void set_GeoInfo(const GeoInfo& v);
    AdInfo get_AdInfo() const;
    void set_AdInfo(const AdInfo& v);
    ClickRequest get_ClickRequest() const;
    void set_ClickRequest(const ClickRequest& v);
    ClickResponse get_ClickResponse() const;
    void set_ClickResponse(const ClickResponse& v);
    __tmp_json_Union__0__();
};

inline
GeoInfo __tmp_json_Union__0__::get_GeoInfo() const {
    if (idx_ != 0) {
        throw avro::Exception("Invalid type for union");
    }
    return boost::any_cast<GeoInfo >(value_);
}

inline
void __tmp_json_Union__0__::set_GeoInfo(const GeoInfo& v) {
    idx_ = 0;
    value_ = v;
}

inline
AdInfo __tmp_json_Union__0__::get_AdInfo() const {
    if (idx_ != 1) {
        throw avro::Exception("Invalid type for union");
    }
    return boost::any_cast<AdInfo >(value_);
}

inline
void __tmp_json_Union__0__::set_AdInfo(const AdInfo& v) {
    idx_ = 1;
    value_ = v;
}

inline
ClickRequest __tmp_json_Union__0__::get_ClickRequest() const {
    if (idx_ != 2) {
        throw avro::Exception("Invalid type for union");
    }
    return boost::any_cast<ClickRequest >(value_);
}

inline
void __tmp_json_Union__0__::set_ClickRequest(const ClickRequest& v) {
    idx_ = 2;
    value_ = v;
}

inline
ClickResponse __tmp_json_Union__0__::get_ClickResponse() const {
    if (idx_ != 3) {
        throw avro::Exception("Invalid type for union");
    }
    return boost::any_cast<ClickResponse >(value_);
}

inline
void __tmp_json_Union__0__::set_ClickResponse(const ClickResponse& v) {
    idx_ = 3;
    value_ = v;
}

inline __tmp_json_Union__0__::__tmp_json_Union__0__() : idx_(0), value_(GeoInfo()) { }
}
}
namespace avro {
template<> struct codec_traits<protocol::click::GeoInfo> {
    static void encode(Encoder& e, const protocol::click::GeoInfo& v) {
        avro::encode(e, v.latitude);
        avro::encode(e, v.longitude);
        avro::encode(e, v.country);
        avro::encode(e, v.province);
        avro::encode(e, v.city);
        avro::encode(e, v.district);
        avro::encode(e, v.street);
    }
    static void decode(Decoder& d, protocol::click::GeoInfo& v) {
        if (avro::ResolvingDecoder *rd =
            dynamic_cast<avro::ResolvingDecoder *>(&d)) {
            const std::vector<size_t> fo = rd->fieldOrder();
            for (std::vector<size_t>::const_iterator it = fo.begin();
                it != fo.end(); ++it) {
                switch (*it) {
                case 0:
                    avro::decode(d, v.latitude);
                    break;
                case 1:
                    avro::decode(d, v.longitude);
                    break;
                case 2:
                    avro::decode(d, v.country);
                    break;
                case 3:
                    avro::decode(d, v.province);
                    break;
                case 4:
                    avro::decode(d, v.city);
                    break;
                case 5:
                    avro::decode(d, v.district);
                    break;
                case 6:
                    avro::decode(d, v.street);
                    break;
                default:
                    break;
                }
            }
        } else {
            avro::decode(d, v.latitude);
            avro::decode(d, v.longitude);
            avro::decode(d, v.country);
            avro::decode(d, v.province);
            avro::decode(d, v.city);
            avro::decode(d, v.district);
            avro::decode(d, v.street);
        }
    }
};

template<> struct codec_traits<protocol::click::AdInfo> {
    static void encode(Encoder& e, const protocol::click::AdInfo& v) {
        avro::encode(e, v.advId);
        avro::encode(e, v.cpid);
        avro::encode(e, v.sid);
        avro::encode(e, v.bid);
        avro::encode(e, v.clickId);
        avro::encode(e, v.adxid);
        avro::encode(e, v.mid);
        avro::encode(e, v.cid);
        avro::encode(e, v.pid);
        avro::encode(e, v.landingUrl);
        avro::encode(e, v.cost);
        avro::encode(e, v.bidPrice);
    }
    static void decode(Decoder& d, protocol::click::AdInfo& v) {
        if (avro::ResolvingDecoder *rd =
            dynamic_cast<avro::ResolvingDecoder *>(&d)) {
            const std::vector<size_t> fo = rd->fieldOrder();
            for (std::vector<size_t>::const_iterator it = fo.begin();
                it != fo.end(); ++it) {
                switch (*it) {
                case 0:
                    avro::decode(d, v.advId);
                    break;
                case 1:
                    avro::decode(d, v.cpid);
                    break;
                case 2:
                    avro::decode(d, v.sid);
                    break;
                case 3:
                    avro::decode(d, v.bid);
                    break;
                case 4:
                    avro::decode(d, v.clickId);
                    break;
                case 5:
                    avro::decode(d, v.adxid);
                    break;
                case 6:
                    avro::decode(d, v.mid);
                    break;
                case 7:
                    avro::decode(d, v.cid);
                    break;
                case 8:
                    avro::decode(d, v.pid);
                    break;
                case 9:
                    avro::decode(d, v.landingUrl);
                    break;
                case 10:
                    avro::decode(d, v.cost);
                    break;
                case 11:
                    avro::decode(d, v.bidPrice);
                    break;
                default:
                    break;
                }
            }
        } else {
            avro::decode(d, v.advId);
            avro::decode(d, v.cpid);
            avro::decode(d, v.sid);
            avro::decode(d, v.bid);
            avro::decode(d, v.clickId);
            avro::decode(d, v.adxid);
            avro::decode(d, v.mid);
            avro::decode(d, v.cid);
            avro::decode(d, v.pid);
            avro::decode(d, v.landingUrl);
            avro::decode(d, v.cost);
            avro::decode(d, v.bidPrice);
        }
    }
};

template<> struct codec_traits<protocol::click::ClickRequest> {
    static void encode(Encoder& e, const protocol::click::ClickRequest& v) {
        avro::encode(e, v.cookiesId);
        avro::encode(e, v.age);
        avro::encode(e, v.sex);
        avro::encode(e, v.geoInfo);
        avro::encode(e, v.adInfo);
    }
    static void decode(Decoder& d, protocol::click::ClickRequest& v) {
        if (avro::ResolvingDecoder *rd =
            dynamic_cast<avro::ResolvingDecoder *>(&d)) {
            const std::vector<size_t> fo = rd->fieldOrder();
            for (std::vector<size_t>::const_iterator it = fo.begin();
                it != fo.end(); ++it) {
                switch (*it) {
                case 0:
                    avro::decode(d, v.cookiesId);
                    break;
                case 1:
                    avro::decode(d, v.age);
                    break;
                case 2:
                    avro::decode(d, v.sex);
                    break;
                case 3:
                    avro::decode(d, v.geoInfo);
                    break;
                case 4:
                    avro::decode(d, v.adInfo);
                    break;
                default:
                    break;
                }
            }
        } else {
            avro::decode(d, v.cookiesId);
            avro::decode(d, v.age);
            avro::decode(d, v.sex);
            avro::decode(d, v.geoInfo);
            avro::decode(d, v.adInfo);
        }
    }
};

template<> struct codec_traits<protocol::click::ClickResponse> {
    static void encode(Encoder& e, const protocol::click::ClickResponse& v) {
        avro::encode(e, v.cookiesId);
        avro::encode(e, v.age);
        avro::encode(e, v.sex);
        avro::encode(e, v.geoInfo);
        avro::encode(e, v.adInfo);
    }
    static void decode(Decoder& d, protocol::click::ClickResponse& v) {
        if (avro::ResolvingDecoder *rd =
            dynamic_cast<avro::ResolvingDecoder *>(&d)) {
            const std::vector<size_t> fo = rd->fieldOrder();
            for (std::vector<size_t>::const_iterator it = fo.begin();
                it != fo.end(); ++it) {
                switch (*it) {
                case 0:
                    avro::decode(d, v.cookiesId);
                    break;
                case 1:
                    avro::decode(d, v.age);
                    break;
                case 2:
                    avro::decode(d, v.sex);
                    break;
                case 3:
                    avro::decode(d, v.geoInfo);
                    break;
                case 4:
                    avro::decode(d, v.adInfo);
                    break;
                default:
                    break;
                }
            }
        } else {
            avro::decode(d, v.cookiesId);
            avro::decode(d, v.age);
            avro::decode(d, v.sex);
            avro::decode(d, v.geoInfo);
            avro::decode(d, v.adInfo);
        }
    }
};

template<> struct codec_traits<protocol::click::__tmp_json_Union__0__> {
    static void encode(Encoder& e, protocol::click::__tmp_json_Union__0__ v) {
        e.encodeUnionIndex(v.idx());
        switch (v.idx()) {
        case 0:
            avro::encode(e, v.get_GeoInfo());
            break;
        case 1:
            avro::encode(e, v.get_AdInfo());
            break;
        case 2:
            avro::encode(e, v.get_ClickRequest());
            break;
        case 3:
            avro::encode(e, v.get_ClickResponse());
            break;
        }
    }
    static void decode(Decoder& d, protocol::click::__tmp_json_Union__0__& v) {
        size_t n = d.decodeUnionIndex();
        if (n >= 4) { throw avro::Exception("Union index too big"); }
        switch (n) {
        case 0:
            {
                protocol::click::GeoInfo vv;
                avro::decode(d, vv);
                v.set_GeoInfo(vv);
            }
            break;
        case 1:
            {
                protocol::click::AdInfo vv;
                avro::decode(d, vv);
                v.set_AdInfo(vv);
            }
            break;
        case 2:
            {
                protocol::click::ClickRequest vv;
                avro::decode(d, vv);
                v.set_ClickRequest(vv);
            }
            break;
        case 3:
            {
                protocol::click::ClickResponse vv;
                avro::decode(d, vv);
                v.set_ClickResponse(vv);
            }
            break;
        }
    }
};

}
#endif
