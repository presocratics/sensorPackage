#!/usr/bin/env sh
# record_Data.sh
# Hong-Bin Yoon
# Created: 2014/10/27

./bin/grabframe /dev/ttyUSB1 > data/framedata.txt &
./bin/spanread /dev/ttyUSB0 -gps data/gps.txt -imu data/imu.txt -att data/att.txt &
