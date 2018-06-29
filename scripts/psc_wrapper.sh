
# Periscope script that checks if the registry server is running and if not it starts it
#   Revision:       $Revision$
#	Revision date:  $Date$
#	Committed by:   $Author$
#
#	This file is part of the Periscope performance measurement tool.
#	See http://www.lrr.in.tum.de/periscope for details.
#
#	Copyright (c) 2005-2011, Technische Universitaet Muenchen, Germany
#	See the COPYING file in the base directory of the package for details.

#usage
if [ $# != 2 ]
then echo "usage: psc_wrapper [cmd] [registry_port]"
     echo "cmd command given as string"
     echo "registry_port given as integer"
     echo "example: psc_wrapper '/path/frontend --apprun=\"string value\"' 25696"
     exit
fi

# Get the data from .periscope
eval $(grep REGSERVICE_HOST $HOME/.periscope | tr -d ' ')
export REGSERVICE_PORT=$2
export PSC_REGISTRY=$REGSERVICE_HOST:$REGSERVICE_PORT

#eval `grep REGSERVICE_PORT $HOME/.periscope | tr -d ' '`

#check if regsrv is started
out=( $(echo -e "list\nquit" |netcat $REGSERVICE_HOST $REGSERVICE_PORT | awk '/PERISCOPE/ { print $3 }'| head -1 ) )

 if [ $out ] && [ 'PERISCOPE' = $out ] 
   then echo "[DEBUG] regsrv already started"
   else regsrv.ia64 $REGSERVICE_PORT &
        export PSC_REGISTRY=$HOSTNAME:$REGSERVICE_PORT
	export REGSERVICE_HOST=$HOSTNAME
 fi

timeout=300
i=1

#wait for start up
while [ $i -le $timeout ]
do
 if [ $out ] && [ 'PERISCOPE' = $out ]
    then break
 fi
 echo "[DEBUG] waiting for regsrv.ia64 ... "
  sleep 1
  out=( $(echo -e "list\nquit" |netcat $REGSERVICE_HOST $REGSERVICE_PORT | awk '/PERISCOPE/ { print $3 }'| head -1 ) )
  i=`expr $i + 1`
done

i=`expr $i - 1`	
if [ $i -eq $timeout ]
 then
   echo "[ERROR] no regsrv.ia64 found !!! "
  exit
fi
echo "[OK] found regsrv.ia64" 
eval $1" --registry="$PSC_REGISTRY
