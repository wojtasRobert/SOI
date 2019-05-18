#!/bin/sh

result=""

while test $# -gt 0
do
	result="$1 $result"
	shift
done	
echo $result
