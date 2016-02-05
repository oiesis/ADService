#!/bin/bash

echo "connection info:"
netstat -napt|grep "1922"
echo "process info:"
ps xao pid,ppid,pgid,sid,command |grep "run.sh"
ps xao pid,ppid,pgid,sid,command |grep "adservice"
