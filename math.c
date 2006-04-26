/* Panorama_Tools	-	Generate, Edit and Convert Panoramic Images
   Copyright (C) 1998,1999 - Helmut Dersch  der@fh-furtwangen.de
   
   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2, or (at your option)
   any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.  */

/*------------------------------------------------------------*/


#include "filter.h"
#include <float.h>
#include "f2c.h"

#define R_EPS  1.0e-6	
#define MAXITER 100



#ifndef abs
#define abs(a) ( (a) >= 0 ? (a) : -(a) )
#endif

void 	matrix_matrix_mult	( double m1[3][3],double m2[3][3],double result[3][3]);
int 	polzeros_();


//------------------------- Some auxilliary math functions --------------------------------------------


void matrix_mult( double m[3][3], double vector[3] )
{
	register int i;
	register double v0 = vector[0];
	register double v1 = vector[1];
	register double v2 = vector[2];
	
	
	for(i=0; i<3; i++)
	{
		vector[i] = m[i][0] * v0 + m[i][1] * v1 + m[i][2] * v2;
	}
}
		
void matrix_inv_mult( double m[3][3], double vector[3] )
{
	register int i;
	register double v0 = vector[0];
	register double v1 = vector[1];
	register double v2 = vector[2];
	
	for(i=0; i<3; i++)
	{
		vector[i] = m[0][i] * v0 + m[1][i] * v1 + m[2][i] * v2;
	}
}
		
void matrix_matrix_mult( double m1[3][3],double m2[3][3],double result[3][3])
{
	register int i,k;
	
	for(i=0;i<3;i++)
	{
		for(k=0; k<3; k++)
		{
			result[i][k] = m1[i][0] * m2[0][k] + m1[i][1] * m2[1][k] + m1[i][2] * m2[2][k];
		}
	}
}

// Set matrix elements based on Euler angles a, b, c

void SetMatrix( double a, double b, double c , double m[3][3], int cl )
{
	double mx[3][3], my[3][3], mz[3][3], dummy[3][3];
	

	// Calculate Matrices;

	mx[0][0] = 1.0 ; 				mx[0][1] = 0.0 ; 				mx[0][2] = 0.0;
	mx[1][0] = 0.0 ; 				mx[1][1] = cos(a) ; 			mx[1][2] = sin(a);
	mx[2][0] = 0.0 ;				mx[2][1] =-mx[1][2] ;			mx[2][2] = mx[1][1];
	
	my[0][0] = cos(b); 				my[0][1] = 0.0 ; 				my[0][2] =-sin(b);
	my[1][0] = 0.0 ; 				my[1][1] = 1.0 ; 				my[1][2] = 0.0;
	my[2][0] = -my[0][2];			my[2][1] = 0.0 ;				my[2][2] = my[0][0];
	
	mz[0][0] = cos(c) ; 			mz[0][1] = sin(c) ; 			mz[0][2] = 0.0;
	mz[1][0] =-mz[0][1] ; 			mz[1][1] = mz[0][0] ; 			mz[1][2] = 0.0;
	mz[2][0] = 0.0 ;				mz[2][1] = 0.0 ;				mz[2][2] = 1.0;

	if( cl )
		matrix_matrix_mult( mz,	mx,	dummy);
	else
		matrix_matrix_mult( mx,	mz,	dummy);
	matrix_matrix_mult( dummy, my, m);
}


// Do 3D-coordinate Transformation

void doCoordinateTransform( CoordInfo *ci, tMatrix *t )
{
	double m[3][3],a,b,c;
	int i;
	double mx[3][3], my[3][3], mz[3][3], dummy[3][3];
	

	// Calculate Matrices;
	a = DEG_TO_RAD( t->alpha );
	b = DEG_TO_RAD( t->beta  );
	c = DEG_TO_RAD( t->gamma );

	mx[0][0] = 1.0 ; 				mx[0][1] = 0.0 ; 				mx[0][2] = 0.0;
	mx[1][0] = 0.0 ; 				mx[1][1] = cos(a) ; 			mx[1][2] = sin(a);
	mx[2][0] = 0.0 ;				mx[2][1] =-mx[1][2] ;			mx[2][2] = mx[1][1];
	
	my[0][0] = cos(b); 				my[0][1] = 0.0 ; 				my[0][2] =-sin(b);
	my[1][0] = 0.0 ; 				my[1][1] = 1.0 ; 				my[1][2] = 0.0;
	my[2][0] = -my[0][2];			my[2][1] = 0.0 ;				my[2][2] = my[0][0];
	
	mz[0][0] = cos(c) ; 			mz[0][1] = sin(c) ; 			mz[0][2] = 0.0;
	mz[1][0] =-mz[0][1] ; 			mz[1][1] = mz[0][0] ; 			mz[1][2] = 0.0;
	mz[2][0] = 0.0 ;				mz[2][1] = 0.0 ;				mz[2][2] = 1.0;

	matrix_matrix_mult( my,	mz,	dummy);
	matrix_matrix_mult( mx, dummy,	m);
	
	// Scale 
	
	for(i=0; i<3; i++)
		ci->x[i] *= t->scale;
	
	// Do shift
	
	for(i=0; i<3; i++)
		ci->x[i] += t->x_shift[i];
	
	// Do rotation
#if 0	
	SetMatrix( DEG_TO_RAD( t->alpha ) , 
			   DEG_TO_RAD( t->beta  ) ,
			   DEG_TO_RAD( t->gamma ) ,
			   m, 0 );

#endif
	matrix_inv_mult( m, ci->x );

}

