#!/bin/bash

curdir=$(pwd)
JVM_OPTIONS="-DMTTY_LIBPATH=${curdir}"

java ${JVM_OPTIONS} com.mtty.cypher.AliBinaryEscaper
