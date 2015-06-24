#!/usr/bin/awk -f
# rmglitch.awk
# Created: 6/12/2015
# Martin Miller
# Checks a file for glitches and removes any that are found.
BEGIN{
	FS=",";
}
{ 
    dt=$1-prev;
    sdt=dt+dtprev;
    if (dt>0.035) {
        print $1 FS $2 FS dt;
        prev=$1;
        dtprev=dt;
    } else if (sdt>0.035 && sdt<0.045) {
        print $1 FS $2 FS sdt;
        prev=$1;
        dtprev=sdt;
    } else if (sdt>0.045) {
        prev=$1 FS $2;
        dtprev=dt;
    } else {
        print "weird";
    }
}
