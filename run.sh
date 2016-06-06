#!/bin/bash

function killPortOwner(){
        pid=`netstat -napt|grep "$1"|grep "LISTEN" |awk -F' ' '{print $7}'|awk -F'/' '{print $1}'`
        kill -9 $pid
}

function killscript(){
	for pid in `ps -ef|grep "$1" |awk '{printf("%s\n",$2)}'`;
        do
               kill $pid 2>/dev/null
        done
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
		if [ -e adservice.pid ];
		then
			mainPid=$(cat adservice.pid)
			kill ${mainPid} 2>/dev/null
		fi
		killPortOwner 1922
	elif [[ $1 == "stophttp" ]];
	then
		killscript start_http_server.sh
		killPortOwner 1922
	elif [[ $1 == "http" ]];
	then
		echo "$LOGROTATE_CONFIG" > ./conf/logrotate.conf
	        export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/usr/local/lib64/:$(pwd)/3rdparty/lib/
		./start_http_server.sh >service_log/run.log 2>&1 &
		#sudo logrotate -s ./service_log/adservice.log ./conf/logrotate.conf
                #sudo cp ./conf/logrotate.conf /etc/logrotate.d/adservice
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
