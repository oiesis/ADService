#!/bin/bash

cd build
if [[ $# == 2 && $2 == "back" ]];
then
	./adservice 2>&1 1>service.log &
else
	./adservice 2>&1 1>service.log
fi
