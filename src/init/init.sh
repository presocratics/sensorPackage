#!/usr/bin/env sh
# init.sh
# Martin Miller
# Created: 2016/05/06

USB0=/dev/ttyUSB0
USB1=/dev/ttyUSB1

# Turn stuff off
echo unlogall > $USB1
echo eventoutcontrol mark1 disable > $USB1

sleep 1

echo log eventincontrol mark2 enable > $USB1
echo log usb1 mark2time onnew > $USB1
echo eventoutcontrol mark1 enable positive 999000000 999000000 > $USB1

sleep 1

echo eventoutcontrol mark1 disable > $USB1

camtime=$(grabframe -1 | head -1 | awk -F, '{printf("%0.5f\n", $3*10^-7)}')
gpstime=$(<$USB0 head -n1| cut -d' ' -f6,7 | awk '{printf("%0.3f\n", 315964800 - 17 + 604800*$1 + $2)}')
echo $gpstime - $camtime | bc

