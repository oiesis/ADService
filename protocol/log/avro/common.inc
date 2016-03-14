
protocol log{
@namespace("com.mtty.protocol")
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

@namespace("com.mtty.protocol")
record AdInfo{
    // 地域Id,可能为空,取决于接收到的链接
    string areaId;
    // 广告主Id,可能为空,取决于接收到的链接
    long advId;
    // 推广计划Id,可能为空,取决于接收到的链接
    string cpid;
    // 推广单元Id,可能为空,取决于接收到的链接
    long sid;
    // 创意Id,可能为空,取决于接收到的链接
    long creativeId;
    // 点击Id,可能为空,取决于接收到的链接
    string clickId;
    // 广告交换商Id,可能为空,取决于接收到的链接
    string adxid ;
    // 网站Id,可能为空,取决于接收到的链接
    string mid ;
    // 频道Id,可能为空,取决于接收到的链接
    string cid ;
    // 广告位Id,可能为空,取决于接收到的链接
    long pid ;
    // 平台Id,可能为空,取决于接收到的链接
    long unionId;
    // 曝光Id,可能为空,取决于接收到的链接
    string imp_id;
    // 广告的落地页面,click模块非空
    string landingUrl;
    // 成本,点击模块为空
    int cost;
    // 投标价,点击模块为空
    int bidPrice;
}

@namespace("com.mtty.protocol")
enum LogPhaseType {
    BID,SHOW,VIEW,CLICK,TRACK,MAPPING
}

@namespace("com.mtty.protocol")
record IPInfo{
    int ipv4;
    array<int> ipv6;
    string proxy;
}

@namespace("com.mtty.protocol")
record UserInfo{
    int age;
    int sex;
    int interest;
}

}
