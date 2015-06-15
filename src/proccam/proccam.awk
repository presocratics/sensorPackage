#!/usr/bin/awk -f
# proccam.awk
# Created: 6/12/2015
# Martin Miller
# Processes grabframe output file.
# Input:
# imgnum, timestamp
# Output:
# imgnum increment, dt, dt/400000, dt/400000!=imgnum inc
#
# Here is some rationale for the outputs.
# If imgnum increment != 1, then we have some issue to examine. If dt!=400000,
# we have some issue to examine. And if dt does not equal 400000*(imgnum
# incrememt), then we have a more complicated issue to examine.



{
    di=$1-previnc;
    dt=$2-prevtime;
    d=round(dt/400000);
    e=0;
    if (di!=d) {
        e=1;
    }
    print di,dt,d,e;
    previnc=$1;
    prevtime=$2;
}
function round(val) {
    ipart=int(val);
    if ((val-ipart)>=0.5) {
        ipart=ipart+1;
    }
    return ipart;
}
