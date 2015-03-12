#!/usr/bin/env sh
# get.sh
# Martin Miller
# Created: 2015/03/11
# Grabs a frame and some IMU data

imupid=$(pgrep -u root -f getimu|tail -n1)
picpid=$(pgrep takePic)

echo $imupid
echo $picpid
sudo kill -10 $imupid
kill -10 $picpid

