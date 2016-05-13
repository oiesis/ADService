#!/bin/bash

function killPortOwner(){
        pid=`netstat -napt|grep "$1"|grep "LISTEN" |awk -F' ' '{print $7}'|awk -F'/' '{print $1}'`
        kill $pid
}


LOGROTATE_CONFIG=$(pwd)/service_log/service.log
LOGROTATE_CONFIG+=$(cat <<'HEREDOC'
{
        daily
        rotate 7
        create 644 root root
        dateext
        copytruncate
}
HEREDOC
)

mkdir -p service_log

if [[ $# == 1 ]];
then
	if [[ $1 == "stop" ]];
	then
		mainPid=$(cat adservice.pid)
		kill ${mainPid}
		#killPortOwner 1922
	elif [[ $1 == "log" ]];
	then
		more service_log/service.log
	fi
else
   	 echo "$LOGROTATE_CONFIG" > ./conf/logrotate.conf
   	 export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/usr/local/lib64/:$(pwd)/3rdparty/lib/
	./adservice >./service_log/service.log 2>&1 &
	#if [[ $? == 0 ]];
	#then
    		#sudo logrotate -s ./service_log/adservice.log ./conf/logrotate.conf
    		#sudo cp ./conf/logrotate.conf /etc/logrotate.d/adservice
    #	fi
fi
