//
// Created by guoze.lin on 16/3/18.
//
#define UNIT_TEST
#include <string>
#include <iostream>
#include <map>
#include <vector>
#include "common/functions.h"
#include "utility/cypher.h"
#include "utility/mttytime.h"
#include "utility/json.h"
#include "functions.h"
#include "platform.h"

using namespace adservice::utility::json;

void json_test(){
    const char* str="{\n"
            "  \"hits\" : {\n"
            "    \"total\" : 1,\n"
            "    \"hits\" : [ {\n"
            "      \"_source\" : {\n"
            "        \"adplacename\" : \"200x200,2012-7-10 12:47:37\",\n"
            "        \"adplacestatus\" : 0,\n"
            "        \"adplacetype\" : 0,\n"
            "        \"adxadplacename\" : \"200x200,2012-7-10 12:47:37\",\n"
            "        \"adxadplacetype\" : 0,\n"
            "        \"adxid\" : 1,\n"
            "        \"adxmediatype\" : 1,\n"
            "        \"adxpid\" : \"mm_10009252_2195968_10466618\",\n"
            "        \"baseprice\" : null,\n"
            "        \"cid\" : 644,\n"
            "        \"displaynumber\" : 1,\n"
            "        \"flowtype\" : 0,\n"
            "        \"height\" : 200,\n"
            "        \"mediatype\" : 1,\n"
            "        \"mid\" : 582,\n"
            "        \"pid\" : 72,\n"
            "        \"pubid\" : 0,\n"
            "        \"pv\" : 576,\n"
            "        \"supportbanner\" : null,\n"
            "        \"width\" : 200\n"
            "      }\n"
            "    } ]\n"
            "  }\n"
            "}";
    rapidjson::Document doc;
    parseJson(str,doc);
}

void fuck_test(){
    std::map<int,std::vector<int>> tMap;

}

int main(int argc,char** argv){
    json_test();
    return 0;
}