#!/usr/bin/awk -f
# rmglitch.awk
# Created: 6/12/2015
# Martin Miller
# Checks a file for glitches and removes any that are found.
BEGIN{
	FS=",";
    T=1/fps;
    lowerbound=T/2;
    upperbound=2*T;
}
{
    diff=$1-old;
    if (diff>lowerbound) {
        print $0;
    }
    old=$1;
}
