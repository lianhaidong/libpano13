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

#include <assert.h>


#ifndef abs
#define abs(a) ( (a) >= 0 ? (a) : -(a) )
#endif

#ifdef _MSC_VER
#define isnan(a) _isnan(a)
#define isinf(a) (_fpclass(a) == _FPCLASS_NINF || _fpclass(a) == _FPCLASS_PINF)
#endif

void 	matrix_matrix_mult	( double m1[3][3],double m2[3][3],double result[3][3]);
int 	polzeros_();


//------------------------- Some auxilliary math functions --------------------------------------------

// atanh is not available on MSVC. Use the atanh routine from gsl
#ifdef _MSC_VER

#define GSL_DBL_EPSILON        2.2204460492503131e-16
#define GSL_SQRT_DBL_EPSILON   1.4901161193847656e-08 

static double
log1p (const double x)
{
  volatile double y;
  y = 1 + x;
  return log(y) - ((y-1)-x)/y ;  /* cancels errors with IEEE arithmetic */
} 

static double
atanh (const double x)
{
  double a = fabs (x);
  double s = (x < 0) ? -1 : 1;

  if (a > 1)
    {
      //return NAN;
      return 0;
    }
  else if (a == 1)
    {
      //return (x < 0) ? GSL_NEGINF : GSL_POSINF; 
      return (x < 0) ? -1e300 : 1e300;
    }
  else if (a >= 0.5)
    {
      return s * 0.5 * log1p (2 * a / (1 - a));
    }
  else if (a > GSL_DBL_EPSILON)
    {
      return s * 0.5 * log1p (2 * a + 2 * a * a / (1 - a));
    }
  else
    {
      return x;
    }
} 
#endif

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


#define 	distanceparam	(*((double*)params))
#define 	shift		(*((double*)params))
#define		var0		((double*)params)[0]
#define		var1		((double*)params)[1]
#define		mp		((struct MakeParams*)params)

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
	    //	    printf("Execute stack %f %f %f %f\n", xd, yd, *x_src, *y_src);
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
	// params :  double Matrix[3][3], double distanceparam

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
	// params :  double Matrix[3][3], double distanceparam, double x-offset, double y-offset

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
								
	*x_src = distanceparam * tan( x_dest / distanceparam ) ;
	*y_src = y_dest / cos( x_dest / distanceparam );
    return 1;
}

int pano_rect( double x_dest,double  y_dest, double* x_src, double* y_src, void* params)
{	
	*x_src = distanceparam * atan ( x_dest / distanceparam );
	*y_src = y_dest * cos( *x_src / distanceparam );
    return 1;
}

int rect_erect( double x_dest,double  y_dest, double* x_src, double* y_src, void* params)
{	
	// params: double distanceparam

	register double  phi, theta;

	phi 	= x_dest / distanceparam;
	theta 	=  - y_dest / distanceparam  + PI / 2.0;
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
	*x_src = distanceparam * tan(phi);
	*y_src = distanceparam / (tan( theta ) * cos(phi));
#endif
    return 1;
}
// This is the cylindrical projection
int pano_erect( double x_dest,double  y_dest, double* x_src, double* y_src, void* params)
{	
	// params: double distanceparam

	*x_src = x_dest;
	*y_src = distanceparam * tan( y_dest / distanceparam);
    return 1;
}

int erect_pano( double x_dest,double  y_dest, double* x_src, double* y_src, void* params)
{	
	// params: double distanceparam

	*x_src = x_dest;
	*y_src = distanceparam * atan( y_dest / distanceparam);
    return 1;
}

