#!/bin/sh
x=0
echo $1
echo $@
while test $x -lt 100
do
	for i in $@
	do
		$i		
	done
	x=`expr $x + 1`
done 
echo $x
