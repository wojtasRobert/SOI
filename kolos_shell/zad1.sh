#!/bin/sh

i=1
while read line
do
echo $i $line
i=`expr $i + 1`
done <kupa.txt
