#!/bin/bash
#echo $1 $2

ln1=$(grep "!CALL start_region(.*,.*,$1,.*,.*)" $2)
ln1=$(echo $ln1 | sed -e 's/^[ \t]*!//')
sed -e "s/!CALL start_region(.*,.*,$1,.*,.*).*$/$ln1/g" $2 >tmp
cat tmp > $2 

ln2=$(grep "!CALL end_region(.*,.*,$1,.*,.*)" $2)
ln2=$(echo $ln2 | sed -e 's/^[ \t]*!//')
sed -e "s/!CALL end_region(.*,.*,$1,.*,.*).*$/$ln2/g" $2 >tmp
cat tmp > $2

echo region $1 instrumented in $2