/** convert from erect to lambert azimuthal */
int lambertazimuthal_erect( double x_dest,double  y_dest, double* x_src, double* y_src, void* params)
{
    // params: distanceparam
    double phi, lambda,k1;
    lambda = x_dest/distanceparam;
    phi = y_dest/distanceparam;

    if (abs(cos(phi) * cos(lambda) + 1.0) <= EPSLN) {
      *x_src = distanceparam * 2 ;
      *y_src = 0;
      return 0;
    }

    k1 = sqrt(2.0 / (1 + cos(phi) * cos(lambda)));

    *x_src = distanceparam * k1 * cos(phi) * sin (lambda);
    *y_src = distanceparam * k1 * sin(phi);

    return 1;
}

/** convert from lambert azimuthal to erect */
int erect_lambertazimuthal( double x_dest,double  y_dest, double* x_src, double* y_src, void* params)
{

    double x, y, ro,c;

    x = x_dest/distanceparam;
    y = y_dest/distanceparam;

    assert(! isnan(x));
    assert(! isnan(y));

    if (fabs(x) > PI || fabs(y) > PI) {
        *y_src = 0;
        *x_src = 0;
	return 0;
    }

    ro = hypot(x, y);

    if (fabs(ro) <= EPSLN)
    {
        *y_src = 0;
        *x_src = 0;
        return 1;
    }

    c = 2 * asin(ro / 2.0);

    *y_src = distanceparam * asin( (y * sin(c)) / ro);


    if (fabs(ro * cos(c)) <= EPSLN ) {
      *x_src = 0;
      return 1;
    }

    *x_src = distanceparam * atan2( x * sin(c), (ro * cos(c)));

    return 1;

}



/** convert from erect to mercator */
int mercator_erect( double x_dest,double  y_dest, double* x_src, double* y_src, void* params)
{
    // params: distanceparam
    *x_src = x_dest;
    *y_src = distanceparam*log(tan(y_dest/distanceparam)+1/cos(y_dest/distanceparam));
    return 1;
}

/** convert from mercator to erect */
int erect_mercator( double x_dest,double  y_dest, double* x_src, double* y_src, void* params)
{
    // params: distanceparam
    *x_src = x_dest;
    *y_src = distanceparam*atan(sinh(y_dest/distanceparam));
    return 1;
}


/** convert from erect to miller */
int millercylindrical_erect( double x_dest,double  y_dest, double* x_src, double* y_src, void* params)
{
    // params: distanceparam
    double phi;

    *x_src = x_dest;
    phi = y_dest/distanceparam;

   *y_src = distanceparam*log(tan(PI/4 +0.4 * phi))/0.8;
    return 1;
}


/** convert from mercator to erect */
int erect_millercylindrical( double x_dest,double  y_dest, double* x_src, double* y_src, void* params)
{
    double y;

    *x_src = x_dest;
    y = y_dest/distanceparam;

    *y_src = 1.25 * atan(sinh(4 * y /5.0));
    *y_src *= distanceparam;
    return 1;
}


/** convert from erect to panini */
int panini_erect( double x_dest,double  y_dest, double* x_src, double* y_src, void* params)
{
    // params: distanceparam
  // this is the inverse

    double phi, lambdaHalf;

    phi = y_dest/distanceparam;
    lambdaHalf = x_dest/ (distanceparam*2);

    *x_src = distanceparam * 2 * tan (lambdaHalf);
    double temp  = cos(lambdaHalf);
    *y_src = distanceparam *  phi / (temp * temp);

    return 1;
}


/** convert from panini to erect */
int erect_panini( double x_dest,double  y_dest, double* x_src, double* y_src, void* params)
{
    double y;
    double x;
    double lambdaHalf;
    double temp;

    double phi;
    y = y_dest/distanceparam;
    x = x_dest/distanceparam;

    temp = cos(x/2);
    phi = y * temp * temp;

    *x_src = 2 * atan2(x,2) * distanceparam;
    *y_src = phi * distanceparam;

    return 1;
}





/** convert from erect to lambert */
int lambert_erect( double x_dest,double  y_dest, double* x_src, double* y_src, void* params)
{
    // params: distanceparam
    *x_src = x_dest;
    *y_src = distanceparam*sin(y_dest/distanceparam);
    return 1;
}