void SettMatrixDefaults( tMatrix *t )
{
	int i;
	
	t->alpha = t->beta = t->gamma = 0.0;
	for(i=0; i<3; i++)
		t->x_shift[i] = 0.0;
	
	t->scale = 1.0;
}


	
	
	
	

//------------------------------- Transformation functions --------------------------------------------


#define 	distance	(*((double*)params))
#define 	shift		(*((double*)params))
#define		var0		((double*)params)[0]
#define		var1		((double*)params)[1]

// execute a stack of functions stored in stack

void execute_stack		( double x_dest, double y_dest, double* x_src, double* y_src, void* params)
{
	register double 		xd = x_dest, 
							yd = y_dest;
	register struct fDesc*  stack = (struct fDesc *) params;;
	
		
	while( (stack->func) != NULL )
	{
		(stack->func) ( xd, yd, x_src, y_src, stack->param );
		xd = *x_src;	
		yd = *y_src;
		stack++;
	}
}

	
int execute_stack_new( double x_dest, double y_dest, double* x_src, double* y_src, void* params)
{
    register double xd = x_dest,
                    yd = y_dest;
    register struct fDesc*  stack = (struct fDesc *) params;

    while( (stack->func) != NULL )
    {
        if ( (stack->func) ( xd, yd, x_src, y_src, stack->param ) ) {
            xd = *x_src;
            yd = *y_src;
            stack++;
        } else {
            return 0;
        }
    }
    return 1;
}


// Rotate equirectangular image

int rotate_erect( double x_dest, double y_dest, double* x_src, double* y_src, void* params)
{
	// params: double 180degree_turn(screenpoints), double turn(screenpoints);

		*x_src = x_dest + var1;

		while( *x_src < - var0 )
			*x_src += 2 *  var0;

		while( *x_src >  var0 )
			*x_src -= 2 *  var0;

		*y_src = y_dest ;
    return 1;
}



// Calculate inverse 4th order polynomial correction using Newton
// Don't use on large image (slow)!


int inv_radial( double x_dest, double y_dest, double* x_src, double* y_src, void* params)
{
	// params: double coefficients[4]

	register double rs, rd, f, scale;
	int iter = 0;

	rd 		= (sqrt( x_dest*x_dest + y_dest*y_dest )) / ((double*)params)[4]; // Normalized 

	rs	= rd;				
	f 	= (((((double*)params)[3] * rs + ((double*)params)[2]) * rs + 
				((double*)params)[1]) * rs + ((double*)params)[0]) * rs;

	while( abs(f - rd) > R_EPS && iter++ < MAXITER )
	{
		rs = rs - (f - rd) / ((( 4 * ((double*)params)[3] * rs + 3 * ((double*)params)[2]) * rs  + 
						  2 * ((double*)params)[1]) * rs + ((double*)params)[0]);

		f 	= (((((double*)params)[3] * rs + ((double*)params)[2]) * rs + 
				((double*)params)[1]) * rs + ((double*)params)[0]) * rs;
	}

	scale = rs / rd;
//	printf("scale = %lg iter = %d\n", scale,iter);	
	
	*x_src = x_dest * scale  ;
	*y_src = y_dest * scale  ;
    return 1;
}

int inv_vertical( double x_dest, double y_dest, double* x_src, double* y_src, void* params)
{
	// params: double coefficients[4]

	register double rs, rd, f, scale;
	int iter = 0;

	rd 		= abs( y_dest ) / ((double*)params)[4]; // Normalized 

	rs	= rd;				
	f 	= (((((double*)params)[3] * rs + ((double*)params)[2]) * rs + 
				((double*)params)[1]) * rs + ((double*)params)[0]) * rs;

	while( abs(f - rd) > R_EPS && iter++ < MAXITER )
	{
		rs = rs - (f - rd) / ((( 4 * ((double*)params)[3] * rs + 3 * ((double*)params)[2]) * rs  + 
						  2 * ((double*)params)[1]) * rs + ((double*)params)[0]);

		f 	= (((((double*)params)[3] * rs + ((double*)params)[2]) * rs + 
				((double*)params)[1]) * rs + ((double*)params)[0]) * rs;
	}

	scale = rs / rd;
//	printf("scale = %lg iter = %d\n", scale,iter);	
	
	*x_src = x_dest  ;
	*y_src = y_dest * scale  ;
    return 1;
}



int resize( double x_dest, double y_dest, double* x_src, double* y_src, void* params)
{
	// params: double scale_horizontal, double scale_vertical;

		*x_src = x_dest * var0;
		*y_src = y_dest * var1;
    return 1;
}

