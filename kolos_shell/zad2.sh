#!/bin/sh

if test $# -eq 1 
then
	i="$1"
	while IFS=: read name x id line
	do
		j="$id"
		if test $j = $i 
		then
			echo $name
		fi
	done</etc/passwd
fi

