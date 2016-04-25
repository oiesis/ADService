#!/bin/bash


if [[ $# != 1 ]];
then
        echo "usage $0 [timespan]"
        exit
fi

t=$1

time_cachemiss=`cat service_log/service.log|grep "${t}"|grep "cache miss"|wc -l`
time_cachefree=`cat service_log/service.log|grep "${t}"|grep "free memory pool"|wc -l`
deadchild=`cat service_log/service.log|grep "${t}"|grep "child"|wc -l`
echo "cache miss:${time_cachemiss},cache free:${time_cachefree},dead child:${deadchild}"