int shear( double x_dest, double y_dest, double* x_src, double* y_src, void* params)
{
	// params: double shear_horizontal, double shear_vertical;

		*x_src  = x_dest + var0 * y_dest;
		*y_src  = y_dest + var1 * x_dest;
    return 1;
}

int horiz( double x_dest, double y_dest, double* x_src, double* y_src, void* params)
{
	// params: double horizontal shift

		*x_src	= x_dest + shift;	
		*y_src  = y_dest;
    return 1;
}

int vert( double x_dest, double y_dest, double* x_src, double* y_src, void* params)
{
	// params: double vertical shift

		*x_src	= x_dest;	
		*y_src  = y_dest + shift;		
    return 1;
}

	
int radial( double x_dest, double y_dest, double* x_src, double* y_src, void* params)
{
	// params: double coefficients[4], scale, correction_radius

	register double r, scale;

	r 		= (sqrt( x_dest*x_dest + y_dest*y_dest )) / ((double*)params)[4];
	if( r < ((double*)params)[5] )
	{
		scale 	= ((((double*)params)[3] * r + ((double*)params)[2]) * r + 
				((double*)params)[1]) * r + ((double*)params)[0];
	}
	else
		scale = 1000.0;
	
	*x_src = x_dest * scale  ;
	*y_src = y_dest * scale  ;
    return 1;
}

int vertical( double x_dest, double y_dest, double* x_src, double* y_src, void* params)
{
	// params: double coefficients[4]

	register double r, scale;

	r 		= y_dest / ((double*)params)[4];

	if( r < 0.0 ) r = -r;

	scale 	= ((((double*)params)[3] * r + ((double*)params)[2]) * r + 
				((double*)params)[1]) * r + ((double*)params)[0];
	
	*x_src = x_dest;
	*y_src = y_dest * scale  ;
    return 1;
}

int deregister( double x_dest, double y_dest, double* x_src, double* y_src, void* params)
{
	// params: double coefficients[4]

	register double r, scale;

	r 		= y_dest / ((double*)params)[4];

	if( r < 0.0 ) r = -r;

	scale 	= (((double*)params)[3] * r + ((double*)params)[2]) * r + 
				((double*)params)[1] ;
	
	*x_src = x_dest + abs( y_dest ) * scale;
	*y_src = y_dest ;
    return 1;
}



	
int persp_sphere( double x_dest,double  y_dest, double* x_src, double* y_src, void* params)
{
	// params :  double Matrix[3][3], double distance

	register double theta,s,r;
	double v[3];

#if 0	// old 
	theta = sqrt( x_dest * x_dest + y_dest * y_dest ) / *((double*) ((void**)params)[1]);
	phi   = atan2( y_dest , x_dest );

	v[0] = *((double*) ((void**)params)[1]) * sin( theta ) * cos( phi );
	v[1] = *((double*) ((void**)params)[1]) * sin( theta ) * sin( phi );
	v[2] = *((double*) ((void**)params)[1]) * cos( theta );
	
	matrix_inv_mult( (double(*)[3]) ((void**)params)[0], v );

	theta = atan2( sqrt( v[0]*v[0] + v[1]*v[1] ), v[2] );
	phi   = atan2( v[1], v[0] );
	*x_src = *((double*) ((void**)params)[1]) * theta * cos( phi );
	*y_src = *((double*) ((void**)params)[1]) * theta * sin( phi );
#endif

	r 		= sqrt( x_dest * x_dest + y_dest * y_dest );
	theta 	= r / *((double*) ((void**)params)[1]);
	if( r == 0.0 )
		s = 0.0;
	else
		s = sin( theta ) / r;

	v[0] =  s * x_dest ;
	v[1] =  s * y_dest ;
	v[2] =  cos( theta );

	matrix_inv_mult( (double(*)[3]) ((void**)params)[0], v );

	r 		= sqrt( v[0]*v[0] + v[1]*v[1] );
	if( r == 0.0 )
		theta = 0.0;
	else
		theta 	= *((double*) ((void**)params)[1]) * atan2( r, v[2] ) / r;
	*x_src 	= theta * v[0];
	*y_src 	= theta * v[1];

    return 1;
}	

int persp_rect( double x_dest, double y_dest, double* x_src, double* y_src, void* params)
{
	// params :  double Matrix[3][3], double distance, double x-offset, double y-offset

	double v[3];
	
	v[0] = x_dest + *((double*) ((void**)params)[2]);
	v[1] = y_dest + *((double*) ((void**)params)[3]);
	v[2] = *((double*) ((void**)params)[1]);
	
	matrix_inv_mult( (double(*)[3]) ((void**)params)[0], v );
	
	*x_src = v[0] * *((double*) ((void**)params)[1]) / v[2] ;
	*y_src = v[1] * *((double*) ((void**)params)[1]) / v[2] ;
    return 1;
}



int rect_pano( double x_dest,double  y_dest, double* x_src, double* y_src, void* params)
{									
								
	*x_src = distance * tan( x_dest / distance ) ;
	*y_src = y_dest / cos( x_dest / distance );
    return 1;
}

