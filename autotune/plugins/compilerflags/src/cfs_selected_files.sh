#!/bin/bash

#echo "The script you are running has basename `basename $0`, dirname `dirname $0`"
#echo "The present working directory is `pwd`"


if ls loop_prof_funcs_* > /dev/null 2>&1
then
    `dirname $0`/cfs_extract_files.sh < loop_prof_funcs_*
else
    echo "ERROR: Intel ifort loop profiler result missing (loop_prof_funcs_*)."
fi

