#!/bin/sh

var=0

while [ true ]
do
    cat /proc/$1/status | grep Name
    echo "$1 fd:"
    ls /proc/$1/task
    cat /proc/$1/status | grep VmSize
    cat /proc/$1/status | grep VmRSS
	cat /proc/meminfo | grep MemFree
    sleep 10
    var=$((var+1))
done