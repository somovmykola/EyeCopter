#bash script takes a photo with the given parameters

#!/bin/bash

DATE=$(date +"%Y-%m-%d_%H%M%S%N")

raspistill -vf -ex sports -hf -t 100 -o /home/pi/camera/$DATE.jpg -w 1280 -h 720

echo $DATE.jpg >> /home/pi/camera/PNames.txt
