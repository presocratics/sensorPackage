#!/usr/bin/env bash
# span2slam.sh
# Martin Miller
# Created: 2016/06/07
# Converts SPAN output to SLAM output.
# Writes sensors to individual fifos
# Usage: pybin | tee data.gps | span2slam
rm -f /tmp/mark2time.ff
mkfifo /tmp/mark2time.ff

syncTime /tmp/mark2time.ff <(tail -n+1 -f $1 | cut -d, -f2,3 ) | tee img.txt | awk -F, \
'{printf("%s,%d\n", $3, int(10^9*$1))}' > clib.txt &

awk -F, 'function euler2qbw(roll,pitch,yaw,q,    rd, pd, yd) {
             rd = roll*0.017453292519943295
             pd = pitch*0.017453292519943295
             yd = yaw*0.017453292519943295
             q[0] = cos(rd/2)*cos(pd/2)*cos(yd/2)+sin(rd/2)*sin(pd/2)*sin(yd/2)
             q[1] = cos(rd/2)*sin(pd/2)*cos(yd/2)+sin(rd/2)*cos(pd/2)*sin(yd/2)
             q[2] = cos(rd/2)*cos(pd/2)*sin(yd/2)-sin(rd/2)*sin(pd/2)*cos(yd/2)
             q[3] = sin(rd/2)*cos(pd/2)*cos(yd/2)-cos(rd/2)*sin(pd/2)*sin(yd/2)
         }

         function gps2gpssec(wk,sec,        gpssec) {
            gpssec = 604800*wk+sec
            return gpssec
         }

         BEGIN  { OFS=","
                  fout=strftime("%d%m%Y-%T",systime()) ".gps"
         }
         # Process RAWIMUS 
         /^40,325/ {printf("%f,ACC,%0.9f,%0.9f,%0.9f\n", gps2gpssec($5,$6),
         -200*200*2^-31*$9,
         200*200*2^-31*$10,
         -200*200*2^-31*$8) > fout } 

         /^40,325/ {printf("%f,ANG,%0.9f,%0.9f,%0.9f\n",  gps2gpssec($5,$6),
         -200*720*2^-31*$12,
         200*720*2^-31*13,
         -200*720*2^-31*$11) > fout}
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
         /^88,508/ {printf("%f,POS,%0.9f,%0.9f,%0.9f\n",
         gps2gpssec($5,$6),$7,$8,$9) > fout} 
         /^88,508/ {printf("%f,VEL,%0.9f,%0.9f,%0.9f\n",
         gps2gpssec($5,$6),$10,$11,-$12) > fout}
         /^88,508/ {
            euler2qbw($13,$14,$15,q);
            printf("%f,QUAT,%0.9f,%0.9f,%0.9f,%0.9f\n",
            gps2gpssec($5,$6),q[0],q[1],q[2],q[3]) > fout
         }

         # Process CORRIMUDATAS
         /^60,813/ {printf("%f,ACC,%0.9f,%0.9f,%0.9f\n", gps2gpssec($5,$6),
         200*$11,200*$10,-200*$12) > fout}
         /^60,813/ {printf("%f,ANG,%0.9f,%0.9f,%0.9f\n", gps2gpssec($5,$6),
         200*$8,200*$7,-200*$9) > fout}

         # Process MARK2TIME
         /^616/ {print gps2gpssec($8,$9*1e-3) > "/tmp/mark2time.ff" }' 

rm -f /tmp/mark2time.ff
