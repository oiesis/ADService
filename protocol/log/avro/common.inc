
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
    // 广告主Id
    int advId;
    // 推广计划Id
    int cpid;
    // 推广单元Id
    int sid;
    // 创意Id
    int bid;
    // 点击Id
    int clickId;
    // 广告交换商Id
    int adxid ;
    // 网站Id
    int mid ;
    // 频道Id
    int cid ;
    // 广告位Id
    int pid ;
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
