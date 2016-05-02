# backup of multipic, only useful for testing

#!/bin/bash 

echo $1
for i in `seq 1 $1`; do
	echo "taking picture $i"
	raspistill -t 1 -o camera/image480$i.jpg -w 640 -h 480
	#sleep 1s
done
