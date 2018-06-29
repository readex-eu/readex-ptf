#!/bin/bash



cd $1

cat *.{f,f90,c,cc,cpp,h,hpp} | sha1sum | cut -d ' ' -f 1
