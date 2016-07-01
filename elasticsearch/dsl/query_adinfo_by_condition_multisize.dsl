{
    "filter":{
        "filtered":{
            "filter":{
                "bool": {
                    "should": [
                        {
                            "bool":{
                                "must":[
                                    {"term":{"_type":"solution"}},
                                    {"terms": {"d_flowtype":["0","%d"]}},
                                    {"terms": {"d_hour":["0","%s"]}},
                                    {"terms": {"d_geo":["0","%d","%d"]}},
                                    {"terms": {"d_device":["0","%d"]}},
                                    {"terms": {"d_os":["0","%d"]}},
                                    {"term": {"solutionstatus":{"value":20}}},
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
                                                    {"term": {"bannerstatus":1}},
                                                    {"terms":{"bannertype":[%s]}},
                                                    {"bool": {"should":[%s]}}
                                                   ]
                                                }
                                            }
                                        }}
                                     }}
                                ],
                                "must_not":[
                                    {"term":{"n_geo":"%d"}}
                                ]
                            }
                        },
                        {
                            "bool":{
                                "must":[
                                     {"term": {"_type":"banner"}},
                                     {"term": {"bannerstatus":1}},
                                     {"terms":{"bannertype":[%s]}},
                                     {"bool": {"should":[%s]}},
                                     {
                                       "has_parent":{
                                           "type":"banner_group",
                                           "query":{
                                               "has_child":{
                                                   "type":"solution",
                                                   "query":{
                                                        "bool":{
                                                            "must":[
                                                                {"terms": {"d_flowtype":["0","%d"]}},
                                                                {"terms": {"d_hour":["0","%s"]}},
                                                                {"terms": {"d_geo":["0","%d","%d"]}},
                                                                {"terms": {"d_device":["0","%d"]}},
                                                                {"terms": {"d_os":["0","%d"]}},
                                                                {"term": {"solutionstatus":{"value":20}}},
                                                                {"range": {"starttime": {"lte": "now"}}},
                                                                {"range": {"endtime":{"gte":"now"}}}
                                                            ],
                                                            "must_not":[
                                                                {"term":{"n_geo":"%d"}}
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