int pano_rect( double x_dest,double  y_dest, double* x_src, double* y_src, void* params)
{	
	*x_src = distance * atan ( x_dest / distance );
	*y_src = y_dest * cos( *x_src / distance );
    return 1;
}

int rect_erect( double x_dest,double  y_dest, double* x_src, double* y_src, void* params)
{	
	// params: double distance

	register double  phi, theta;

	phi 	= x_dest / distance;
	theta 	=  - y_dest / distance  + PI / 2.0;
	if(theta < 0)
	{
		theta = - theta;
		phi += PI;
	}
	if(theta > PI)
	{
		theta = PI - (theta - PI);
		phi += PI;
	}

#if 0
	v[2] = *((double*)params) * sin( theta ) * cos( phi );   //  x' -> z
	v[0] = *((double*)params) * sin( theta ) * sin( phi );	//  y' -> x
	v[1] = *((double*)params) * cos( theta );				//  z' -> y
	
	phi   = atan2( v[1], v[0] );
//  old:	
//	theta = atan2( sqrt( v[0]*v[0] + v[1]*v[1] ) , v[2] );
//	rho = *((double*)params) * tan( theta );
//  new:
	rho = *((double*)params) * sqrt( v[0]*v[0] + v[1]*v[1] ) / v[2];
	*x_src = rho * cos( phi );
	*y_src = rho * sin( phi );
#endif
#if 1
	*x_src = distance * tan(phi);
	*y_src = distance / (tan( theta ) * cos(phi));
#endif
    return 1;
}

int pano_erect( double x_dest,double  y_dest, double* x_src, double* y_src, void* params)
{	
	// params: double distance

	*x_src = x_dest;
	*y_src = distance * tan( y_dest / distance);
    return 1;
}

int erect_pano( double x_dest,double  y_dest, double* x_src, double* y_src, void* params)
{	
	// params: double distance

	*x_src = x_dest;
	*y_src = distance * atan( y_dest / distance);
    return 1;
}

/** convert from erect to mercator */
int mercator_erect( double x_dest,double  y_dest, double* x_src, double* y_src, void* params)
{
    // params: distance
    *x_src = x_dest;
    *y_src = distance*log(tan(y_dest/distance)+1/cos(y_dest/distance));
    return 1;
}

/** convert from mercator to erect */
int erect_mercator( double x_dest,double  y_dest, double* x_src, double* y_src, void* params)
{
    // params: distance
    *x_src = x_dest;
    *y_src = distance*atan(sinh(y_dest/distance));
    return 1;
}


/** convert from erect to transverse mercator */
int transmercator_erect( double x_dest,double  y_dest, double* x_src, double* y_src, void*  params)
{
    // params: distance
    double B;
    x_dest /= distance;
    y_dest /= distance;
    B = cos(y_dest)*sin(x_dest);
    *x_src = distance * atanh(B);
    *y_src = distance * atan2(tan(y_dest), cos(x_dest));
    return 1;
}

/** convert from erect to transverse mercator */
int erect_transmercator( double x_dest,double  y_dest, double* x_src, double* y_src, void* params)
{
    // params: distance
    x_dest /= distance;
    y_dest /= distance;
    *x_src = distance * atan2(sinh(x_dest),cos(y_dest));
    *y_src = distance * asin(sin(y_dest)/cosh(x_dest));
    return 1;
}

/** convert from erect to sinusoidal */
int sinusoidal_erect( double x_dest,double  y_dest, double* x_src, double* y_src, void*  params)
{
    // params: distance

    *x_src = distance * (x_dest/distance*cos(y_dest/distance));
    *y_src = y_dest;
    return 1;
}

/** convert from sinusoidal to erect */
int erect_sinusoidal( double x_dest,double  y_dest, double* x_src, double* y_src, void*  params)
{
    // params: distance

    *y_src = y_dest;
    *x_src = x_dest/cos(y_dest/distance);
    if (*x_src/distance < -PI || *x_src/distance > PI)
	return 0; 
    return 1;
}

/** convert from erect to stereographic */
int stereographic_erect_old( double x_dest,double  y_dest, double* x_src, double* y_src, void*  params)
{
    // params: distance
    double lon = x_dest / distance;
    double lat = y_dest / distance;

    // use: R = 1
    double k=2.0/(1+cos(lat)*cos(lon));
    *x_src = distance * k*cos(lat)*sin(lon);
    *y_src = distance * k*sin(lat);
    return 1;
}

int stereographic_erect( double x_dest,double  y_dest, double* x_src, double* y_src, void*  params)
{
    double lon, lat;
    double sinphi, cosphi, coslon;
    double g,ksp;

    lon = x_dest / distance;
    lat = y_dest / distance;

    sinphi = sin(lat);
    cosphi = cos(lat);
    coslon = cos(lon);

    g = cosphi * coslon;

    // point projects to infinity:
    //    if (fabs(g + 1.0) <= EPSLN)

    ksp = distance * 2.0 / (1.0 + g);
    *x_src = ksp * cosphi * sin(lon);
    *y_src = ksp * sinphi;

    return 1;
}

