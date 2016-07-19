{
    "filter":{
        "filtered":{
            "filter":{
                "bool": {
                    "should": [
                        {"bool": {"must": [{"term": {"_type": "adplace"}},{"term":{"pid":"%s"}}]}},
                        {
                            "bool":{
                                "must":[
                                    {"term":{"_type":"es_adplace"}},
                                    {"query":{"term": {
                                       "pid": {
                                          "value": "%s"
                                       }
                                    }
                                    }}
                                    ]
                            }
                        },
                        {
                            "bool":{
                                "must":[
                                    {"term":{"_type":"solution"}},
                                    {"term": {"solutionstatus":{"value":20}}},
                                    {"range": {"starttime": {"lte": "now-8h"}}},
                                    {"range": {"endtime":{"gte":"now-8h"}}},
                                    {"has_parent": {
                                        "type": "banner_group",
                                        "query": {"has_child":{
                                            "type": "banner",
                                            "query":{
                                                "bool": {
                                                   "must": [
                                                      {"term": {"_type":"banner"}},
                                                      {"term": {"width":%d}},
                                                      {"term": {"height":%d}},
                                                      {"term": {"bannerstatus":1}},
                                                      {"terms":{"bannertype":[%s]}}
                                                   ]
                                                }
                                            }
                                        }}
                                     }}
                                ]
                            }
                        },
                        {
                            "bool":{
                                "must":[
                                     {"term": {"_type":"banner"}},
                                     {"term": {"width":%d}},
                                     {"term": {"height":%d}},
                                     {"term": {"bannerstatus":1}},
                                     {"terms":{"bannertype":[%s]}},
                                     {
                                       "has_parent":{
                                           "type":"banner_group",
                                           "query":{
                                               "has_child":{
                                                   "type":"solution",
                                                   "query":{
                                                        "bool":{
                                                            "must":[
                                                                {"term": {"solutionstatus":{"value":20}}},
                                                                {"range": {"starttime": {"lte": "now-8h"}}},
                                                                {"range": {"endtime":{"gte":"now-8h"}}}
                                                            ]
                                                        }
                                                   }
                                               }
                                           }
                                       }
                                     }
                                    ]
                            }
                        }
                ]
                }
            }
        }
    }
}