/** convert from lambert to erect */
int erect_lambert( double x_dest,double  y_dest, double* x_src, double* y_src, void* params)
{
    // params: distanceparam
    *x_src = x_dest;
    *y_src = distanceparam*asin(y_dest/distanceparam);
    return 1;
}


/** convert from erect to transverse mercator */
int transmercator_erect( double x_dest,double  y_dest, double* x_src, double* y_src, void*  params)
{
    // params: distanceparam
    double B;
    x_dest /= distanceparam;
    y_dest /= distanceparam;
    B = cos(y_dest)*sin(x_dest);
    *x_src = distanceparam * atanh(B);
    *y_src = distanceparam * atan2(tan(y_dest), cos(x_dest));

    if (isinf(*x_src)) {
      return 0;
    }

    return 1;
}

/** convert from erect to transverse mercator */
int erect_transmercator( double x_dest,double  y_dest, double* x_src, double* y_src, void* params)
{
    // params: distanceparam
    x_dest /= distanceparam;
    y_dest /= distanceparam;

    if (fabs(y_dest) > PI ) {
        *y_src = 0;
        *x_src = 0;
	return 0;
    }


    *x_src = distanceparam * atan2(sinh(x_dest),cos(y_dest));
    *y_src = distanceparam * asin(sin(y_dest)/cosh(x_dest));
    return 1;
}

/** convert from erect to sinusoidal */
int sinusoidal_erect( double x_dest,double  y_dest, double* x_src, double* y_src, void*  params)
{
    // params: distanceparam

    *x_src = distanceparam * (x_dest/distanceparam*cos(y_dest/distanceparam));
    *y_src = y_dest;
    return 1;
}

/** convert from sinusoidal to erect */
int erect_sinusoidal( double x_dest,double  y_dest, double* x_src, double* y_src, void*  params)
{
    // params: distanceparam

    *y_src = y_dest;
    *x_src = x_dest/cos(y_dest/distanceparam);
    if (*x_src/distanceparam < -PI || *x_src/distanceparam > PI)
	return 0; 
    return 1;
}

/** convert from erect to stereographic */
int stereographic_erect_old( double x_dest,double  y_dest, double* x_src, double* y_src, void*  params)
{
    // params: distanceparam
    double lon = x_dest / distanceparam;
    double lat = y_dest / distanceparam;

    // use: R = 1
    double k=2.0/(1+cos(lat)*cos(lon));
    *x_src = distanceparam * k*cos(lat)*sin(lon);
    *y_src = distanceparam * k*sin(lat);
    return 1;
}

int stereographic_erect( double x_dest,double  y_dest, double* x_src, double* y_src, void*  params)
{
    double lon, lat;
    double sinphi, cosphi, coslon;
    double g,ksp;

    lon = x_dest / distanceparam;
    lat = y_dest / distanceparam;

    sinphi = sin(lat);
    cosphi = cos(lat);
    coslon = cos(lon);

    g = cosphi * coslon;

    // point projects to infinity:
    //    if (fabs(g + 1.0) <= EPSLN)

    ksp = distanceparam * 2.0 / (1.0 + g);
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
    double x = x_dest / distanceparam;
    double y = y_dest / distanceparam;
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
        *lat = asin((y * sinc) / rh) * distanceparam;
        con = HALF_PI;
   
        con = cosc;
        if ((fabs(cosc) < EPSLN) && (fabs(x) < EPSLN))
            return 0;
        else
            *lon = atan2((x * sinc), (cosc * rh)) * distanceparam;
    }
    return 1;
}


/** convert from stereographic to erect */
int erect_stereographic_old( double x_dest,double  y_dest, double* x_src, double* y_src, void*  params)
{
    // params: distanceparam

    // use: R = 1
    double p=sqrt(x_dest*x_dest + y_dest*y_dest) / distanceparam;
    double c= 2.0*atan(p/2.0);

    *x_src = distanceparam * atan2(x_dest/distanceparam*sin(c),(p*cos(c)));
    *y_src = distanceparam * asin(y_dest/distanceparam*sin(c)/p);
    return 1;
}