/** convert from stereographic to erect */
int erect_stereographic( double x_dest,double  y_dest, double* lon, double* lat, void*  params)
{
    double rh;		/* height above sphere*/
    double c;		/* angle					*/
    double sinc,cosc;	/* sin of c and cos of c			*/
    double con;

    /* Inverse equations
     -----------------*/
    double x = x_dest / distance;
    double y = y_dest / distance;
    rh = sqrt(x * x + y * y);
    c = 2.0 * atan(rh / (2.0 * 1));
    sinc = sin(c);
    cosc = cos(c);
    *lon = 0;
    if (fabs(rh) <= EPSLN)
    {
        *lat = 0;
        return 0;
    }
    else
    {
        *lat = asin((y * sinc) / rh) * distance;
        con = HALF_PI;
   
        con = cosc;
        if ((fabs(cosc) < EPSLN) && (fabs(x) < EPSLN))
            return 0;
        else
            *lon = atan2((x * sinc), (cosc * rh)) * distance;
    }
    return 1;
}


/** convert from stereographic to erect */
int erect_stereographic_old( double x_dest,double  y_dest, double* x_src, double* y_src, void*  params)
{
    // params: distance

    // use: R = 1
    double p=sqrt(x_dest*x_dest + y_dest*y_dest) / distance;
    double c= 2.0*atan(p/2.0);

    *x_src = distance * atan2(x_dest/distance*sin(c),(p*cos(c)));
    *y_src = distance * asin(y_dest/distance*sin(c)/p);
    return 1;
}


int sphere_cp_erect( double x_dest,double  y_dest, double* x_src, double* y_src, void* params)
{
	// params: double distance, double b

	register double phi, theta;
#if 0	
	phi 	= - x_dest /  ( var0 * PI / 2.0);
	theta 	=  - ( y_dest + var1 ) / (var0 * PI / 2.0) ;
	
	*x_src = var0 * theta * cos( phi );
	*y_src = var0 * theta * sin( phi );
#endif
	phi 	= - x_dest /  ( var0 * PI / 2.0);
	theta 	=  - ( y_dest + var1 ) / ( PI / 2.0) ;
	
	*x_src =  theta * cos( phi );
	*y_src =  theta * sin( phi );
    return 1;
}

int sphere_tp_erect( double x_dest,double  y_dest, double* x_src, double* y_src, void* params)
{
	// params: double distance

	register double phi, theta, r,s;
	double v[3];

	phi 	= x_dest / distance;
	theta 	=  - y_dest / distance  + PI / 2;
	if(theta < 0)
	{
		theta = - theta;
		phi += PI;
	}
	if(theta > PI)
	{
		theta = PI - (theta - PI);
		phi += PI;
	}

#if 0	

	v[2] = *((double*)params) * sin( theta ) * cos( phi );   //  x' -> z
	v[0] = *((double*)params) * sin( theta ) * sin( phi );	//  y' -> x
	v[1] = *((double*)params) * cos( theta );				//  z' -> y
	
	theta = atan2( sqrt( v[0]*v[0] + v[1]*v[1] ) , v[2] );
	
	phi   = atan2( v[1], v[0] );
	
	*x_src = *((double*)params) * theta * cos( phi );
	*y_src = *((double*)params) * theta * sin( phi );
#endif
	s = sin( theta );
	v[0] =  s * sin( phi );	//  y' -> x
	v[1] =  cos( theta );				//  z' -> y
	
	r = sqrt( v[1]*v[1] + v[0]*v[0]);	

	theta = distance * atan2( r , s * cos( phi ) );
	
	*x_src =  theta * v[0] / r;
	*y_src =  theta * v[1] / r;
    return 1;
}

int erect_sphere_cp( double x_dest,double  y_dest, double* x_src, double* y_src, void* params)
{
	// params: double distance, double b

	register double phi, theta;

#if 0
	theta = sqrt( x_dest * x_dest + y_dest * y_dest ) / var0;
	phi   = atan2( y_dest , -x_dest );
	
	*x_src = var0 * phi;
	*y_src = var0 * theta - var1;
#endif
	theta = sqrt( x_dest * x_dest + y_dest * y_dest ) ;
	phi   = atan2( y_dest , -x_dest );
	
	*x_src = var0 * phi;
	*y_src = theta - var1;
    return 1;
}

int rect_sphere_tp( double x_dest,double  y_dest, double* x_src, double* y_src, void* params)
{
	// params: double distance

	register double rho, theta,r;

#if 0	
	theta = sqrt( x_dest * x_dest + y_dest * y_dest ) / distance;
	phi   = atan2( y_dest , x_dest );
	
	if( theta > PI /2.0  ||  theta < -PI /2.0 )
		theta = PI /2.0 ;

	rho = distance * tan( theta );

	*x_src = rho * cos( phi );
	*y_src = rho * sin( phi );
#endif
	r 		= sqrt( x_dest * x_dest + y_dest * y_dest );
	theta 	= r / distance;
	
	if( theta >= PI /2.0   )
		rho = 1.6e16 ;
	else if( theta == 0.0 )
		rho = 1.0;
	else
		rho =  tan( theta ) / theta;
	*x_src = rho * x_dest ;
	*y_src = rho * y_dest ;
    return 1;
}

