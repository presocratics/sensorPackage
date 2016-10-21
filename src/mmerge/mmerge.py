#!/usr/bin/env python2.7
# mmerge.py
# Martin Miller
# Created: 2016/06/28
# Merge sorts two files by the first column. Attempts to replicate
#  sort -t, -n -m foo bar
# In my experience <(tail -n+1 -f foo) <(tail -n+1 -f bar) does not work for
# unix sort, hence this program
import sys

def main():
    fin1 = open(sys.argv[1],'r')
    fin2 = open(sys.argv[2],'r')

    line1=fin1.readline().strip()
    line2=fin2.readline().strip()

    t1,msg1 = line1.split(',',1)
    t2,msg2 = line2.split(',',1)

    time1=float(t1)
    time2=float(t2)

    while time2>time1:
        print(line1)
        line1=fin1.readline().strip()
        t1,msg1 = line1.split(',',1)
        time1=float(t1)

    while 1:
        line1 = fin1.readline().strip()
        t1,msg1 = line1.split(',',1)
        time1=float(t1)
        while time2<time1:
            print(line2)
            line2 = fin2.readline().strip()
            t2,msg2 = line2.split(',',1)
            time2=float(t2)
        print(line1)


if __name__=='__main__':
    main()

