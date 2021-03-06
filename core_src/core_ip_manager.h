//
// Created by guoze.lin on 16/6/6.
//

#ifndef ADCORE_CORE_IP_MANAGER_H
#define ADCORE_CORE_IP_MANAGER_H

#include "muduo/base/tbb/concurrent_hash_map.h"
#include "common/constants.h"

namespace adservice{
    namespace server{

        struct AreaItem{
            std::string name;
            int country;
            int province;
            int city;
            AreaItem(){}
            AreaItem(const std::string& n,int _country,int _province,int _city){
                name = n;
                country = _country;
                province = _province;
                city = _city;
            }
            AreaItem(const AreaItem& areaItem){
                name = areaItem.name;
                country = areaItem.country;
                province = areaItem.province;
                city = areaItem.city;
            }
        };

        typedef tbb::concurrent_hash_map<std::string,AreaItem> AreaDict;
        typedef AreaDict::accessor AreaDictAccessor;

        class IpManager{
        public:
            static void init(const char* ipdataFile = IP_DATA_FILE,const char* areaCodeFile = AREA_CODE_FILE);
            static void destroy();
            static void loadAreaData(const char* areafile);
            static IpManager& getInstance(){
                static IpManager ipManager;
                if(started==0)
                    ipManager.init();
                return ipManager;
            }
            static AreaDict areaDict;
            static int started;
        public:
            IpManager(){}
            IpManager(const IpManager&) = delete;
            int getAreaByIp(const char* ip);
            std::string getAreaCodeStrByIp(const char* ip);
        };

    }
}

#endif //ADCORE_CORE_IP_MANAGER_H
