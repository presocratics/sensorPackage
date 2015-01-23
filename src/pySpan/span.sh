#! /bin/sh
device=/dev/ttyUSB0
if [ ! -e $device ]
then
    echo "No device"
    exit 1
fi
sudo python span.py $device
if [ ! $? -eq 0 ]
do
    echo "Span failed."
    echo "If it's 'resource temporarily unavailable, or something about 'readiness to read'"
    echo "Retrying in a few seconds usually helps."
    exit 1
done
sudo screen -L $device 115200
