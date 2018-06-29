#!/bin/bash

ln=$(grep "$1.*reinstrumented" $2)

if [ -z "$ln" ];then

        sed -e "s/$1/$1    reinstrumented/g" $2 >tmp
        cat tmp>$2
        echo $tmp
fi
