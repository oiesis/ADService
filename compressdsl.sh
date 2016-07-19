#!/bin/bash

src_folder=elasticsearch/dslori
target_folder=elasticsearch/dsl
cwd=`pwd`
target_folder=${cwd}/${target_folder}
cd $src_folder
for name in `ls`;
do 
	content=`cat $name` && echo $content |tr -d "\t\r\n " > ${target_folder}/${name}
done
