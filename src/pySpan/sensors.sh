#! /bin/sh
# span.sh
# 1/27/2015
# Martin Miller
# Runs span.py and starts logging. Generates a logname based on the current time
# to prevent overwrites.
# Usage: span.py [N] [N parent]
# N: FPS (if not specified default: 5)
# parent: root directory for data storage

case "$#" in
    0)  FPS=5
        parent="./"
        sia=""
        ;;
    1)  FPS=$1
        parent="./"
        sia=""
        ;;
    2)  FPS=$1
        parent=$2
        sia=""
        ;;
    3)  FPS=$1
        parent=$2
        sia="-s"
esac

mkdir -p ${parent}/data
mkdir -p ${parent}/images

device=/dev/ttyUSB0
screenrc=/tmp/spanpy-screenrc

## check if device is connected
if [ ! -e $device ]
then
    echo "No device"
    exit 1
fi

## run span and check if it worked.
sudo ./bin/span -b -f $FPS $sia -d $device
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
# Leading tabs (not spaces) ignored.
tee $screenrc <<- EOF
	logfile ${parent}/data/${date}.gps
    screen -L -t gps 0 ./bin/pybin
	logfile ${parent}/data/${date}.pics
	screen -L -t cam 1 ./bin/grabframe ${parent}
	split
	focus
	next
	focus
EOF
sudo screen -c $screenrc
