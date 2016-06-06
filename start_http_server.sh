#!/bin/bash

while true
do
	./adservice >>service_log/service.log 2>&1
done
