#!/bin/bash

cd build
if [[ $# == 1 ]];
then
	if [[ $1 == "stop" ]];
	then
		mainPid=$(cat adservice.pid)
		kill ${mainPid}	
	elif [[ $1 == "log" ]];
	then
		more service.log
	fi
else
	./adservice 2>&1 1>service.log &
fi
