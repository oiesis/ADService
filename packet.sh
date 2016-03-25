#!/bin/bash

tcpdump dst port 1922 and 'tcp[((tcp[12:1] & 0xf0) >> 2):4] = 0x47455420' -X -c 2
