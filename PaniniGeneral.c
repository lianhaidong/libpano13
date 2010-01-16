/* PaniniGeneral.c		15Jan2010 TKS

This is the reference implementation of the General Pannini
Projection, an elaboration of the basic Pannini projection 
discovered by Bruno Postle and Thomas Sharpless in December 
2008 in paintings by Gian Paolo Pannini (1691-1765).

    (C) copyright 2010 Thomas K Sharpless
Free license is hereby granted for noncommercial use
under the terms of the GNU Lesser General Public License 
version 2.1 as published by the Free Software Foundation 
and appearing in the file LICENSE.LGPL included in the
packaging of this file.  Please review the following 
information to ensure the requirementsGeneral of the GNU Lesser 
General Public License version 2.1 will be met: 
http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.


*/
#include "PaniniGeneral.h"

#include <math.h>

#define PI 3.1415926535897932384626433832795
#define D2R( x ) ((x) * PI / 180 )
#define R2D( x ) ((x) * 180 / PI )

int panini_general_toSphere	( double* lon, double* lat, 
							  double h,  double v, 
							  double d, double top, double bot
							 )
{ 
	double S, cl, q, t;

	if( d < 0 ) return 0;
	if( h == 0 ){
        *lon = 0;
		S = 1;
		cl = 1;
	} else {
	/* solve quadratic for cosine of azimuth angle */
        double k, kk, dd, del;
        k = fabs(h) / (d + 1);
        kk = k * k;
        dd = d * d;
        del = kk * kk * dd - (kk + 1) * (kk * dd - 1);
        if( del < 0 ) 
            return 0;
        cl = (-kk * d + sqrt( del )) / (kk + 1);
	/* use that to compute S, and angle */
        S = (d + cl)/(d + 1);
		*lon = atan2( S * h, cl );
    }
	*lat = atan(S * v);

  /* squeeze */
	q = v < 0 ? top : bot;
	if( q < 0 ){
		q = -q;
	/* soft squeeze */
		t = q * 2 * (cl - 0.707) / PI;
		*lat *= t + 1;
	} else if( q > 0 ){
	/* hard squeeze */
		t = atan( v * cl );
		t = q * (t - *lat);
		*lat += t;
	}

	return 1;
}
int panini_general_toPlane	( double lon, double lat, 
							  double*  h,  double*  v, 
							  double d, double top, double bot
							 )
{
	double S, q, t;

	if( d < 0 ) return 0;

	S = (d + 1) / (d + cos(lon));
	*h = sin(lon) * S;
	*v = tan(lat) * S;
    
  /* squeeze */
	q = v < 0 ? top : bot;
	if( q < 0 ){
		q = -q;
	/* soft squeeze */
		t = q * 2 * (cos(lon) - 0.707) / PI;
		*v = S * tan(lat /(t + 1));
	} else if( q > 0 ){
	/* hard squeeze */
		t = tan(lat) * cos(lon);
		*v += q * (t - *v);
	}


	return 1;
}

int panini_general_maxVAs	( double d, 
							  double maxProj,
							  double * maxView
							 )
{	double a, s;

	if( d < 0 ) return 0;

/* hFOV... */
  // theoretical max angle (infeasible for d < 1.1 or so)
	if( d > 1. )
		s =  -1/d;
	else 
		s = -d;  
	a = acos( s );
  // actual limit may be max projection angle...
	s = asin(d * sin(maxProj)) + maxProj ; 
	if( a > s ){	// clip to projection angle limit
		a = s;	
	} 
    maxView[0] = a;

/* vFOV... */
	maxView[1] = maxProj;

	return 1;
}
