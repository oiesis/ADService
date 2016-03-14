#!/bin/bash

for i in `seq 1 100000`
do
        if (( $i % 200 == 0 ));
        then
                sleep 1
        fi
        curl "http://127.0.0.1:1922/test?s=144105&x=99&r=0490337e-d6ce-11e5-8300-00163e08015d&d=12&t=144105&e=38&c=273&f=http%3A%2F%2Fwww.mtty.com%2Fdemo%2Fpcdd.html&h=000&a=0086-0010-0010&url=http%3A%2F%2Fm.rong360.com%2Fexpress%3Ffrom%3Dsem22%26utm_source%3Dmtwj%26utm_medium%3Dmtwj_dk" &
done