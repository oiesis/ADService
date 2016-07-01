{
    "query": {
        "bool": {
            "must": [
               {"term": {"adxpid":"%s"}},
               {"term": {"adxid":%d}}
            ]
        }
    }
}