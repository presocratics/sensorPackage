#!/usr/bin/env sh
# span2slam.sh
# Martin Miller
# Created: 2016/06/07
# Converts SPAN output to SLAM output.
# Writes sensors to individual fifos
# Usage: pybin | tee data.gps | span2slam

awk -F, 'function euler2qbw(roll,pitch,yaw,q) {
             q[0] = cos(roll/2)*cos(pitch/2)*cos(yaw/2)+sin(roll/2)*sin(pitch/2)*sin(yaw/2)
             q[1] = cos(roll/2)*sin(pitch/2)*cos(yaw/2)+sin(roll/2)*cos(pitch/2)*sin(yaw/2)
             q[2] = cos(roll/2)*cos(pitch/2)*sin(yaw/2)-sin(roll/2)*sin(pitch/2)*cos(yaw/2)
             q[3] = sin(roll/2)*cos(pitch/2)*cos(yaw/2)-cos(roll/2)*sin(pitch/2)*sin(yaw/2)
         }
         

         BEGIN  { OFS="," }
         # Process RAWIMUS 
         /^40,325/ {printf("%d,%f,ACC,%0.9f,%0.9f,%0.9f\n", $5,$6,
         -200*200*2^-31*$9,
         200*200*2^-31*$10,
         -200*200*2^-31*$8) } 

         /^40,325/ {printf("%d,%f,ANG,%0.9f,%0.9f,%0.9f\n",  $5,$6,
         -200*720*2^-31*$12,
         200*720*2^-31*13,
         -200*720*2^-31*$11)}
         # Process RAWIMUS for calibration
         /^40,325/ {printf("%f,%0.9f,%0.9f,%0.9f,%0.9f,%0.9f,%0.9f\n", $6,
         -200*720*2^-31*$12,
         200*720*2^-31*13,
         -200*720*2^-31*$11,
         -200*200*2^-31*$9,
         200*200*2^-31*$10,
         -200*200*2^-31*$8) > "imu.csv"} 


         # Process INSPVAS
         /^88,508/ {printf("%d,%f,POS,%0.9f,%0.9f,%0.9f\n", $5,$6,$7,$8,$9)} 
         /^88,508/ {printf("%d,%f,VEL,%0.9f,%0.9f,%0.9f\n", $5,$6,$10,$11,-$12)}
         /^88,508/ {
            euler2qbw($13,$14,$15,q);
            printf("%d,%f,QUAT,%0.9f,%0.9f,%0.9f,%0.9f\n",
            $5,$6,q[0],q[1],q[2],q[3])
         }

         # Process CORRIMUDATAS
         /^60,813/ {printf("%d,%f,ACC,%0.9f,%0.9f,%0.9f\n", $5,$6,
         200*$11,200*$10,-200*$12)}
         /^60,813/ {printf("%d,%f,ANG,%0.9f,%0.9f,%0.9f\n", $5,$6,
         200*$8,200*$7,-200*$9)}

         # Process MARK2TIME
         /^616/ {printf("%d,%0.9f\n", $8,$9*1e-3) > "mark2time.txt" }'


