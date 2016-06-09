#!/usr/bin/env bash
# span2slam.sh
# Martin Miller
# Created: 2016/06/07
# Converts SPAN output to SLAM output.
# Writes sensors to individual fifos
# Usage: pybin | tee data.gps | span2slam
rm -f /tmp/mark2time.ff
mkfifo /tmp/mark2time.ff

syncTime /tmp/mark2time.ff <(cut -d, -f2,3 $1) | tee img.txt | awk -F, \
'{printf("%d,%d\n", $3, int(10^9*$1))}' > clib.txt &

awk -F, 'function euler2qbw(roll,pitch,yaw,q) {
             q[0] = cos(roll/2)*cos(pitch/2)*cos(yaw/2)+sin(roll/2)*sin(pitch/2)*sin(yaw/2)
             q[1] = cos(roll/2)*sin(pitch/2)*cos(yaw/2)+sin(roll/2)*cos(pitch/2)*sin(yaw/2)
             q[2] = cos(roll/2)*cos(pitch/2)*sin(yaw/2)-sin(roll/2)*sin(pitch/2)*cos(yaw/2)
             q[3] = sin(roll/2)*cos(pitch/2)*cos(yaw/2)-cos(roll/2)*sin(pitch/2)*sin(yaw/2)
         }

         function gps2gpssec(wk,sec,        gpssec) {
            gpssec = 604800*wk+sec
            return gpssec
         }

         BEGIN  { OFS="," }
         # Process RAWIMUS 
         /^40,325/ {printf("%f,ACC,%0.9f,%0.9f,%0.9f\n", gps2gpssec($5,$6),
         -200*200*2^-31*$9,
         200*200*2^-31*$10,
         -200*200*2^-31*$8) } 

         /^40,325/ {printf("%f,ANG,%0.9f,%0.9f,%0.9f\n",  gps2gpssec($5,$6),
         -200*720*2^-31*$12,
         200*720*2^-31*13,
         -200*720*2^-31*$11)}
         # Process RAWIMUS for calibration
         /^40,325/ {printf("%d,%0.9f,%0.9f,%0.9f,%0.9f,%0.9f,%0.9f\n", 
         int(1e9*gps2gpssec($5,$6)),
         -200*720*2^-31*$12,
         200*720*2^-31*13,
         -200*720*2^-31*$11,
         -200*200*2^-31*$9,
         200*200*2^-31*$10,
         -200*200*2^-31*$8) > "imu.csv"} 


         # Process INSPVAS
         /^88,508/ {printf("%f,POS,%0.9f,%0.9f,%0.9f\n", gps2gpssec($5,$6),$7,$8,$9)} 
         /^88,508/ {printf("%f,VEL,%0.9f,%0.9f,%0.9f\n", gps2gpssec($5,$6),$10,$11,-$12)}
         /^88,508/ {
            euler2qbw($13,$14,$15,q);
            printf("%f,QUAT,%0.9f,%0.9f,%0.9f,%0.9f\n",
            gps2gpssec($5,$6),q[0],q[1],q[2],q[3])
         }

         # Process CORRIMUDATAS
         /^60,813/ {printf("%f,ACC,%0.9f,%0.9f,%0.9f\n", gps2gpssec($5,$6),
         200*$11,200*$10,-200*$12)}
         /^60,813/ {printf("%f,ANG,%0.9f,%0.9f,%0.9f\n", gps2gpssec($5,$6),
         200*$8,200*$7,-200*$9)}

         # Process MARK2TIME
         /^616/ {print gps2gpssec($8,$9*1e-3) > "/tmp/mark2time.ff" }' 

rm -f /tmp/mark2time.ff
