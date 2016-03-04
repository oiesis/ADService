#!/bin/bash

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

cd build
if [[ $# == 1 ]];
then
	if [[ $1 == "stop" ]];
	then
		mainPid=$(cat adservice.pid)
		kill ${mainPid}	
	elif [[ $1 == "log" ]];
	then
		more ../service_log/service.log
	fi
else
    echo "$LOGROTATE_CONFIG" > ../conf/logrotate.conf
	./adservice 2>&1 1>../service_log/service.log &
	if [[ $? == 0 ]];
	then
    	logrotate -s ../service_log/adservice.log ../conf/logrotate.conf
    fi
fi
