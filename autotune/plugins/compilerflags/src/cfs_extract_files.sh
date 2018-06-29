#!/bin/bash

# Takes as argument a functions profile file and extract the files with functions using CFS_MAX_FUNC_AGGREGATE time of the total execution time.
# Houssam Haitof 08.2013

CFS_MIN_FUNC_PERCENTAGE=${CFS_MIN_FUNC_PERCENTAGE:-10}
CFS_MAX_FUNC_AGGREGATE=${CFS_MAX_FUNC_AGGREGATE:-95}

let flag=1
let total=0
rm  "files2touch"
touch files2touch
while IFS=$'\t' read -r -a funcArray
do
 if [ $flag -eq 1 ] 
  then
   let flag=0
   continue
 fi 
 total=$(echo "$total+${funcArray[3]}" | bc) 
 if awk "BEGIN {exit (${funcArray[3]} >= $CFS_MIN_FUNC_PERCENTAGE && $total < $CFS_MAX_FUNC_AGGREGATE) ? 0 : 1}" 
  then
   
    echo ${funcArray[8]} | cut -d':' -f 1 >> "files2touch"
 fi
done 
