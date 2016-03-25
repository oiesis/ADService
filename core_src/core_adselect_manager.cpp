//
// Created by guoze.lin on 16/3/16.
//

#include "core_adselect_manager.h"
#include "atomic.h"

namespace adservice{
    namespace adselect{

        rapidjson::Value& AdSelectManager::queryCreativeById(int seqId,const std::string& bannerId,rapidjson::Document& result){
            ElasticSearch& agent = getAvailableConnection(seqId);
            int cnt = 0;
            try {
                char buffer[256];
                sprintf(buffer, dsl_query_banner, std::stoi(bannerId));
                cnt = agent.search(ES_INDEX_SOLUTIONS, ES_DOCUMENT_BANNER, ES_FILTER_FORMAT, buffer, result);
            }catch(std::exception& e){
                DebugMessage("exception in queryCreativeById,e:",e.what());
            }
            if(cnt!=1) {
                DebugMessageWithTime("error occured in queryCreativeById,document id:",bannerId);
                //throw AdSelectException("not exactly one docuemnts fetched in queryCreativeById", -1);
                return result["hits"];
            }
            return result["hits"]["hits"][0]["_source"];
        }
    }
}