int albersEqualAreaConic_ParamCheck(Image *im) 
{

    //Parameters: phi1, phi2, phi0, n, C, rho0, yoffset
    double phi1, phi2, n, C, rho0, phi0, y1, y2, y, twiceN;
    double phi[] = {-PI/2, 0, PI/2};
    double lambda[] = {-PI, 0, PI};
    int i, j, first;

    assert(PANO_PROJECTION_MAX_PARMS >= 2);

    if (im->formatParamCount == 1) {
	// WHen only one parameter provided, assume phi1=phi0
	im->formatParamCount = 2;
	im->formatParam[1] = im->formatParam[0];
    }

    if (im->formatParamCount == 0) {
	im->formatParamCount = 2;
	im->formatParam[0] = 0;  //phi1
	im->formatParam[1] = -60; //phi2
    }

    if (im->precomputedCount == 0) {
	im->precomputedCount = 10;

	assert(PANO_PROJECTION_PRECOMPUTED_VALUES >=im->precomputedCount );


	// First, invert standard parallels. 
	// This is a hack, as the resulting projections look backwards to what they are supposed to be
	// (with respect to maps)
	
	im->precomputedValue[0] =  -1.0 * im->formatParam[0];
	im->precomputedValue[1] =  -1.0 * im->formatParam[1];

	phi1 = im->precomputedValue[0] * PI / 180.0; //phi1 to rad
	phi2 = im->precomputedValue[1] * PI / 180.0; //phi2 to rad

	//Calculate the y at 6 different positions (lambda=-pi,0,+pi; phi=-pi/2,0,pi/2).
	///Then calculate a yoffset so that the image is centered.
	first = 1;
	for (i = 0; i < 3; i++) for (j = 0; j < 3; j++) {
	    y = sqrt(pow(cos(phi1), 0.2e1) + 0.2e1 * (sin(phi1) / 0.2e1 + sin(phi2) / 0.2e1) * sin(phi1)) / (sin(phi1) / 0.2e1 + sin(phi2) / 0.2e1) - sqrt(pow(cos(phi1), 0.2e1) + 0.2e1 * (sin(phi1) / 0.2e1 + sin(phi2) / 0.2e1) * sin(phi1) - 0.2e1 * (sin(phi1) / 0.2e1 + sin(phi2) / 0.2e1) * sin(phi[i])) / (sin(phi1) / 0.2e1 + sin(phi2) / 0.2e1) * cos((sin(phi1) / 0.2e1 + sin(phi2) / 0.2e1) * lambda[j]);
	    if (!isnan(y)) {
		if (first || y < y1) y1 = y;
		if (first || y > y2) y2 = y;
		first = 0;
	    }
	}
	if (first) {
	    y = 0;
	} else {
	    y = y1 + fabs(y1 - y2)/2.0;
	}

	// The stability of these operations should be improved
	phi0 = 0;
	twiceN = sin(phi1) + sin(phi2);
	n = twiceN /2.0;
	C = cos(phi1) * cos(phi1) + 2.0 * n * sin(phi1);
	rho0 = sqrt(C - 2.0 * n * sin(phi0)) / n;

	im->precomputedValue[0] = phi1;
	im->precomputedValue[1] = phi2;
	im->precomputedValue[2] = phi0;
	im->precomputedValue[3] = n;
	im->precomputedValue[4] = C;
	im->precomputedValue[5] = rho0;
	im->precomputedValue[6] = y;
	im->precomputedValue[7] = n*n;
	im->precomputedValue[8] = sin(phi1) + sin(phi2);
	im->precomputedValue[9] = twiceN;

	//	printf("Parms phi1 %f phi2 %f pho0 %f, n %f, C %f, rho0 %f, %f\n", 
	//	       phi1, phi2, phi0, n, C, rho0, y);

    }

    for (i=0;i<im->precomputedCount;i++) {
	assert(!isnan(im->precomputedValue[i]));
    }
    
    if (im->precomputedCount > 0) return 1;
    //    PrintError("false in alberts equal area parameters");
    return 0;
}

