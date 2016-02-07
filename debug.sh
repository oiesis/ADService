#!/bin/bash

echo "connection info:"
netstat -napt|grep "1922"
echo "process info:"
echo -e "  pid\tppid\tpgid\tsid"
ps xao pid,ppid,pgid,sid,command |grep "\\brun.sh"
ps xao pid,ppid,pgid,sid,command |grep "\\badservice"
