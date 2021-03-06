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
                                    {"terms": {"d_adexchange":["0","%d"]}},
                                    {"terms": {"d_mediatype": ["0","%d"]}},
                                    {"terms": {"d_adplace":["0","%s"]}},
                                    {"terms": {"d_adplacetype":["0","%d"]}},
                                    {"terms": {"d_displaynumber":["0","%d"]}},
                                    {"terms": {"d_flowtype":["0","%d"]}},
                                    {"terms": {"d_hour":["0","%s"]}},
                                    {"terms": {"d_geo":["0","%d","%d"]}},
                                    {"terms": {"d_device":["0","%d"]}},
                                    {"terms": {"d_os":["0","%d"]}},
                                    {"term":  {"d_dealid":"%s"}},
                                    {"term": {"solutionstatus":{"value":11}}},
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
                                ],
                                "must_not":[
                                    {"term":{"n_adplace":"%s"}},
                                    {"term":{"n_geo":"%d"}}
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
                                                                {"terms": {"d_adexchange":["0","%d"]}},
                                                                {"terms": {"d_mediatype": ["0","%d"]}},
                                                                {"terms": {"d_adplace":["0","%s"]}},
                                                                {"terms": {"d_adplacetype":["0","%d"]}},
                                                                {"terms": {"d_displaynumber":["0","%d"]}},
                                                                {"terms": {"d_flowtype":["0","%d"]}},
                                                                {"terms": {"d_hour":["0","%s"]}},
                                                                {"terms": {"d_geo":["0","%d","%d"]}},
                                                                {"terms": {"d_device":["0","%d"]}},
                                                                {"terms": {"d_os":["0","%d"]}},
                                                                {"term":  {"d_dealid":"%s"}},
                                                                {"term": {"solutionstatus":{"value":11}}},
                                                                {"range": {"starttime": {"lte": "now-8h"}}},
                                                                {"range": {"endtime":{"gte":"now-8h"}}}
                                                            ],
                                                            "must_not":[
                                                                {"term":{"n_adplace":"%s"}},
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