int sphere_tp_rect( double x_dest,double  y_dest, double* x_src, double* y_src, void* params)
{	
	// params: double distance

	register double  theta, r;

#if 0	
	theta = atan( sqrt(x_dest*x_dest + y_dest*y_dest) / *((double*)params));
	phi   = atan2( y_dest , x_dest );
	
	*x_src = *((double*)params) * theta * cos( phi );
	*y_src = *((double*)params) * theta * sin( phi );
#endif
	r 		= sqrt(x_dest*x_dest + y_dest*y_dest) / distance;
	if( r== 0.0 )
		theta = 1.0;
	else
		theta 	= atan( r ) / r;
	
	*x_src =  theta * x_dest ;
	*y_src =  theta * y_dest ;
    return 1;
}

int sphere_tp_pano( double x_dest,double  y_dest, double* x_src, double* y_src, void* params)
{
	// params: double distance

	register double r, s, Phi, theta;
	
#if 0
	register double Theta, phi;
	double v[3];
	
	Phi = x_dest / *((double*)params);
	Theta = PI /2.0 - atan( y_dest / distance );
	

	v[2] = *((double*)params) * sin( Theta ) * cos( Phi );   //  x' -> z
	v[0] = *((double*)params) * sin( Theta ) * sin( Phi );	//  y' -> x
	v[1] = *((double*)params) * cos( Theta );				//  z' -> y
	
	theta = atan2( sqrt( v[0]*v[0] + v[1]*v[1] ) , v[2] );
	phi   = atan2( v[1], v[0] );
	
	*x_src = *((double*)params) * theta * cos( phi );
	*y_src = *((double*)params) * theta * sin( phi );
#endif
#if 1
	Phi = x_dest / distance;
		
	s =  distance * sin( Phi ) ;	//  y' -> x
	
	r = sqrt( s*s + y_dest*y_dest );
	theta = distance * atan2( r , (distance * cos( Phi )) ) / r;
	
	*x_src =  theta * s ;
	*y_src =  theta * y_dest ;
#endif
    return 1;
}

int pano_sphere_tp( double x_dest,double  y_dest, double* x_src, double* y_src, void* params)
{
	// params: double distance
	register double r,s, theta;
	double v[3];

#if 0	
	theta = sqrt( x_dest * x_dest + y_dest * y_dest ) / distance;
	phi   = atan2( y_dest , x_dest );

	v[1] = *((double*)params) * sin( theta ) * cos( phi );   //  x' -> y
	v[2] = *((double*)params) * sin( theta ) * sin( phi );	//  y' -> z
	v[0] = *((double*)params) * cos( theta );				//  z' -> x

	theta = atan2( sqrt( v[0]*v[0] + v[1]*v[1] ) , v[2] );
	phi   = atan2( v[1], v[0] );

	*x_src = *((double*)params) * phi;
	*y_src = *((double*)params) * tan( (-theta + PI /2.0) );
#endif
	
	r = sqrt( x_dest * x_dest + y_dest * y_dest );
	theta = r / distance;
	if( theta == 0.0 )
		s = 1.0 / distance;
	else
		s = sin( theta ) /r;

	v[1] =  s * x_dest ;   //  x' -> y
	v[0] =  cos( theta );				//  z' -> x


	*x_src = distance * atan2( v[1], v[0] );
	*y_src = distance * s * y_dest / sqrt( v[0]*v[0] + v[1]*v[1] );

    return 1;
}


int sphere_cp_pano( double x_dest,double  y_dest, double* x_src, double* y_src, void* params)
{
	// params: double distance

	register double phi, theta;
	
	
	phi 	= -x_dest / (distance * PI / 2.0) ;
	theta	= PI /2.0 + atan( y_dest / (distance * PI/2.0) );

	*x_src = distance * theta * cos( phi );
	*y_src = distance * theta * sin( phi );
    return 1;
}

int erect_rect( double x_dest,double  y_dest, double* x_src, double* y_src, void* params)
{
	// params: double distance
#if 0
	theta = atan( sqrt(x_dest*x_dest + y_dest*y_dest) / distance );
	phi   = atan2( y_dest , x_dest );


	v[1] = distance * sin( theta ) * cos( phi );   //  x' -> y
	v[2] = distance * sin( theta ) * sin( phi );	//  y' -> z
	v[0] = distance * cos( theta );				//  z' -> x
	
	theta = atan2( sqrt( v[0]*v[0] + v[1]*v[1] ) , v[2] );
	phi   = atan2( v[1], v[0] );

	*x_src = distance * phi;
	*y_src = distance * (-theta + PI /2.0);
#endif

	*x_src = distance * atan2( x_dest, distance );
	*y_src = distance * atan2(  y_dest, sqrt( distance*distance + x_dest*x_dest ) );

    return 1;
}


