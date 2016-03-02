#!/bin/bash

if [[ $# == 1 ]];
then
	case $1 in
	install)
		make all && \
		make export
		if [[ $? == 0 ]];
		then
			echo "build successfully!"
		else
			echo "build failed!"
		fi
		;;
	clean)
		make clean
		make clean_jni
		;;
	*)
	esac
fi
