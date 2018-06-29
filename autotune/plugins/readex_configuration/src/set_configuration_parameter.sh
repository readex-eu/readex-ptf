#echo REPLACE in $1 $2 by $3
#cat $1
#echo
sed -i -e "s/$2/$3/g" $1
#cat $1