/** convert from erect to albersequalareaconic */
int albersequalareaconic_erect( double x_dest,double  y_dest, double* x_src, double* y_src, void *params)
{
    double yoffset, lambda, phi, lambda0, n, C, rho0, theta, rho;
    double twiceN;

    // Forward calculation


    if (!albersEqualAreaConic_ParamCheck(mp->pn))  {
	//	printf("REturning abert->erect 0\n");
	return 0;
    }

    assert(!isnan(x_dest));
    assert(!isnan(y_dest));

    lambda = x_dest / mp->distance;
    phi = y_dest / mp->distance;

    if (lambda > PI) lambda-=2*PI;
    if (lambda < -PI) lambda+=2*PI;

    lambda0 = 0;

    n = mp->pn->precomputedValue[3];
    C = mp->pn->precomputedValue[4];
    rho0 = mp->pn->precomputedValue[5];
    yoffset = mp->pn->precomputedValue[6];
    twiceN = mp->pn->precomputedValue[9];

    theta = n * (lambda - lambda0);

    
    //    printf("value %f\n", (phi));
    //    printf("value %f\n", sin(phi));
    //    printf("value %f\n", C - 2.0 * n * sin(phi));
    //assert(C - 2.0 * n * sin(phi) >=0);
    rho = sqrt(C - twiceN * sin(phi)) / n;

    *x_src = mp->distance * (rho * sin(theta));
    *y_src = mp->distance * (rho0 - rho * cos(theta) - yoffset);

    if (isnan(*x_src) ||
	isnan(*y_src)) {
	*x_src = 0;
	*y_src = 0;
	//	PrintError("false in alberts equal area 4");
	return 0;
    }

    assert(!isnan(*x_src));
    assert(!isnan(*y_src));

    return 1;
}

/** convert from albersequalareaconic to erect */
int erect_albersequalareaconic(double x_dest, double y_dest, double* x_src, double* y_src, void* params)
{
    double x, y, yoffset, lambda0, n, C, rho0, theta, phi, lambda, nsign;
    double rho2; // rho^2
    double n2; // n^2
    double twiceN; // n * 2.0
    
    //  Inverse calculation

    if (!albersEqualAreaConic_ParamCheck(mp->pn))  {
	*x_src = 0;
	*y_src = 0;
	//	printf("false in alberts equal area\n");
	return 0;
    }

    x = x_dest / mp->distance;
    y = y_dest / mp->distance;

    lambda0 = 0;

    n = mp->pn->precomputedValue[3];
    C = mp->pn->precomputedValue[4];
    rho0 = mp->pn->precomputedValue[5];
    yoffset = mp->pn->precomputedValue[6];
    n2 = mp->pn->precomputedValue[7];
    twiceN = mp->pn->precomputedValue[9];

    y = y + yoffset;

    rho2 = x*x + (rho0 - y)*(rho0 - y);
    nsign = 1.0;

    if (n < 0) nsign = -1.0;

    theta = atan2(nsign * x, nsign * (rho0 - y));

    phi = asin((C - rho2 * n2)/twiceN);

    lambda = lambda0 + theta / n;
    if (lambda > PI || lambda < -PI)  {
	*x_src = 0;
	*y_src = 0;
	//	PrintError("false in alberts equal area 2");
	return 0;
    }

    *x_src = mp->distance * lambda;
    *y_src = mp->distance * phi;

    if (isnan(*x_src) ||  
	isnan(*y_src)) {
	*x_src = 0;
	*y_src = 0;
	//	PrintError("false in alberts equal area 3");
	return 0;
    }

    assert(!isnan(*x_src));
    assert(!isnan(*y_src));

    return 1;
}

