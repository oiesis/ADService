
protocol log{
record GeoInfo{
    // 纬度
    string latitude;
    // 经度
    string longitude;
    // 国家编码
    int country;
    // 省份编码
    int province;
    // 城市编码
    int city;
    // 区域编码
    int district;
    //街道编码
    int street;
}


record AdInfo{
    // 地域Id
    string areaId;
    // 广告主Id
    int advId;
    // 推广计划Id
    string cpid;
    // 推广单元Id
    string sid;
    // 创意Id
    string creativeId;
    // 点击Id
    string clickId;
    // 广告交换商Id
    string adxid ;
    // 网站Id
    string mid ;
    // 频道Id
    string cid ;
    // 广告位Id
    string pid ;
    // 曝光Id
    string imp_id;
    // 广告的落地页面,仅在click模块有用
    string landingUrl;
    // 成本
    int cost;
    // 投标价
    int bidPrice;
}

enum LogPhaseType {
    BID,SHOW,VIEW,CLICK,TRACK,MAPPING
}

record IPInfo{
    array<string> ipv4;
    array<string> ipv6;
    array<string> proxy;
}

record UserInfo{
    int age;
    int sex;
    int interest;
}

}
