#! /bin/sh
# span.sh
# 1/27/2015
# Martin Miller
# Runs span.py and starts logging. Generates a logname based on the current time
# to prevent overwrites.

device=/dev/ttyUSB0
screenrc=/tmp/spanpy-screenrc

## check if device is connected
if [ ! -e $device ]
then
    echo "No device"
    exit 1
fi

## run span and check if it worked.
sudo python span.py $device
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
echo "logfile ${date}.log" | tee $screenrc
sudo screen -c $screenrc -L $device 115200
