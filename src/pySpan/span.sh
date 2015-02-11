#! /bin/sh
# span.sh
# 1/27/2015
# Martin Miller
# Runs span.py and starts logging. Generates a logname based on the current time
# to prevent overwrites.
# Usage: span.py [N]
# N: FPS (if not specified default: 5)
# parent: root directory for data storage
parent=/mnt/hdd/marty/sp

mkdir -p ${parent}/data
mkdir -p ${parent}/images

if [ $# -eq 1 ]
then
    FPS=$1
else
    FPS=5
fi
device=/dev/ttyUSB0
screenrc=/tmp/spanpy-screenrc

## check if device is connected
if [ ! -e $device ]
then
    echo "No device"
    exit 1
fi

## run span and check if it worked.
sudo python span.py -f $FPS $device
if [ ! $? -eq 0 ]
then
    echo "Span failed."
    echo "If it's 'resource temporarily unavailable, or something about 'readiness to read'"
    echo "Retrying in a few seconds usually helps."
    exit 1
fi
date=`date -I'minutes'`
if [ ! $? -eq 0 ]
then
    echo "`date` failed."
    exit 1
fi
# Create a temp screenrc to set the logfile name
echo "logfile ${parent}/data/${date}.gps" | tee $screenrc
echo "screen -L -t gps 0 $device 115200" | tee -a $screenrc
echo "logfile ${parent}/data/${date}.pics" | tee -a $screenrc
echo "screen -L -t cam 1 ../../bin/grabframe ${parent}" | tee -a $screenrc
sudo screen -c $screenrc
