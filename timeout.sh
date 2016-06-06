#!/bin/bash

lastpid=0

function killPortOwner(){
        pid=`netstat -napt|grep "$1"|grep "LISTEN" |awk -F' ' '{print $7}'|awk -F'/' '{print $1}'`
        if (( $lastpid != $pid ));
	then
		lastpid=$pid
		kill $pid
	else
		kill -9 $pid
	fi
}

while [ 1 ]
do
        CLOSEWAIT=`netstat -napt|grep "1922"|grep "CLOSE_WAIT"|wc -l`
        if (( $CLOSEWAIT > 20000 ));
        then
                triggerTime=`date`
                echo "${triggerTime}:${CLOSEWAIT} close_wait, dangerous condition detected,trigger hedge"
                killPortOwner 1922
        fi
        sleep 10
done


