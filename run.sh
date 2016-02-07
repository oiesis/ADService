#!/bin/bash

cd build
if [[ $# == 1 && $1 == "stop" ]];
then
	mainPid=$(cat adservice.pid)
	kill ${mainPid}	
else
	./adservice 2>&1 1>service.log &
fi