int erect_sphere_tp( double x_dest,double  y_dest, double* x_src, double* y_src, void* params)
{
	// params: double distance

	register double  theta,r,s;
	double	v[3];
#if 0	
	theta = sqrt( x_dest * x_dest + y_dest * y_dest ) / *((double*)params);
	phi   = atan2( y_dest , x_dest );
	
	v[1] = *((double*)params) * sin( theta ) * cos( phi );   //  x' -> y
	v[2] = *((double*)params) * sin( theta ) * sin( phi );	//  y' -> z
	v[0] = *((double*)params) * cos( theta );				//  z' -> x
	
	theta = atan( sqrt( v[0]*v[0] + v[1]*v[1] ) / v[2] ); //was atan2
	phi   = atan2( v[1], v[0] );

	*x_src = *((double*)params) * phi;
	if(theta > 0.0)
	{
		*y_src = *((double*)params) * (-theta + PI /2.0);
	}
	else
		*y_src = *((double*)params) * (-theta - PI /2.0);
#endif
	r = sqrt( x_dest * x_dest + y_dest * y_dest );
	theta = r / distance;
	if(theta == 0.0)
		s = 1.0 / distance;
	else
		s = sin( theta) / r;
	
	v[1] =  s * x_dest;   
	v[0] =  cos( theta );				
	

	*x_src = distance * atan2( v[1], v[0] );
	*y_src = distance * atan( s * y_dest /sqrt( v[0]*v[0] + v[1]*v[1] ) ); 
    return 1;
}

int mirror_sphere_cp( double x_dest,double  y_dest, double* x_src, double* y_src, void* params)
{
	// params: double distance, double b

	register double rho, phi, theta;

	theta = sqrt( x_dest * x_dest + y_dest * y_dest ) / ((double*)params)[0];
	phi   = atan2( y_dest , x_dest );
	
	rho = ((double*)params)[1] * sin( theta / 2.0 );
	
	*x_src = - rho * cos( phi );
	*y_src = rho * sin( phi );
    return 1;
}

int mirror_erect( double x_dest,double  y_dest, double* x_src, double* y_src, void* params)
{
	// params: double distance, double b, double b2

	register double phi, theta, rho;
	
	phi 	=  x_dest / ( ((double*)params)[0] * PI/2.0) ;
	theta 	=  - ( y_dest + ((double*)params)[2] ) / (((double*)params)[0] * PI/2.0)  ;
	
	rho = ((double*)params)[1] * sin( theta / 2.0 );
	
	*x_src = - rho * cos( phi );
	*y_src = rho * sin( phi );
    return 1;
}

int mirror_pano( double x_dest,double  y_dest, double* x_src, double* y_src, void* params)
{
	// params: double distance, double b

	register double phi, theta, rho;
	
	
	phi 	= -x_dest / (((double*)params)[0] * PI/2.0) ;
	theta	= PI /2.0 + atan( y_dest / (((double*)params)[0] * PI/2.0) );

	rho = ((double*)params)[1] * sin( theta / 2.0 );
	
	*x_src = rho * cos( phi );
	*y_src = rho * sin( phi );
    return 1;
}

int sphere_cp_mirror( double x_dest,double  y_dest, double* x_src, double* y_src, void* params)
{
	// params: double distance, double b

	register double phi, theta, rho;

	rho = sqrt( x_dest*x_dest + y_dest*y_dest );
	
	theta = 2 * asin( rho/((double*)params)[1] );
	phi   = atan2( y_dest , x_dest );

	*x_src = ((double*)params)[0] * theta * cos( phi );
	*y_src = ((double*)params)[0] * theta * sin( phi );
    return 1;
}


int shift_scale_rotate( double x_dest,double  y_dest, double* x_src, double* y_src, void* params){
	// params: double shift_x, shift_y, scale, cos_phi, sin_phi
	
	register double x = x_dest - ((double*)params)[0];
	register double y = y_dest - ((double*)params)[1];

	*x_src = (x * ((double*)params)[3] - y * ((double*)params)[4]) * ((double*)params)[2];
	*y_src = (x * ((double*)params)[4] + y * ((double*)params)[3]) * ((double*)params)[2];
    return 1;
}













// Correct radial luminance change using parabel

unsigned char radlum( unsigned char srcPixel, int xc, int yc, void *params )
{
	// params: second and zero order polynomial coeff
	register double result;

	result = (xc * xc + yc * yc) * ((double*)params)[0] + ((double*)params)[1];
	result = ((double)srcPixel) - result;

    // JMW 2003/08/25  randomize a little
    result = result * ( (1 + LUMINANCE_RANDOMIZE/2) - LUMINANCE_RANDOMIZE * rand() / (double)RAND_MAX );
    
	if(result < 0.0) return 0;
	if(result > 255.0) return 255;

	return( (unsigned char)(result+0.5) );
}

