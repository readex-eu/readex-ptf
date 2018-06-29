#!/bin/bash
#
# NAME
#     Beautify
#
# DESCRIPTION
#     Beautify all source files. The script will automatically find the git root.
#
# OPTIONS/ARGUMENTS
#     beautify does not take any options and/or arguments.
#
# USAGE
#     Call beautify from anywhere in your repository.
#
# AUTHOR
#     Robert Mijakovic <mijakovi@in.tum.de>
#     Michael Firbach <firbach@in.tum.de>
#
set -e

AWK=awk
BEAUTIFY=uncrustify
BEAUTIFY_CONFIG=beautify.cfg
FIND=find
ROOT=`git rev-parse --show-toplevel`

RELEVANT_FILES=`$FIND $ROOT ! -path "$ROOT/examples/*" -type f -regex ".*\.\(c\|h\|cc\|cpp\|hpp\)"`
for FILE in $RELEVANT_FILES
do
    $BEAUTIFY -c $BEAUTIFY_CONFIG --no-backup $FILE
    if [ $? -ne 0 ]; then
        printf "Beautifying aborted: %s (%s) failed on file \"%s\".\n" \
               "$($BEAUTIFY --version)" \
               "$(command -v $BEAUTIFY)" \
               "$FILE"
        exit 1
    fi
done
