#!/bin/bash

#sed -e 's/sub user loop nestedloop call/reinstrumented/g' $1 >tmp	
sed -e 's/^[^#|$]\(\s*[0-9]*\)\(\s*[0-9A-Za-z/_.-]*\)\(.\s*\)\(.*$\)/\1\2\3 reinstrumented/g' $1 >tmp
cat tmp >$1
#sed -e 's/all/ reinstrumented/g' $1 >tmp
#cat tmp >$1

#echo $1 changed
#cat $1