//Kekus 16 bit: 2003/Nov/18
//Correct radial luminance change using parabel (16-bit supported)
unsigned short radlum16( unsigned short srcPixel, int xc, int yc, void *params ) 
{
	// params: second and zero order polynomial coeff
	register double result;

	result = (xc * xc + yc * yc) * ((double*)params)[0] + ((double*)params)[1];
    result = ((double) srcPixel) - result*256;
    // JMW 2003/08/25  randomize a little to remove banding added by Kekus Digital 26 Aug 2003
	// JMW 2004/07/11 a power of two less randomizing for 16 bit
    result = result * ( (1 + LUMINANCE_RANDOMIZE * LUMINANCE_RANDOMIZE /2) - 
		LUMINANCE_RANDOMIZE * LUMINANCE_RANDOMIZE * rand() / (double)RAND_MAX );
    if(result > 65535.0) return 65535;
	if(result < 0.0) return 0;

	return( (unsigned short)(result+0.5) );
}
//Kekus.

// Get smallest positive (non-zero) root of polynomial with degree deg and
// (n+1) real coefficients p[i]. Return it, or 1000.0 if none exists or error occured
// Changed to only allow degree 3
#if 0
double smallestRoot( double *p )
{
	doublecomplex 		root[3], poly[4];
	doublereal 			radius[3], apoly[4], apolyr[4];
	logical 			myErr[3];
	double 				sRoot = 1000.0;
	doublereal 			theEps, theBig, theSmall;
	integer 			nitmax;
	integer 			iter;
	integer 			n,i;
	
	n 		= 3;

	
	for( i=0; i< n+1; i++)
	{
		poly[i].r = p[i];
		poly[i].i = 0.0;
	}
	
	theEps   = DBL_EPSILON;  		// machine precision 
	theSmall = DBL_MIN ; 			// smallest positive real*8          
	theBig   = DBL_MAX ; 			// largest real*8  

	nitmax 	= 100;

    polzeros_(&n, poly, &theEps, &theBig, &theSmall, &nitmax, root, radius, myErr, &iter, apoly, apolyr);

	for( i = 0; i < n; i++ )
	{
//		PrintError("No %d : Real %g, Imag %g, radius %g, myErr %ld", i, root[i].r, root[i].i, radius[i], myErr[i]);
		if( (root[i].r > 0.0) && (dabs( root[i].i ) <= radius[i]) && (root[i].r < sRoot) )
			sRoot = root[i].r;
	}

	return sRoot;
}
#endif

void cubeZero( double *a, int *n, double *root );
void squareZero( double *a, int *n, double *root );
double cubeRoot( double x );


void cubeZero( double *a, int *n, double *root ){
	if( a[3] == 0.0 ){ // second order polynomial
		squareZero( a, n, root );
	}else{
		double p = ((-1.0/3.0) * (a[2]/a[3]) * (a[2]/a[3]) + a[1]/a[3]) / 3.0;
		double q = ((2.0/27.0) * (a[2]/a[3]) * (a[2]/a[3]) * (a[2]/a[3]) - (1.0/3.0) * (a[2]/a[3]) * (a[1]/a[3]) + a[0]/a[3]) / 2.0;
		
		if( q*q + p*p*p >= 0.0 ){
			*n = 1;
			root[0] = cubeRoot(-q + sqrt(q*q + p*p*p)) + cubeRoot(-q - sqrt(q*q + p*p*p)) - a[2] / (3.0 * a[3]); 
		}else{
			double phi = acos( -q / sqrt(-p*p*p) );
			*n = 3;
			root[0] =  2.0 * sqrt(-p) * cos(phi/3.0) - a[2] / (3.0 * a[3]); 
			root[1] = -2.0 * sqrt(-p) * cos(phi/3.0 + PI/3.0) - a[2] / (3.0 * a[3]); 
			root[2] = -2.0 * sqrt(-p) * cos(phi/3.0 - PI/3.0) - a[2] / (3.0 * a[3]); 
		}
	}
	// PrintError("%lg, %lg, %lg, %lg root = %lg", a[3], a[2], a[1], a[0], root[0]);
}

void squareZero( double *a, int *n, double *root ){
	if( a[2] == 0.0 ){ // linear equation
		if( a[1] == 0.0 ){ // constant
			if( a[0] == 0.0 ){
				*n = 1; root[0] = 0.0;
			}else{
				*n = 0;
			}
		}else{
			*n = 1; root[0] = - a[0] / a[1];
		}
	}else{
		if( 4.0 * a[2] * a[0] > a[1] * a[1] ){
			*n = 0; 
		}else{
			*n = 2;
			root[0] = (- a[1] + sqrt( a[1] * a[1] - 4.0 * a[2] * a[0] )) / (2.0 * a[2]);
			root[1] = (- a[1] - sqrt( a[1] * a[1] - 4.0 * a[2] * a[0] )) / (2.0 * a[2]);
		}
	}

}

double cubeRoot( double x ){
	if( x == 0.0 )
		return 0.0;
	else if( x > 0.0 )
		return pow(x, 1.0/3.0);
	else
		return - pow(-x, 1.0/3.0);
}

double smallestRoot( double *p ){
	int n,i;
	double root[3], sroot = 1000.0;
	
	cubeZero( p, &n, root );
	
	for( i=0; i<n; i++){
		// PrintError("Root %d = %lg", i,root[i]);
		if(root[i] > 0.0 && root[i] < sroot)
			sroot = root[i];
	}
	
	// PrintError("Smallest Root  = %lg", sroot);
	return sroot;
}



