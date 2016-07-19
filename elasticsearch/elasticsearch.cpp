#include "elasticsearch.h"

#include <iostream>
#include <sstream>
#include <cstring>
#include <cassert>
#include <locale>
#include <vector>
#include "common/constants.h"

using namespace adservice::utility::time;
using namespace adservice::utility::json;


size_t curl_read_cb(void* data,size_t s,size_t nmemb,void* param){
    rapidjson::Document& doc = *(rapidjson::Document*)param;
    size_t dataSize = s*nmemb;
    if(dataSize>0){
        parseJson((const char*)data,doc);
    }
    return dataSize;
}

#define SEARCH_PATH "/"ES_INDEX_SOLUTIONS"/"ES_DOCUMENT_SOLBANADPLACE"/_search"ES_FILTER_FORMAT2

ElasticSearch::ElasticSearch(const std::string& node, bool readOnly,const std::string& auth): _http(node, true,auth), _readOnly(readOnly) {
    // Test if instance is active.
    if(!isActive()) {
        _active = false;
        DebugMessageWithTime("Cannot create engine,elastic search is not active");
        //EXCEPTION("Cannot create engine, database is not active.");
    }else{
        _active = true;
    }
    curl = curl_easy_init();
    curl_easy_setopt(curl,CURLOPT_URL,(node+SEARCH_PATH).data());
    curl_easy_setopt(curl,CURLOPT_WRITEFUNCTION,curl_read_cb);
}


ElasticSearch::~ElasticSearch() {
    curl_easy_cleanup(curl);
}

// Test connection with node.
bool ElasticSearch::isActive() {

    rapidjson::Document root;

    try {
        _http.get(0, 0, root);
    }
    catch(ElasticSearchException& e){
        printf("get(0) failed in ElasticSearch::isActive(). ElasticSearchException caught: %s\n", e.what());
        _active = false;
        return false;
    }
    catch(std::exception& e){
        printf("get(0) failed in ElasticSearch::isActive(). std::exception caught: %s\n", e.what());
        _active = false;
        return false;
    }
    catch(...){
        printf("get(0) failed in ElasticSearch::isActive().\n");
        _active = false;
        return false;
    }

    if(root.ObjectEmpty()) {
        _active = false;
        return false;
    }

    if(!root.HasMember("status") || root["status"].GetInt() != 200){
        printf("Status is not 200. Cannot find Elasticsearch Node.\n");
        _active = false;
        return false;
    }
    _active = true;
    return true;
}

// Request the document by index/type/id.
bool ElasticSearch::getDocument(const char* index, const char* type, const char* id, rapidjson::Document& msg) {
    if(!this->good()){
        if(!this->isActive())
            return false;
    }
    std::ostringstream oss;
    oss << index << "/" << type << "/" << id;
    _http.get(oss.str().c_str(), 0, msg);
    return json::getField(msg,"found",false);
}



// Request the document by index/type/ query key:value.
void ElasticSearch::getDocument(const std::string& index, const std::string& type, const std::string& key, const std::string& value, rapidjson::Document& msg) {
    if(!this->good()){
        if(!this->isActive())
            return;
    }
    std::ostringstream oss;
    oss << index << "/" << type << "/_search";
    std::stringstream query;
    query << "{\"query\":{\"match\":{\""<< key << "\":\"" << value << "\"}}}";
    _http.post(oss.str().c_str(), query.str().c_str(), msg);
}


// Request the document number of type T in index I.
long unsigned int ElasticSearch::getDocumentCount(const char* index, const char* type) {
    if(!this->good()){
        if(!this->isActive())
            return 0;
    }
    std::ostringstream oss;
    oss << index << "/" << type << "/_count";
    rapidjson::Document msg;
    _http.get(oss.str().c_str(),0,msg);

    size_t pos = 0;
    if(msg.HasMember("count"))
        pos = msg["count"].GetInt();
    else
        printf("We did not find \"count\" member.\n");

    return pos;
}

// Test if document exists
bool ElasticSearch::exist(const std::string& index, const std::string& type, const std::string& id) {
    if(!this->good()){
        if(!this->isActive())
            return false;
    }
    std::stringstream url;
    url << index << "/" << type << "/" << id;

    rapidjson::Document result;
    _http.get(url.str().c_str(), 0, result);

    if(!result.HasMember("found")){
        std::cout << json::toJson(result) << std::endl;
        EXCEPTION("Database exception, field \"found\" must exist.");
    }

    return result["found"].GetBool();
}


/// Search API of ES.
long ElasticSearch::search(const std::string& index, const std::string& type, const std::string& query, rapidjson::Document& result) {
    if(!this->good()){
        if(!this->isActive())
            return 0;
    }
    std::stringstream url;
    url << index << "/" << type << "/_search";


    _http.post(url.str().c_str(), query.c_str(), result);

    if(!result.HasMember("timed_out")){
        std::cout << url.str() << " -d " << query << std::endl;
        std::cout << "result: " << json::toJson(result) << std::endl;
        EXCEPTION("Search failed.");
    }

    if(result["timed_out"].GetBool()){
        std::cout << "result: " << json::toJson(result) << std::endl;
        EXCEPTION("Search timed out.");
    }

    return result["hits"]["total"].GetInt();
}

long ElasticSearch::search(const std::string& index, const std::string& type, const std::string& searchParam,const std::string& query, rapidjson::Document& result) {
    if(!this->good()){ //初始状态不正常,再尝试一次
        if(!this->isActive()) //依然不正常
            return 0;
    }
    std::stringstream url;
    url << index << "/" << type << "/_search"<<searchParam;
//    DebugMessage("query string:",query.c_str());
    _http.post(url.str().c_str(), query.c_str(), result);
    if(result.ObjectEmpty()){
        DebugMessageWithTime("elasticsearch received empty result!!");
        return 0;
    }
    return result["hits"]["total"].GetInt();
}

long ElasticSearch::search2(const std::string& index, const std::string& type, const std::string& searchParam,const std::string& query, rapidjson::Document& result) {
    int64_t beginTime = getCurrentTimeStampMs();
    curl_easy_setopt(curl,CURLOPT_WRITEDATA,&result);
    curl_easy_setopt(curl,CURLOPT_POSTFIELDS,query.data());
    curl_easy_perform(curl);
    int64_t endTime = getCurrentTimeStampMs();
    return result["hits"]["total"].GetInt();
}


// Test if index exists
bool ElasticSearch::exist(const std::string& index) {
    rapidjson::Document doc;
    return (200 == _http.head(index.c_str(), 0, doc));
}



