#!/bin/bash

# Periscope registry and agents clean-up
#	Author:			Ventsislav Petkov
#   Revision:       $Revision$
#	Revision date:  $Date$
#	Committed by:   $Author$
#
#	This file is part of the Periscope performance measurement tool.
#	See http://www.lrr.in.tum.de/periscope for details.
#
#	Copyright (c) 2005-2011, Technische Universitaet Muenchen, Germany
#	See the COPYING file in the base directory of the package for details.

SUCCESS=0

echo "Killing all agents..."
killall -u $USER hlagent.ia64 analysisagent.ia64 
echo "Done!"

# Get the data from .periscope
eval $(grep REGSERVICE_HOST $HOME/.periscope | tr -d ' ')
eval `grep REGSERVICE_PORT $HOME/.periscope | tr -d ' '`

echo "Registry at $REGSERVICE_HOST:$REGSERVICE_PORT"

# Connect to the applications and stop them
STOPCMD="terminate;\n"
# get the registered app
APPS=( $(echo -e "list\nquit" | netcat -v $REGSERVICE_HOST $REGSERVICE_PORT|awk '/port/ { print $3 }') )
# get the hosts
HOSTS=( $(echo -e "list\nquit" | netcat -v $REGSERVICE_HOST $REGSERVICE_PORT|awk '/port/ { print $6 }') )
# get ports
PORTS=( $(echo -e "list\nquit" | netcat -v $REGSERVICE_HOST $REGSERVICE_PORT|awk '/port/ { print $7 }') )
for (( i = 0 ; i < ${#APPS[@]} ; i++ ))
do
	# $app - application name
	eval ${APPS[$i]}
	# $node - host where the app runs
	eval ${HOSTS[$i]}
	# $port - port number
	eval ${PORTS[$i]}
	
	echo "Sending Terminate command to \"$app\" application..."
	echo -e $STOPCMD | netcat $node $port
	
done


echo "Cleaning the registry at $REGSERVICE_HOST:$REGSERVICE_PORT..."
echo -e "list\nclean\nquit" | netcat -v $REGSERVICE_HOST $REGSERVICE_PORT
if [ "$?" -eq $SUCCESS ]
then
  echo "Done!"
  exit $SUCCESS
else
  echo "Error cleaning the registry"
fi

