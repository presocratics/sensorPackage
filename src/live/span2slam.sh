#!/usr/bin/env bash
# span2slam.sh
# Martin Miller
# Created: 2016/06/07
# Converts SPAN output to SLAM output.
# Writes sensors to individual fifos
# Usage: pybin | tee data.gps | span2slam

gawk -F, -v fname=$1 'function euler2qbw(roll,pitch,yaw,q,    rd, pd, yd) {
             rd = roll*0.017453292519943295
             pd = pitch*0.017453292519943295
             yd = yaw*0.017453292519943295
             q[0] = cos(rd/2)*cos(pd/2)*cos(yd/2)+sin(rd/2)*sin(pd/2)*sin(yd/2)
             q[1] = sin(rd/2)*cos(pd/2)*cos(yd/2)-cos(rd/2)*sin(pd/2)*sin(yd/2)
             q[2] = cos(rd/2)*sin(pd/2)*cos(yd/2)+sin(rd/2)*cos(pd/2)*sin(yd/2)
             q[3] = cos(rd/2)*cos(pd/2)*sin(yd/2)-sin(rd/2)*sin(pd/2)*cos(yd/2)
         }

         function gps2gpssec(wk,sec,        gpssec) {
            gpssec = 604800*wk+sec
            return gpssec
         }

         BEGIN  { OFS=","
                  fout=fname
         }
         # Process RAWIMUS 
         /^40,325/ {printf("%f,ACC,%0.9f,%0.9f,%0.9f\n", gps2gpssec($5,$6),
         -200*200*2^-31*$9,
         200*200*2^-31*$10,
         -200*200*2^-31*$8) > fout } 

         /^40,325/ {printf("%f,ANG,%0.9f,%0.9f,%0.9f\n",  gps2gpssec($5,$6),
         -200*720*2^-31*$12,
         200*720*2^-31*$13,
         -200*720*2^-31*$11) > fout}
         # Process RAWIMUS for calibration
         /^40,325/ {printf("%d,%0.9f,%0.9f,%0.9f,%0.9f,%0.9f,%0.9f\n", 
         int(1e9*gps2gpssec($5,$6)),
         -200*720*2^-31*$12,
         200*720*2^-31*$13,
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
            euler2qbw($14,$13,$15,q);
            printf("%f,QUAT,%0.9f,%0.9f,%0.9f,%0.9f\n",
            gps2gpssec($5,$6),q[0],q[1],q[2],q[3]) > fout
         }

         # Process BESTUTM
         /^726/ {printf("%f,UTM,%0.9f,%0.9f,%0.9f,%d,%c\n", gps2gpssec($8,$9*1e-3),
         $17,$18,-1*$19,$15,$16) > fout}

         # Process CORRIMUDATAS
         /^60,813/ {printf("%f,ACC,%0.9f,%0.9f,%0.9f\n", gps2gpssec($5,$6),
         200*$11,200*$10,-200*$12) > fout}
         /^60,813/ {printf("%f,ANG,%0.9f,%0.9f,%0.9f\n", gps2gpssec($5,$6),
         200*$8,200*$7,-200*$9) > fout}

         # Process inscovs
         /^228,320,/ {printf("%f,COVP,%0.9f,%0.9f,%0.9f,%0.9f,%0.9f,%0.9f,%0.9f,%0.9f,%0.9f\n",
             gps2gpssec($5,$6),$7,$8,$9,$10,$11,$12,$13,$14,$15) > fout }
         /^228,320,/ {printf("%f,COVA,%0.9f,%0.9f,%0.9f,%0.9f,%0.9f,%0.9f,%0.9f,%0.9f,%0.9f\n",
             gps2gpssec($5,$6),$16,$17,$18,$19,$20,$21,$22,$23,$24) > fout }
         /^228,320,/ {printf("%f,COVV,%0.9f,%0.9f,%0.9f,%0.9f,%0.9f,%0.9f,%0.9f,%0.9f,%0.9f\n",
             gps2gpssec($5,$6),$25,$26,$27,$28,$29,$30,$31,$32,$33) > fout }

         # Process MARK2TIME
         /^616/ {printf("%0.9f,IMG\n", gps2gpssec($13,$14)) > "imgtimes.txt";
         fflush() }
         '
