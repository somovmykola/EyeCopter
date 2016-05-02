#takes multiple photos by calling rapistill in a loop

#!/bin/bash 

echo $1
for i in `seq 1 $1`; do
	echo "taking picture $i"
	raspistill -t 1 -o camera/image$i.jpg -w 1280 -h 720 
	#sleep 1s
done
