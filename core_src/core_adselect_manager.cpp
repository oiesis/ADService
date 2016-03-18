//
// Created by guoze.lin on 16/3/16.
//

#include "core_adselect_manager.h"
#include "atomic.h"

namespace adservice{
    namespace adselect{

        rapidjson::Value& AdSelectManager::queryCreativeById(const std::string& bannerId,rapidjson::Document& result){
            int cur = getAvailableConnection();
            ElasticSearch& agent = *(agents[cur]);
            int cnt = 0;
            try {
                char buffer[256];
                sprintf(buffer, dsl_query_banner, std::stoi(bannerId));
                cnt = agent.search(ES_INDEX_SOLUTIONS, ES_DOCUMENT_BANNER, ES_FILTER_FORMAT, buffer, result);
            }catch(std::exception& e){
                ATOM_CAS(&roundTable[cur],1,0);
            }
            if(cnt!=1) {
                DebugMessageWithTime("error occured in queryCreativeById,document id:",bannerId);
                throw AdSelectException("not exactly one docuemnts fetched in queryCreativeById", -1);
            }
            return result["hits"]["hits"][0]["_source"];
        }
    }
}