int albersequalareaconic_distance(double* x_src, void* params) {
    double x1, x2, y, phi1, phi2, lambda;

    //    printf("alber distance\n");

    if (!albersEqualAreaConic_ParamCheck(mp->pn))  {
	*x_src = 0;
	//	printf("false in alberts equal area distance 0\n");
	return 0;
    }

    mp->distance = 1;
    phi1 = mp->pn->precomputedValue[0];
    phi2 = mp->pn->precomputedValue[1];

    //lambda where x is a maximum.
    if (phi1 == phi2  &&
	phi1 == 0.0) {
	// THIS IS A HACK...it needs to further studied
	// why this when phi1==phi2==0 
	// this functions return 0
	// Avoid approximation error
	PrintError("The Albers projection cannot be used for phi1==phi2==0. Use Lambert Cylindrical Equal Area instead");

	*x_src = PI;
	return 0;
    }
    lambda = fabs(PI / (sin(phi1) + sin(phi2)));
    if (lambda > PI) lambda = PI;
    albersequalareaconic_erect(lambda, -PI/2.0, &x1, &y, mp);
    albersequalareaconic_erect(lambda, PI/2.0, &x2, &y, mp);
    *x_src = max(fabs(x1), fabs(x2));

    if (isnan(*x_src))  {
	*x_src = 0;
	PrintError("false in alberts equal area distance 1");
	return 0;
    }

    assert(!isnan(*x_src));

    //    printf("return albers distance %f\n", *x_src);

    return 1;

}


int sphere_cp_erect( double x_dest,double  y_dest, double* x_src, double* y_src, void* params)
{
	// params: double distanceparam, double b

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
	// params: double distanceparam

	register double phi, theta, r,s;
	double v[3];

	phi 	= x_dest / distanceparam;
	theta 	=  - y_dest / distanceparam  + PI / 2;
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

	theta = distanceparam * atan2( r , s * cos( phi ) );
	
	*x_src =  theta * v[0] / r;
	*y_src =  theta * v[1] / r;
    return 1;
}

int erect_sphere_cp( double x_dest,double  y_dest, double* x_src, double* y_src, void* params)
{
	// params: double distanceparam, double b

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
	// params: double distanceparam

	register double rho, theta,r;

#if 0	
	theta = sqrt( x_dest * x_dest + y_dest * y_dest ) / distanceparam;
	phi   = atan2( y_dest , x_dest );
	
	if( theta > PI /2.0  ||  theta < -PI /2.0 )
		theta = PI /2.0 ;

	rho = distanceparam * tan( theta );

	*x_src = rho * cos( phi );
	*y_src = rho * sin( phi );
#endif
	r 		= sqrt( x_dest * x_dest + y_dest * y_dest );
	theta 	= r / distanceparam;
	
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
	// params: double distanceparam

	register double  theta, r;

#if 0	
	theta = atan( sqrt(x_dest*x_dest + y_dest*y_dest) / *((double*)params));
	phi   = atan2( y_dest , x_dest );
	
	*x_src = *((double*)params) * theta * cos( phi );
	*y_src = *((double*)params) * theta * sin( phi );
#endif
	r 		= sqrt(x_dest*x_dest + y_dest*y_dest) / distanceparam;
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
	// params: double distanceparam

	register double r, s, Phi, theta;
	
#if 0
	register double Theta, phi;
	double v[3];
	
	Phi = x_dest / *((double*)params);
	Theta = PI /2.0 - atan( y_dest / distanceparam );
	

	v[2] = *((double*)params) * sin( Theta ) * cos( Phi );   //  x' -> z
	v[0] = *((double*)params) * sin( Theta ) * sin( Phi );	//  y' -> x
	v[1] = *((double*)params) * cos( Theta );				//  z' -> y
	
	theta = atan2( sqrt( v[0]*v[0] + v[1]*v[1] ) , v[2] );
	phi   = atan2( v[1], v[0] );
	
	*x_src = *((double*)params) * theta * cos( phi );
	*y_src = *((double*)params) * theta * sin( phi );
#endif
#if 1
	Phi = x_dest / distanceparam;
		
	s =  distanceparam * sin( Phi ) ;	//  y' -> x
	
	r = sqrt( s*s + y_dest*y_dest );
	theta = distanceparam * atan2( r , (distanceparam * cos( Phi )) ) / r;
	
	*x_src =  theta * s ;
	*y_src =  theta * y_dest ;
#endif
    return 1;
}

