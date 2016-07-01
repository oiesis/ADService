{
    "query": {
        "bool": {
            "must": [
               {"term": {"pid":"%s"}},
               {"term": {"adxid":%d}}
            ]
        }
    }
}