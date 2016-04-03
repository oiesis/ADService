GET /solutionsv2/solution,banner/_search
{
    "filter":{
        "filtered":{
            "query":{
                "has_parent": {
                         "type":"banner_group",
                         "filter":{
                             "has_child": {
                                 "type":"banner",
                                 "filter":{
                                        "bool":{
                                            "must":[
                                                {"term":{"width":200}},
                                                {"term":{"height":200}},
                                                {"has_parent": {
                                                    "type": "banner_group",
                                                    "query": {"has_child":{
                                                        "type": "solution",
                                                        "query":{
                                                            "match": {
                                                                "d_media":"youku"
                                                            }
                                                        }
                                                    }}
                                                 }}
                                            ]
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
                            "bool":{
                                "must":[
                                    {"term":{"_type":"solution"}},
                                    {"has_parent": {
                                        "type": "banner_group",
                                        "query": {"has_child":{
                                            "type": "solution",
                                            "query":{
                                                "match": {
                                                    "d_media":"youku"
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
                                     {"term": {"width":200}},
                                     {"term": {"height":200}}
                                    ]
                            }
                        }
                ]
                }
            }
        }
    }
}
