#!/bin/bash

cd $1
for file in `dir -d *.f` ; do
	echo touch $file
	touch $file
done
for file in `dir -d *.f90` ; do
	echo touch $file
	touch $file
done
