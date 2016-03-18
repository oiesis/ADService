#ifndef ELASTICSEARCH_H
#define ELASTICSEARCH_H

#include <string>
#include <sstream>
#include <list>
#include <mutex>
#include <vector>

#include "http.h"

/// API class for elastic search server.
/// Node: Instance of elastic search on server represented by url:port
class ElasticSearch {
    public:
        ElasticSearch(const std::string& node, bool readOnly = false);
        ~ElasticSearch();

         /// Test connection with node.
        bool isActive();

        /// Request document number of type T in index I.
        long unsigned int getDocumentCount(const char* index, const char* type);

        /// Request the document by index/type/id.
        bool getDocument(const char* index, const char* type, const char* id, rapidjson::Document& msg);

        /// Request the document by index/type/ query key:value.
        void getDocument(const std::string& index, const std::string& type, const std::string& key, const std::string& value, rapidjson::Document& msg);

		/// Test if document exists
        bool exist(const std::string& index, const std::string& type, const std::string& id);

        /// Get Id of document
        bool getId(const std::string& index, const std::string& type, const std::string& key, const std::string& value, std::string& id);

        /// Search API of ES. Specify the doc type.
        long search(const std::string& index, const std::string& type, const std::string& query, rapidjson::Document& result);

        /// Search API
        long search(const std::string& index, const std::string& type, const std::string& searchParam,const std::string& query, rapidjson::Document& result);
    public:
        /// Test if index exists
        bool exist(const std::string& index);
    
    private:
        /// Private constructor.
        ElasticSearch();

        /// HTTP Connexion module.
        HTTP _http;

        /// Read Only option, all index functions return false.
        bool _readOnly;
};


#endif // ELASTICSEARCH_H
