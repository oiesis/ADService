GET /solutions/banner,solution/_search?pretty&filter_path=took,hits.hits._type,hits.hits._source
{
    "filter":{
        "filtered":{
            "query":{
                "has_parent": {
                         "type":"banner_group",
                         "filter":{
                             "has_child": {
                                 "type":"solution",
                                 "filter":{
                                     "has_child":{
                                        "type":"media_adplace",
                                        "filter":{
                                            "term":{
                                                "pid":%s
                                            }
                                        }
                                     }
                                 }
                             }
                         }
                }
            },
            "filter":{
                "bool": {
                    "should": [
                        {
                            "has_child":{
                                "type":"media_adplace",
                                "filter":{
                                    "term":{
                                        "pid":%s
                                    }
                                }
                            }

                        },
                        {
                            "bool":{
                                "must":[
                                    {"term": {"_type":"banner"}},
                                    {"range": {
                                       "width": {
                                          "from": 0,
                                          "to": 500
                                       }
                                    }}
                                    ]
                            }
                        }
                ]
                }
            }
        }
    }
}