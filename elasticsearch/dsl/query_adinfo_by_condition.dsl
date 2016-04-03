{
    "filter":{
        "filtered":{
            "filter":{
                "bool": {
                    "should": [
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
                                    {"terms": {"d_mediatype": ["0","%d"]}},
                                    {"terms": {"d_adplace":["0","%s"]}},
                                    {"terms": {"d_adplacetype":["0","%d"]}},
                                    {"terms": {"d_displaynumber":["0","%d"]}},
                                    {"terms": {"d_flowtype":["0","%d"]}},
                                    {"range": {"starttime": {"lte": "now"}}},
                                    {"range": {"endtime":{"gte":"now"}}},
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
                                                      {"terms":{"banner_type":[%s]}}
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
                                     {"terms":{"banner_type":[%s]}},
                                     {
                                       "has_parent":{
                                           "type":"banner_group",
                                           "query":{
                                               "has_child":{
                                                   "type":"solution",
                                                   "query":{
                                                        "bool":{
                                                            "must":[
                                                                {"terms": {"d_mediatype": ["0","%d"]}},
                                                                {"terms": {"d_adplace":["0","%s"]}},
                                                                {"terms": {"d_adplacetype":["0","%d"]}},
                                                                {"terms": {"d_displaynumber":["0","%d"]}},
                                                                {"terms": {"d_flowtype":["0","%d"]}},
                                                                {"range": {"starttime": {"lte": "now"}}},
                                                                {"range": {"endtime":{"gte":"now"}}}
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
