#!/bin/bash

if [[ $# < 1 ]];
then
        echo "usage: $0 [condition]"
        exit
fi

if [[ $1 != "stop" ]];
then
        for (( i=0;i<9;i++ ));
        do
                ./kafka_checker -C -t mt-log -b 192.168.2.52 -p $i -o -1 -v 2 -f $1 -q >result/result${i}.txt &
        done
else
        for pid in `ps -ef|grep "kafka_checker" |awk '{printf("%s\n",$2)}'`;
        do
                kill $pid
        done
fi

