#!/bin/bash

verbose=0

if [ $# -eq 1 ];
then
	if [ $1 = 'clean' ];
	then
		rm -rf */*.pb.*
		exit 0
	elif [ $1 = '--verbose' ];
	then
		verbose=1	
	fi
fi

for src_dir in `find * -maxdepth 0 -type d`
do
	dst_dir="$src_dir"
	error_msg=$(protoc -I="$src_dir"/proto/ --cpp_out="$dst_dir" "$src_dir"/proto/*.proto 2>&1)
	if [[ $? != 0 && ${verbose} == 1 ]];
	then
		echo "error occured processing ${src_dir}:${error_msg}"
	fi
done
