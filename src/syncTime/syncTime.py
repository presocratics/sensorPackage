#!/usr/bin/env python2.7
# syncTime.py
# Martin Miller
# Created: 2016/05/12
"""Synchronizes live feed of gps and camera timestamps
Usage: syncTime <gps times> <cam times(UTC)>"""
import argparse

leaps = [46828800, 78364801, 109900802, 173059203, 252028804, 315187205,
      346723206, 393984007, 425520008, 457056009, 504489610, 551750411,
      599184012, 820108813, 914803214, 1025136015, 1119744016, 1341118800]

def isGlitch(cur,prev,mindel=0.01):
    """Returns True if it is a glitch, False if not"""
    if cur-prev<mindel:
        return True
    return False

def leapSeconds(gpstime):
    """Returns the number of leapSeconds"""
    nleaps = len(leaps)
    for ls in leaps.__reversed__():
        if gpstime>=ls:
            break
        else:
            nleaps-=1
    return nleaps

def gpsToUTC(wk,sec):
    """Converts gps time to UTC"""
    utcgpsoffset = 315964800. # 1980/01/06 00:00:00
    gpstime = 604800.*wk + sec
    nleaps = leapSeconds(gpstime)

    utc = utcgpsoffset + int(gpstime) - nleaps
    """Linear interpolate when we approach a leap second as seen here:
        https://losc.ligo.org/s/js/gpstimeutil.js"""
    if 1+int(gpstime) in leaps:
        utc += gpstime%1/2
    elif int(gpstime) in leaps:
        utc += (1+gpstime%1)/2
    else:
        utc += gpstime%1

    return utc

def getValidGPS(gps,prevgps):
    """Gets the next valid gps time from File gps and returns the time in UTC"""
    while True:
        try:
            line=gps.readline()
            line=line.strip()
            gpssec=float(line)
        except ValueError:
            return -1
        if prevgps is None or not isGlitch(gpssec,prevgps):
            return gpssec

def getNextCam(cam,coff):
    try:
        line=cam.readline()
        line=line.strip()
        fno,camtime = line.split(',')
    except ValueError:
        return -1,-1,-1
    fno = int(fno)
    camtime = int(camtime)
    return fno,10**-7*camtime+coff

def calcCamTimeOffset(camtime,utc):
    """Returns the offset in seconds between camtime and utc"""
    return utc-camtime

def closeAndExit(fn1,fn2):
    fn1.close()
    fn2.close()
    exit(0)

def main():
    parser = argparse.ArgumentParser(description="Synchronize gps and camera timestamps")
    parser.add_argument('-o', type=float, default=0, help='Set initial offset between gps and camera')
    parser.add_argument('gps', help='File containing gps times in SECONDS format')
    parser.add_argument('cam', help="""File containing cam times in
                        FRAMENO,CAMTIME[ns] format""")
    args=parser.parse_args()

    gps = open(args.gps, 'r')
    cam = open(args.cam, 'r')

    prevfno=0
    prevgps=None
    camgpsoffset=None
    coff=0
    iter=0
    while True:
        fno,ctime= getNextCam(cam,coff)
        gpssec = getValidGPS(gps,prevgps)
        if gpssec==-1 or fno==-1: # end of file reached
            print(gpssec,fno)
            closeAndExit(gps,cam)
        if coff==0:
            coff+=calcCamTimeOffset(ctime,gpssec)
        diff=gpssec-ctime
        while diff<-0.03:
            gpssec = getValidGPS(gps,prevgps)
            diff=gpssec-ctime
        while diff>0.03:
            fno,ctime= getNextCam(cam,coff)
            diff=gpssec-ctime

        print("%0.3f,IMG,%010d" % (gpssec,fno))
        prevfno=fno
        prevgps=gpssec
        iter+=1

    closeAndExit(gps,cam)

if __name__=='__main__':
    main()

