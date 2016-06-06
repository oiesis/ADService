#!/bin/bash

tcpdump -X -s 0 'tcp port 20009 and (((ip[2:2] - ((ip[0]&0xf)<<2)) - ((tcp[12]&0xf0)>>2)) != 0)' -c 10
