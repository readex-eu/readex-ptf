#!/bin/bash
files=( $(cat "files2touch") ) 
cd $1
for element in $(seq 0 $((${#files[@]} - 1)))
  do               
   touch "${files[$element]}"
  done
exit 0