int pano_sphere_tp( double x_dest,double  y_dest, double* x_src, double* y_src, void* params)
{
	// params: double distanceparam
	register double r,s, theta;
	double v[3];

#if 0	
	theta = sqrt( x_dest * x_dest + y_dest * y_dest ) / distanceparam;
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
	theta = r / distanceparam;
	if( theta == 0.0 )
		s = 1.0 / distanceparam;
	else
		s = sin( theta ) /r;

	v[1] =  s * x_dest ;   //  x' -> y
	v[0] =  cos( theta );				//  z' -> x


	*x_src = distanceparam * atan2( v[1], v[0] );
	*y_src = distanceparam * s * y_dest / sqrt( v[0]*v[0] + v[1]*v[1] );

    return 1;
}


int sphere_cp_pano( double x_dest,double  y_dest, double* x_src, double* y_src, void* params)
{
	// params: double distanceparam

	register double phi, theta;
	
	
	phi 	= -x_dest / (distanceparam * PI / 2.0) ;
	theta	= PI /2.0 + atan( y_dest / (distanceparam * PI/2.0) );

	*x_src = distanceparam * theta * cos( phi );
	*y_src = distanceparam * theta * sin( phi );
    return 1;
}

int erect_rect( double x_dest,double  y_dest, double* x_src, double* y_src, void* params)
{
	// params: double distanceparam
#if 0
	theta = atan( sqrt(x_dest*x_dest + y_dest*y_dest) / distanceparam );
	phi   = atan2( y_dest , x_dest );


	v[1] = distanceparam * sin( theta ) * cos( phi );   //  x' -> y
	v[2] = distanceparam * sin( theta ) * sin( phi );	//  y' -> z
	v[0] = distanceparam * cos( theta );				//  z' -> x
	
	theta = atan2( sqrt( v[0]*v[0] + v[1]*v[1] ) , v[2] );
	phi   = atan2( v[1], v[0] );

	*x_src = distanceparam * phi;
	*y_src = distanceparam * (-theta + PI /2.0);
#endif

	*x_src = distanceparam * atan2( x_dest, distanceparam );
	*y_src = distanceparam * atan2(  y_dest, sqrt( distanceparam*distanceparam + x_dest*x_dest ) );

    return 1;
}


int erect_sphere_tp( double x_dest,double  y_dest, double* x_src, double* y_src, void* params)
{
	// params: double distanceparam

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
	theta = r / distanceparam;
	if(theta == 0.0)
		s = 1.0 / distanceparam;
	else
		s = sin( theta) / r;
	
	v[1] =  s * x_dest;   
	v[0] =  cos( theta );				
	

	*x_src = distanceparam * atan2( v[1], v[0] );
	*y_src = distanceparam * atan( s * y_dest /sqrt( v[0]*v[0] + v[1]*v[1] ) ); 
    return 1;
}

int mirror_sphere_cp( double x_dest,double  y_dest, double* x_src, double* y_src, void* params)
{
	// params: double distanceparam, double b

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
	// params: double distanceparam, double b, double b2

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
	// params: double distanceparam, double b

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
	// params: double distanceparam, double b

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



