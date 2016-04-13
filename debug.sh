#!/bin/bash

﻿echo "connection info:"
#netstat -napt|grep "1922"
CONN=`netstat -napt|grep "1922"|grep "ESTABLISHED"|wc -l`
echo "connected:${CONN}"
CLOSE=`netstat -napt|grep "1922"|grep "WAIT"|wc -l`
echo "to close or closed:${CLOSE}"
echo "process info:"
echo -e "  pid\tppid\tpgid\tsid"
ps xao pid,ppid,pgid,sid,command |grep "\\brun.sh"
ps xao pid,ppid,pgid,sid,command |grep "\\badservice"

