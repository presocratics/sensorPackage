#!/bin/awk -f
BEGIN {
	numerr = 0;
	valueerr = 0;
	absvalueerr= 0;
	greatesterr=0;
	threshold = 200000;
	scale =10000000;
	line =0;
	FS =",";
}
{
	dt = ($3*scale) - $4;
	if( abs(dt) > threshold) {
		numerr++;
		valueerr+=dt;
		if (valueerr > greatesterr){
			greatesterr=valueerr;
			line = $2;
			##print greatesterr;
		}
		absvalueerr+=abs(dt);
		##print NR,$2, dt, valueerr, greatesterr;
	}
}
END {
	print numerr;
	print valueerr/scale;
	print absvalueerr/scale;
	print line, greatesterr/scale;
}
function abs(dt){
	if (dt< 0){
		dt = dt * -1;
	}
	return dt;
}
