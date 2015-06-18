#!/usr/bin/env python2.7
# rectify.py
# Martin Miller
# Created: 2015/06/16
# Reads in a gps timestamp file and a pics file and removes gps timestamps where
# pics increment is greater than 1
import sys

def main():
    if len(sys.argv)!=3:
        print("Usage: %s gps pics" % sys.argv[0])
        exit()

    fgps=open(sys.argv[1],'r')
    fpic=open(sys.argv[2],'r')
    for line in fpic:
        line=line.strip()
        inc=int(line)
        gln=""
        while inc!=0:
            inc-=1
            gln=fgps.next()
        gln=gln.strip()
        print(gln)
            

    fgps.close()
    fpic.close()


if __name__=='__main__':
    main()

