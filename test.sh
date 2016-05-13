#!/bin/bash

while true
do
	ab -c 500 -t 300 "http://192.168.31.156:1922/v?pid=2787&callback=jsonp_callback_605913&of=1"
	sleep 1
done
