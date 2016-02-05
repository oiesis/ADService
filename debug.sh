#!/bin/bash

echo "connection info:"
netstat -napt|grep "1922"
echo "process info:"
ps -ef|grep "adservice"
