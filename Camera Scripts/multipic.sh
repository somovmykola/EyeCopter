#!/bin/bash 

echo $1
for i in `seq 1 $1`; do
	echo "taking picture $i"
	./camera.sh
	sleep 1s
done
