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

// Added Fix of Kekus Kratzke: March.2004
// Radial Shift, when colors channels have different values and
// d is > 1 would give incorrect results around the edge of the image

// Modified by Fulvio Senore: June.2004
// Added linear interpolation between pixels in the geometric transform phase
// to speed up computation.
// Changes are bracketed between
//
// // FS+
//
// and
//
// // FS-
//
// comments



// Program specific includes

#include "filter.h" 			


// Standard C includes

#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>


// 			This file uses functions of type
// 	resample( unsigned char *dst, 	unsigned char **rgb,
//							register double Dx, 
//							register double Dy,
//							int color, int SamplesPerPixel);
//
// dst - output pixel
// rgb - input pixels, may be Lab as well.
// Dx  - offset of output pixel position in x-direction
// Dy  - offset of output pixel position in y-direction
// color = 0: all rgb colors; color = 1,2,3: one of r,g,b
// BytesPerPixel = 3,4. Using color != 0, any value should (?) work.



// Arrays used for Gamma correction


PTGamma glu; // Lookup table


// FS+
// used for fast pixel transform. It is the width of the starting step for linear interpolation
// a value of 0 disables the fast transform
int fastTransformStep = 0;
// FS-


// Some locally needed math functions

static double 	sinc		( double x );
static double 	cubic01		( double x );
static double 	cubic12		( double x ); 



// Interpolators

static void nn( unsigned char *dst, unsigned char **rgb, 
		register double Dx, register double Dy,	int color, int SamplesPerPixel);
		
static void bil( unsigned char *dst, unsigned char **rgb,  
		register double Dx, register double Dy,	int color, int SamplesPerPixel);
		
static void poly3( unsigned char *dst, unsigned char **rgb,  
		register double Dx, register double Dy,	int color, int SamplesPerPixel);
		
static void spline16( unsigned char *dst, unsigned char **rgb,  
		register double Dx, register double Dy,	int color, int SamplesPerPixel);
		
static void spline36( unsigned char *dst, unsigned char **rgb,  
		register double Dx, register double Dy,	int color, int SamplesPerPixel);
		
static void spline64( unsigned char *dst, unsigned char **rgb,  
		register double Dx, register double Dy,	int color, int SamplesPerPixel);

static void sinc256( unsigned char *dst, unsigned char **rgb,  
		register double Dx, register double Dy,	int color, int SamplesPerPixel);

static void sinc1024( unsigned char *dst, unsigned char **rgb,  
		register double Dx, register double Dy,	int color, int SamplesPerPixel);

static void nn_16( unsigned char *dst, unsigned char **rgb, 
		register double Dx, register double Dy,	int color, int SamplesPerPixel);
		
static void bil_16( unsigned char *dst, unsigned char **rgb,  
		register double Dx, register double Dy,	int color, int SamplesPerPixel);
		
static void poly3_16( unsigned char *dst, unsigned char **rgb,  
		register double Dx, register double Dy,	int color, int SamplesPerPixel);
		
static void spline16_16( unsigned char *dst, unsigned char **rgb,  
		register double Dx, register double Dy,	int color, int SamplesPerPixel);
		
static void spline36_16( unsigned char *dst, unsigned char **rgb,  
		register double Dx, register double Dy,	int color, int SamplesPerPixel);
		
static void spline64_16( unsigned char *dst, unsigned char **rgb,  
		register double Dx, register double Dy,	int color, int SamplesPerPixel);

static void sinc256_16( unsigned char *dst, unsigned char **rgb,  
		register double Dx, register double Dy,	int color, int SamplesPerPixel);

static void sinc1024_16( unsigned char *dst, unsigned char **rgb,  
		register double Dx, register double Dy,	int color, int SamplesPerPixel);






// Various interpolators; a[] is array of coeeficients; 0 <= x < 1


#define 	NNEIGHBOR(x, a , NDIM )											\
			a[0] = 1.0;	


#define 	BILINEAR(x, a, NDIM )											\
			a[1] = x;														\
			a[0] = 1.0 - x;	
			

// Unused; has been replaced by 'CUBIC'.

#define 	POLY3( x, a , NDIM )											\
			a[3] = (  x * x - 1.0) * x / 6.0;								\
			a[2] = ( (1.0 - x) * x / 2.0 + 1.0) * x; 						\
			a[1] = ( ( 1.0/2.0  * x - 1.0 ) * x - 1.0/2.0 ) * x + 1.0;		\
			a[0] = ( ( -1.0/6.0 * x + 1.0/2.0 ) * x - 1.0/3.0 ) * x ;



#define		SPLINE16( x, a, NDIM )											\
			a[3] = ( ( 1.0/3.0  * x - 1.0/5.0 ) * x -   2.0/15.0 ) * x;		\
			a[2] = ( ( 6.0/5.0 - x     ) * x +   4.0/5.0 ) * x;				\
			a[1] = ( ( x - 9.0/5.0 ) * x -   1.0/5.0     ) * x + 1.0;		\
			a[0] = ( ( -1.0/3.0 * x + 4.0/5.0     ) * x -   7.0/15.0 ) * x ;


#define		CUBIC( x, a, NDIM )											\
			a[3] = cubic12( 2.0 - x);									\
			a[2] = cubic01( 1.0 - x);									\
			a[1] = cubic01( x );										\
			a[0] = cubic12( x + 1.0);									\




#define		SPLINE36( x, a , NDIM )														\
	a[5] = ( ( -  1.0/11.0  * x +  12.0/ 209.0 ) * x +   7.0/ 209.0  ) * x;				\
	a[4] = ( (    6.0/11.0  * x -  72.0/ 209.0 ) * x -  42.0/ 209.0  ) * x;				\
	a[3] = ( ( - 13.0/11.0  * x + 288.0/ 209.0 ) * x + 168.0/ 209.0  ) * x;				\
	a[2] = ( (   13.0/11.0  * x - 453.0/ 209.0 ) * x -   3.0/ 209.0  ) * x + 1.0;		\
	a[1] = ( ( -  6.0/11.0  * x + 270.0/ 209.0 ) * x - 156.0/ 209.0  ) * x;				\
	a[0] = ( (    1.0/11.0  * x -  45.0/ 209.0 ) * x +  26.0/ 209.0  ) * x;



#define		SPLINE64( x, a , NDIM )														\
	a[7] = ((  1.0/41.0 * x -   45.0/2911.0) * x -   26.0/2911.0) * x;					\
	a[6] = ((- 6.0/41.0 * x +  270.0/2911.0) * x +  156.0/2911.0) * x;					\
	a[5] = (( 24.0/41.0 * x - 1080.0/2911.0) * x -  624.0/2911.0) * x;					\
	a[4] = ((-49.0/41.0 * x + 4050.0/2911.0) * x + 2340.0/2911.0) * x;					\
	a[3] = (( 49.0/41.0 * x - 6387.0/2911.0) * x -    3.0/2911.0) * x + 1.0;			\
	a[2] = ((-24.0/41.0 * x + 4032.0/2911.0) * x - 2328.0/2911.0) * x;					\
	a[1] = ((  6.0/41.0 * x - 1008.0/2911.0) * x +  582.0/2911.0) * x;					\
	a[0] = ((- 1.0/41.0 * x +  168.0/2911.0) * x -   97.0/2911.0) * x;					


#define		SINC( x, a, NDIM )										\
	{																\
		register int idx;											\
		register double xadd;										\
		for( idx = 0, xadd = NDIM / 2 - 1.0 + x; 					\
			 idx < NDIM / 2; 										\
			 xadd-=1.0)												\
		{															\
			a[idx++] = sinc( xadd ) * sinc( xadd / ( NDIM / 2 ));	\
		}															\
		for( xadd = 1.0 - x; 										\
			 idx < NDIM; 											\
			 xadd+=1.0)												\
		{															\
			a[idx++] = sinc( xadd ) * sinc( xadd / ( NDIM / 2 ));	\
		}															\
	}																\
		








// Set up the arrays for gamma correction

int SetUpGamma( double pgamma, int psize)
{
	int i;
	double gnorm, xg, rgamma = 1.0/pgamma;

	if( psize == 1 )
	{
		glu.ChannelSize 	=   256;
		glu.ChannelStretch 	=    16;
	}
	else if( psize == 2 )
	{
		glu.ChannelSize 	= 65536;
		glu.ChannelStretch 	= 	  4;
	}
	else
		return -1;

	glu.GammaSize = glu.ChannelSize * glu.ChannelStretch;
	
	glu.DeGamma 	= NULL;
	glu.Gamma  		= NULL;
	glu.DeGamma 	= (double*) 		malloc( glu.ChannelSize * sizeof( double ) );
	glu.Gamma  		= (unsigned short*) malloc( glu.GammaSize * sizeof( unsigned short) );
	
	if( glu.DeGamma == NULL || glu.Gamma == NULL )
	{
		PrintError("Not enough memory");
		return -1;
	}

	glu.DeGamma[0] = 0.0;
	gnorm = (glu.ChannelSize-1) / pow( glu.ChannelSize-1 , pgamma ) ; 
	for(i=1; i<glu.ChannelSize; i++)
	{
		glu.DeGamma[i] = pow( (double)i , pgamma ) * gnorm;
	}

	glu.Gamma[0] = 0;
	gnorm = (glu.ChannelSize-1) /  pow( glu.ChannelSize-1 , rgamma ) ; 
	if( psize == 1 )
	{
		for(i=1; i<glu.GammaSize; i++)
		{
			xg	 = pow(  ((double)i) / glu.ChannelStretch , rgamma ) * gnorm;
			DBL_TO_UC( glu.Gamma[i], xg );
		}
	}
	else
	{
		for(i=1; i<glu.GammaSize; i++)
		{
			xg	 = pow(  ((double)i) / glu.ChannelStretch , rgamma ) * gnorm;
			DBL_TO_US( glu.Gamma[i], xg );
		}
	}
	return 0;
}

unsigned short gamma_correct( double pix )
{
	int k = glu.ChannelStretch * pix;
	if( k < 0 )
		return 0;
	if( k > glu.GammaSize - 1 )
		return glu.ChannelSize - 1;
	return (glu.Gamma)[ k ] ;
}




/////////// N x N Sampler /////////////////////////////////////////////

#define RESAMPLE_N( intpol, ndim, psize )								\
	double yr[ndim], yg[ndim], yb[ndim], w[ndim];						\
	register double rd, gd, bd, weight ;								\
	register int k,i;													\
	register unsigned psize *r, *ri;									\
																		\
	intpol( Dx, w, ndim )												\
	if( color == 0 )													\
	{																	\
		for(k=0; k<ndim; k++)											\
		{																\
			r = ((unsigned psize**)rgb)[k]  + SamplesPerPixel - 3;		\
			rd = gd = bd = 0.0;											\
																		\
			for(i=0; i<ndim; i++)										\
			{															\
				weight = w[ i ];										\
				ri	   = r + i * SamplesPerPixel;						\
				rd += glu.DeGamma[(int)*ri++] * weight;					\
				gd += glu.DeGamma[(int)*ri++] * weight;					\
				bd += glu.DeGamma[(int)*ri]   * weight;					\
			}															\
			yr[k] = rd; yg[k] = gd; yb[k] = bd;							\
		}																\
																		\
		intpol( Dy, w, ndim )											\
		rd = gd = bd = 0.0;												\
																		\
		for(i=0; i<ndim; i++)											\
		{																\
			weight = w[ i ];											\
			rd += yr[i] * weight;										\
			gd += yg[i] * weight;										\
			bd += yb[i] * weight;										\
		}																\
		*((unsigned psize*)dst)++ 	= 	gamma_correct( rd );			\
		*((unsigned psize*)dst)++	= 	gamma_correct( gd );			\
		*((unsigned psize*)dst) 	=  	gamma_correct( bd );			\
	}																	\
	else																\
	{																	\
		color-=1;														\
		for(k=0; k<ndim; k++)											\
		{																\
			r = ((unsigned psize**)rgb)[k] + SamplesPerPixel - 3 + color;\
			yr[k] =  0.0;												\
																		\
			for(i=0; i<ndim; i++)										\
			{															\
				yr[k] += glu.DeGamma[(int)r[i*SamplesPerPixel]] * w[i];\
			}															\
		}																\
																		\
		intpol( Dy, w, ndim )											\
		rd = 0.0;														\
																		\
		for(i=0; i<ndim; i++)											\
		{																\
			rd += yr[i] * w[ i ];										\
		}																\
		 *((unsigned psize*)dst+color)  = 	gamma_correct( rd );		\
	}																	\




static double sinc( double x )
{
	x *= PI;
	if(x != 0.0) 
		return(sin(x) / x);
	return(1.0);
}


// Cubic polynomial with parameter A
// A = -1: sharpen; A = - 0.5 homogeneous
// make sure x >= 0
#define	A	(-0.75)

// 0 <= x < 1
static double cubic01( double x )
{
	return	(( A + 2.0 )*x - ( A + 3.0 ))*x*x +1.0;
}
// 1 <= x < 2

static double cubic12( double x )
{
	return	(( A * x - 5.0 * A ) * x + 8.0 * A ) * x - 4.0 * A;

}

#undef A





// ---------- Sampling functions ----------------------------------


// Nearest neighbor sampling, nowhere used (yet)

static void nn( unsigned char *dst, unsigned char **rgb, 
		register double Dx, register double Dy,	int color, int SamplesPerPixel)
		{
#pragma unused(Dx)
#pragma unused(Dy)
			RESAMPLE_N( NNEIGHBOR, 1, char)	}

// Bilinear sampling, nowhere used (yet).

static void bil( unsigned char *dst, unsigned char **rgb,  
		register double Dx, register double Dy,	int color, int SamplesPerPixel)
		{	RESAMPLE_N( BILINEAR, 2, char) 	}


// Lowest quality sampler in distribution; since version 1.8b1 changed to closely
// resemble Photoshop's bicubic interpolation

static void poly3( unsigned char *dst, unsigned char **rgb,  
		register double Dx, register double Dy,	int color, int SamplesPerPixel)
		{	RESAMPLE_N( CUBIC, 4, char) 	}


// Spline using 16 pixels; smoother and less artefacts than poly3, softer; same speed

static void spline16( unsigned char *dst, unsigned char **rgb,  
		register double Dx, register double Dy,	int color, int SamplesPerPixel)
		{	RESAMPLE_N( SPLINE16, 4, char) 	}

// Spline using 36 pixels; significantly sharper than both poly3 and spline16,
// almost no artefacts

static void spline36( unsigned char *dst, unsigned char **rgb,  
		register double Dx, register double Dy,	int color, int SamplesPerPixel)
		{	RESAMPLE_N( SPLINE36, 6, char) 	}

// Not used anymore

static void spline64( unsigned char *dst, unsigned char **rgb,  
		register double Dx, register double Dy,	int color, int SamplesPerPixel)
		{	RESAMPLE_N( SPLINE64, 8, char) 	}


// Highest quality sampler since version 1.8b1
// Extremely slow, but defintely worth every second.

static void sinc256( unsigned char *dst, unsigned char **rgb,  
		register double Dx, register double Dy,	int color, int SamplesPerPixel)
		{	RESAMPLE_N( SINC, 16, char) 	}
		

// Highest quality sampler since version 1.8b1
// Extremely slow, but defintely worth every second.

static void sinc1024( unsigned char *dst, unsigned char **rgb,  
		register double Dx, register double Dy,	int color, int SamplesPerPixel)
		{	RESAMPLE_N( SINC, 32, char) 	}


//--------------- Same as above, for shorts (16 bit channel size-------------------

// Nearest neighbor sampling, nowhere used (yet)

static void nn_16( unsigned char *dst, unsigned char **rgb, 
		register double Dx, register double Dy,	int color, int SamplesPerPixel)
		{
#pragma unused(Dx)
#pragma unused(Dy)
			RESAMPLE_N( NNEIGHBOR, 1, short)	}

// Bilinear sampling, nowhere used (yet).

static void bil_16( unsigned char *dst, unsigned char **rgb,  
		register double Dx, register double Dy,	int color, int SamplesPerPixel)
		{	RESAMPLE_N( BILINEAR, 2, short) 	}


// Lowest quality sampler in distribution; since version 1.8b1 changed to closely
// resemble Photoshop's bicubic interpolation

static void poly3_16( unsigned char *dst, unsigned char **rgb,  
		register double Dx, register double Dy,	int color, int SamplesPerPixel)
		{	RESAMPLE_N( CUBIC, 4, short) 	}


// Spline using 16 pixels; smoother and less artefacts than poly3, softer; same speed

static void spline16_16( unsigned char *dst, unsigned char **rgb,  
		register double Dx, register double Dy,	int color, int SamplesPerPixel)
		{	RESAMPLE_N( SPLINE16, 4, short) 	}

// Spline using 36 pixels; significantly sharper than both poly3 and spline16,
// almost no artefacts

static void spline36_16( unsigned char *dst, unsigned char **rgb,  
		register double Dx, register double Dy,	int color, int SamplesPerPixel)
		{	RESAMPLE_N( SPLINE36, 6, short) 	}

// Not used anymore

static void spline64_16( unsigned char *dst, unsigned char **rgb,  
		register double Dx, register double Dy,	int color, int SamplesPerPixel)
		{	RESAMPLE_N( SPLINE64, 8, short) 	}


// Highest quality sampler since version 1.8b1
// Extremely slow, but defintely worth every second.

static void sinc256_16( unsigned char *dst, unsigned char **rgb,  
		register double Dx, register double Dy,	int color, int SamplesPerPixel)
		{	RESAMPLE_N( SINC, 16, short) 	}
		

// Highest quality sampler since version 1.8b1
// Extremely slow, but defintely worth every second.

static void sinc1024_16( unsigned char *dst, unsigned char **rgb,  
		register double Dx, register double Dy,	int color, int SamplesPerPixel)
		{	RESAMPLE_N( SINC, 32, short) 	}
		

////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// FS+ start of functions used to compute the pixel tranform from dest to source using linear interpolation
////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// computes the source coordinates of a single pixel at position x using the math transforms
void ComputePixelCoords( double *ax, double *ay, int *trinum, char *avalid, long x, long offset, double w2, double y_d, 
						  fDesc *fD, double sw2, double sh2, double min_x, double max_x, double min_y, double max_y ) {
	double x_d, Dx, Dy;

	// Convert destination screen coordinates to cartesian coordinates.			
	x_d = (double) (x + offset) - w2;

	// Get source cartesian coordinates 
	fD->func( x_d, y_d , &Dx, &Dy, fD->param);

	// Convert source cartesian coordinates to screen coordinates 
	Dx += sw2;
	Dy =  sh2 + Dy ;

	// stores the computed pixel
	ax[x] = Dx;
	ay[x] = Dy;
	trinum[x] = getLastCurTriangle();

	// Is the pixel valid, i.e. from within source image?
	if( (Dx >= max_x)   || (Dy >= max_y) || (Dx < min_x) || (Dy < min_y)  )
		avalid[x] = FALSE;
	else
		avalid[x] = TRUE;
}

// fills a part of the arrays with the coordinates in the source image for every pixel
// xl is the left border of the array, xr is the right border. The array values have already been
//   computed in xl and xr.
void ComputePartialRowCoords( double *ax, double *ay, int *trinum, char *avalid, long xl, long xr, long offset, double w2, double y_d, 
						  fDesc *fD, double sw2, double sh2, double min_x, double max_x, double min_y, double max_y ) {
	long xm, idx;
	double srcX_lin, srcY_lin;
	double deltaX, deltaY, tmpX, tmpY;

	////////////////////////////////////////////
	// maximum estimated error to be accepted: higher values produce a faster execution but a more distorted image
	// the real maximum error seems to be much lower, about 1/4 of MAX_ERR
	double MAX_ERR = 1;

	if( xl >= (xr - 1) ) return;

	if( !avalid[xl] && !avalid[xr] ) {
		// first and last pixel are not valid, assume that others are not valid too
		// ax[] and ay[] values are not set since thay will not be used
		for( idx = xl + 1; idx < xr; idx++ ) {
			avalid[idx] = FALSE;
		}
		return;
	}

	// computes the source coords of the middle point of [xl, xr] using the transformation
	xm = (xl + xr)/2;
	ComputePixelCoords( ax, ay, trinum, avalid, xm, offset, w2, y_d, fD, sw2, sh2, min_x, max_x, min_y, max_y );
	// computes the coords of the same point with linear interpolation
	srcX_lin = ax[xl] + ((ax[xr] - ax[xl])/(xr - xl))*(xm - xl);
	srcY_lin = ay[xl] + ((ay[xr] - ay[xl])/(xr - xl))*(xm - xl);

	if( fabs(srcX_lin - ax[xm]) > MAX_ERR || fabs(srcY_lin - ay[xm]) > MAX_ERR ||
	    trinum[xl] != trinum[xr] || trinum[xl] != trinum[xm]) {
		// the error is still too large or the points are in different morph triangles: recursion
		ComputePartialRowCoords( ax, ay, trinum, avalid, xl, xm, offset, w2, y_d, fD, sw2, sh2, min_x, max_x, min_y, max_y );
		ComputePartialRowCoords( ax, ay, trinum, avalid, xm, xr, offset, w2, y_d, fD, sw2, sh2, min_x, max_x, min_y, max_y );
		return;
	}

	// fills the array, first the left half...
	if( !avalid[xl] || !avalid[xm] ) {
		// one end is valid and the other is not: computes every pixel with math transform
		for( idx = xl + 1; idx < xm; idx++ ) {
			ComputePixelCoords( ax, ay, trinum, avalid, idx, offset, w2, y_d, fD, sw2, sh2, min_x, max_x, min_y, max_y );
		}
	}
	else {
		// linear interpolation	
		deltaX = (ax[xm] - ax[xl]) / (xm - xl);
		deltaY = (ay[xm] - ay[xl]) / (xm - xl);
		tmpX = ax[xl];
		tmpY = ay[xl];
		for( idx = xl + 1; idx < xm; idx++ ) {
			tmpX += deltaX;
			tmpY += deltaY;
			ax[idx] = tmpX;
			ay[idx] = tmpY;
			if( (tmpX >= max_x)   || (tmpY >= max_y) || (tmpX < min_x) || (tmpY < min_y)  )
				avalid[idx] = FALSE;
			else
				avalid[idx] = TRUE;
			trinum[idx] = trinum[xl];
		}
	}

	// ...then the right half
	if( !avalid[xm] || !avalid[xr] ) {
		// one end is valid and the other is not: computes every pixel with math transform
		for( idx = xm + 1; idx < xr; idx++ ) {
			ComputePixelCoords( ax, ay, trinum, avalid, idx, offset, w2, y_d, fD, sw2, sh2, min_x, max_x, min_y, max_y );
		}
	}
	else {
		// linear interpolation	
		deltaX = (ax[xr] - ax[xm]) / (xr - xm);
		deltaY = (ay[xr] - ay[xm]) / (xr - xm);
		tmpX = ax[xm];
		tmpY = ay[xm];
		for( idx = xm + 1; idx < xr; idx++ ) {
			tmpX += deltaX;
			tmpY += deltaY;
			ax[idx] = tmpX;
			ay[idx] = tmpY;
			if( (tmpX >= max_x)   || (tmpY >= max_y) || (tmpX < min_x) || (tmpY < min_y)  )
				avalid[idx] = FALSE;
			else
				avalid[idx] = TRUE;
			trinum[idx] = trinum[xr];
		}
	}

}


// fills the arrays with the source coords computed using linear interpolation
// asize is the number of elements of the arrays
// the array elements lie in the interval [0, asize], the image elements in [destRect.left, destRect.right]: the offset parameter
//   is used for the conversion
void ComputeRowCoords( double *ax, double *ay, int *trinum, char *avalid, long asize, long offset, double w2, double y_d, 
						  fDesc *fD, double sw2, double sh2, double min_x, double max_x, double min_y, double max_y ) {

	// initial distance betwen correctly computed points. The distance will be reduced if needed.
//	int STEP_WIDTH = 40;
	int STEP_WIDTH = fastTransformStep;

	long x;

	x = 0;
	ComputePixelCoords( ax, ay, trinum, avalid, x, offset, w2, y_d, fD, sw2, sh2, min_x, max_x, min_y, max_y );
	x += STEP_WIDTH;
	while( x < asize ) {
		ComputePixelCoords( ax, ay, trinum, avalid, x, offset, w2, y_d, fD, sw2, sh2, min_x, max_x, min_y, max_y );
		ComputePartialRowCoords( ax, ay, trinum, avalid, x - STEP_WIDTH, x, offset, w2, y_d, fD, sw2, sh2, min_x, max_x, min_y, max_y );
		x += STEP_WIDTH;
	}
	// compute the last pixels, if any
	x -= STEP_WIDTH;
	if( x < asize - 1 ) {
		ComputePixelCoords( ax, ay, trinum, avalid, asize - 1, offset, w2, y_d, fD, sw2, sh2, min_x, max_x, min_y, max_y );
		ComputePartialRowCoords( ax, ay, trinum, avalid, x, asize - 1, offset, w2, y_d, fD, sw2, sh2, min_x, max_x, min_y, max_y );
	}

}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// FS- end of functions used to compute the pixel transform from dest to source using linear interpolation
////////////////////////////////////////////////////////////////////////////////////////////////////////////////



//    Main transformation function. Destination image is calculated using transformation
//    Function "func". Either all colors (color = 0) or one of rgb (color =1,2,3) are
//    determined. If successful, TrPtr->success = 1. Memory for destination image
//    must have been allocated and locked!

void transForm( TrformStr *TrPtr, fDesc *fD, int color){
	register int 		x, y;		// Loop through destination image
	register int     	i, k; 	 	// Auxilliary loop variables
	int 			skip = 0;	// Update progress counter
	unsigned char 		*dest,*src,*sry;// Source and destination image data
	register unsigned char 		*sr;	// Source  image data
	char*			progressMessage;// Message to be displayed by progress reporter
	char                	percent[8];	// Number displayed by Progress reporter
	int			valid;		// Is this pixel valid? (i.e. inside source image)
	long			coeff;		// pixel coefficient in destination image
	long			cy;		// rownum in destimage
	int			xc,yc;

	double 			x_d, y_d;	// Cartesian Coordinates of point ("target") in Destination image
	double 		  	Dx, Dy;		// Coordinates of target in Source image
	int 			xs, ys;	

	unsigned char		**rgb  = NULL, 
				*cdata = NULL;	// Image data handed to sampler

	double			max_x = (double) TrPtr->src->width; // Maximum x values in source image
	double			max_y = (double) TrPtr->src->height; // Maximum y values in source image
	double			min_x =  -1.0;//0.0; // Minimum x values in source image
	double			min_y =  -1.0;//0.0; // Minimum y values in source image

	int			mix	  = TrPtr->src->width - 1; // maximum x-index src
	int			mix2;
	int			miy	  = TrPtr->src->height - 1;// maximum y-index src
	int			miy2;

	// Variables used to convert screen coordinates to cartesian coordinates

		
	double 			w2 	= (double) TrPtr->dest->width  / 2.0 - 0.5;  // Steve's L
	double 			h2 	= (double) TrPtr->dest->height / 2.0 - 0.5;
	double 			sw2 = (double) TrPtr->src->width   / 2.0 - 0.5;
	double 			sh2 = (double) TrPtr->src->height  / 2.0 - 0.5;
	
	int			BytesPerLine	= TrPtr->src->bytesPerLine;
	int			BytesPerPixel, FirstColorByte, SamplesPerPixel, BytesPerSample;

	int			n, n2;		// How many pixels should be used for interpolation	
	intFunc 		intp; 		// Function used to interpolate
	int 			lu = 0;		// Use lookup table?
	int			wrap_x = FALSE;
	double			theGamma;	// gamma handed to SetUpGamma()

	//////////////////////////////////////////////////////////////////////////
	// FS+ variables used for linear interpolation of the pixel transform
	double *ax = NULL, *ay = NULL;	// source coordinates of each pixel in a row
	int *trinum = NULL;             // triangle number if morphing
	char *avalid = NULL;			// is the pixel valid?
	double maxErrX, maxErrY;
	long offset;
//	int useFastTransform;	// true if we will use the new fast pixel transformation
	int evaluateError;		// true if we want to write a file with the transformation errors
	// FS-
	//////////////////////////////////////////////////////////////////////////

	// Selection rectangle
	PTRect			destRect;
	if( TrPtr->dest->selection.bottom == 0 && TrPtr->dest->selection.right == 0 ){
		destRect.left 	= 0;
		destRect.right	= TrPtr->dest->width;
		destRect.top	= 0;
		destRect.bottom = TrPtr->dest->height;
	}else{
		memcpy( &destRect, &TrPtr->dest->selection, sizeof(PTRect) );
	}

	// FS+
	offset = -destRect.left;
	maxErrX = 0;
	maxErrY = 0;
	// FS-

	switch( TrPtr->src->bitsPerPixel ){
		case 64:	FirstColorByte = 2; BytesPerPixel = 8; SamplesPerPixel = 4; BytesPerSample = 2; break;
		case 48:	FirstColorByte = 0; BytesPerPixel = 6; SamplesPerPixel = 3; BytesPerSample = 2; break;
		case 32:	FirstColorByte = 1; BytesPerPixel = 4; SamplesPerPixel = 4; BytesPerSample = 1; break;
		case 24:	FirstColorByte = 0; BytesPerPixel = 3; SamplesPerPixel = 3; BytesPerSample = 1; break;
		case  8:	FirstColorByte = 0; BytesPerPixel = 1; SamplesPerPixel = 1; BytesPerSample = 1; break;
		default:	PrintError("Unsupported Pixel Size: %d", TrPtr->src->bitsPerPixel);
					TrPtr->success = 0;
					return;
	}
	
	// Set interpolator etc:
	switch( TrPtr->interpolator ){
		case _poly3:// Third order polynomial fitting 16 nearest pixels
			if( BytesPerSample == 1 ) intp = poly3; else intp = poly3_16;		
			n = 4;
			break;
		case _spline16:// Cubic Spline fitting 16 nearest pixels
			if( BytesPerSample == 1 ) intp = spline16; else intp = spline16_16;		
			n = 4;
			break;
		case _spline36:	// Cubic Spline fitting 36 nearest pixels
			if( BytesPerSample == 1 ) intp = spline36; else intp = spline36_16;		
			n = 6;
			break;
		case _spline64:	// Cubic Spline fitting 64 nearest pixels
			if( BytesPerSample == 1 ) intp = spline64; else intp = spline64_16;	
			n = 8;
			break;
		case _sinc256:	// sinc windowed to 256 (2*8)^2 pixels
			if( BytesPerSample == 1 ) intp = sinc256; else intp = sinc256_16;	
			n = 16;
			break;
		case _sinc1024:	// sinc windowed to 1024 (2*16)^2 pixels
			if( BytesPerSample == 1 ) intp = sinc1024; else intp = sinc1024_16;	
			n = 32;
			break;
		case _bilinear:	// Bilinear fit using 4 nearest points
			if( BytesPerSample == 1 ) intp = bil; else intp = bil_16;	
			n = 2;
			break;
		case _nn:// nearest neighbor fit using 4 nearest points
			if( BytesPerSample == 1 ) intp = nn; else intp = nn_16;	
			n = 1;
			break;
		default: 
			PrintError( "Invalid Interpolator selected" );
			TrPtr->success = 0;
			return;
	}

	// Set up arrays that hold color data for interpolators

	rgb 	= (unsigned char**) malloc( n * sizeof(unsigned char*) );
	cdata	= (unsigned char*)  malloc( n * n * BytesPerPixel * sizeof( unsigned char ) );
	
	
	if( rgb == NULL || cdata == NULL ){
		PrintError( "Not enough Memory" );
		TrPtr->success = 0;
		goto Trform_exit;
	}
		
	n2 = n/2 ;
	mix2 = mix +1 - n;
	miy2 = miy +1 - n;

	dest = *TrPtr->dest->data;
	src  = *TrPtr->src->data; // is locked

	if(TrPtr->mode & _show_progress){
		switch(color){
			case 0: progressMessage = "Image Conversion"; 	break;
			case 1:	switch( TrPtr->src->dataformat){
						case _RGB: 	progressMessage = "Red Channel"  ; break;
						case _Lab:	progressMessage = "Lightness" 	 ; break;
					} break;
			case 2:	switch( TrPtr->src->dataformat){
						case _RGB: 	progressMessage = "Green Channel"; break;
						case _Lab:	progressMessage = "Color A" 	 ; break;
					} break; 
			case 3:	switch( TrPtr->src->dataformat){
						case _RGB: 	progressMessage = "Blue Channel"; break;
						case _Lab:	progressMessage = "Color B" 	; break;
					} break; 
			default: progressMessage = "Something is wrong here";
		}
		Progress( _initProgress, progressMessage );
	}

	if(TrPtr->mode & _wrapX)
		wrap_x = TRUE;

	if( TrPtr->src->dataformat == _RGB )	// Gamma correct only RGB-images
		theGamma = TrPtr->gamma;
	else
		theGamma = 1.0;
	
	if( SetUpGamma( theGamma, BytesPerSample) != 0 ){
		PrintError( "Could not set up lookup table for Gamma Correction" );
		TrPtr->success = 0;
		goto Trform_exit;
	}

	// FS+ allocates the temporary arrays
	ax = (double *) malloc( (destRect.right - destRect.left + 20)*sizeof(double) );
	ay = (double *) malloc( (destRect.right - destRect.left + 20)*sizeof(double) );
	trinum = (int *) malloc( (destRect.right - destRect.left + 20)*sizeof(int) );
	avalid = (char *) malloc( (destRect.right - destRect.left + 20)*sizeof(char) );
	// opens the preference file to read options
	evaluateError = FALSE;
	if( fastTransformStep != 0 ) {
		FILE *fp;
		char buf[100];
		char *s;
		s = buf;
//		useFastTransform = FALSE;
		fp = fopen( "pano12_opt.txt", "rt" );
		if( fp != NULL ) {
			// parse the file
			s = fgets( s, 98, fp );
			while( !feof(fp) && buf != NULL ) {
				//s = strupr( buf );	commented out because it causes linking problems with the microsoft compiler
//				if( strncmp( s, "FAST_TRANSFORM", 14 )  == 0 )
//					useFastTransform = TRUE;
				if( strncmp( s, "EVALUATE_ERROR", 14 )  == 0 )
					evaluateError = TRUE;
				s = fgets( buf, 98, fp );
			}
			fclose( fp );
		}
	}
	// FS-

	for(y=destRect.top; y<destRect.bottom; y++){
		// Update Progress report and check for cancel every 5 lines.
		skip++;
		if( skip == 5 ){
			if(TrPtr->mode & _show_progress){	
				sprintf( percent, "%d", (int) (y * 100)/ TrPtr->dest->height);
				if( ! Progress( _setProgress, percent ) ){
					TrPtr->success = 0;
					goto Trform_exit;
				}
			}else{
				if( ! Progress( _idleProgress, 0) ){
					TrPtr->success = 0;
					goto Trform_exit;
				}
			}
			skip = 0;
		}
		
		// y-coordinate in dest image relative to center		
		y_d = (double) y - h2 ;
		cy  = (y-destRect.top) * TrPtr->dest->bytesPerLine;	
		
		// FS+ computes the transform for this row using linear interpolation
		if( fastTransformStep != 0 || evaluateError )
			ComputeRowCoords( ax, ay, trinum, avalid, destRect.right - destRect.left + 1, offset, w2, y_d, fD, sw2, sh2, min_x, max_x, min_y, max_y );
		// FS-

		for(x=destRect.left; x<destRect.right; x++){
			// Calculate pixel coefficient in dest image just once

			coeff = cy  + BytesPerPixel * (x-destRect.left);		

			// FS+
			if( fastTransformStep == 0 || evaluateError ) {
				// Convert destination screen coordinates to cartesian coordinates.			
				x_d = (double) x - w2 ;
				
				// Get source cartesian coordinates 
				fD->func( x_d, y_d , &Dx, &Dy, fD->param);

				// Convert source cartesian coordinates to screen coordinates 
				Dx += sw2;
				Dy =  sh2 + Dy ;
				
				if( evaluateError ) {
					valid = avalid[x];
				}
				else {
					// Is the pixel valid, i.e. from within source image?
					if( (Dx >= max_x)   || (Dy >= max_y) || (Dx < min_x) || (Dy < min_y)  )
						valid = FALSE;
					else
						valid = TRUE;
				}
			} else {
				Dx = ax[x];
				Dy = ay[x];
				valid = avalid[x];
			}
			// was:

			//// Convert destination screen coordinates to cartesian coordinates.			
			//x_d = (double) x - w2 ;
			//
			//// Get source cartesian coordinates 
			//fD->func( x_d, y_d , &Dx, &Dy, fD->param);

			//// Convert source cartesian coordinates to screen coordinates 
			//Dx += sw2;
			//Dy =  sh2 + Dy ;
			//

			//// Is the pixel valid, i.e. from within source image?
			//if( (Dx >= max_x)   || (Dy >= max_y) || (Dx < min_x) || (Dy < min_y)  )
			//	valid = FALSE;
			//else
			//	valid = TRUE;

			// FS-

			// Convert only valid pixels
			if( valid ){

				// FS+
				if( evaluateError ) {
					double errX, errY;
					errX = fabs( Dx - ax[x + offset] );
					errY = fabs( Dy - ay[x + offset] );
					if( errX > maxErrX )
						maxErrX = errX;
					if( errY > maxErrY )
						maxErrY = errY;
				}
				// FS-

				// Extract integer and fractions of source screen coordinates
				xc 	  =  (int)floor( Dx ) ; Dx -= (double)xc;
				yc 	  =  (int)floor( Dy ) ; Dy -= (double)yc;
				
				// if alpha channel marks valid portions, set valid 
				if(TrPtr->mode & _honor_valid)
				switch( FirstColorByte ){
					case 1:{
						int xn = xc, yn = yc;
						if( xn < 0 ) xn = 0; //  -1  crashes Windows
						if( yn < 0 ) yn = 0; //  -1  crashes Windows
						if( src[ yn * BytesPerLine + BytesPerPixel * xn] == 0 )
							valid = FALSE;
						}
						break;
					case 2:{
						int xn = xc, yn = yc;
						if( xn < 0 ) xn = 0; //  -1  crashes Windows
						if( yn < 0 ) yn = 0; //  -1  crashes Windows
						if( *((USHORT*)(src + yn * BytesPerLine + BytesPerPixel * xn)) == 0 )
							valid = FALSE;
						}
						break;
					default: break;
				}
			}
			
			if( valid ){	
				ys = yc +1 - n2 ; // smallest y-index used for interpolation
				xs = xc +1 - n2 ; // smallest x-index used for interpolation
					
				// y indices used: yc-(n2-1)....yc+n2
				// x indices used: xc-(n2-1)....xc+n2
					
				if( ys >= 0 && ys <= miy2 && xs >= 0 && xs <= mix2 ){  // all interpolation pixels inside image
					sry = src + ys * BytesPerLine + xs * BytesPerPixel;
					for(i = 0;  i < n;  i++, sry += BytesPerLine){
						rgb[i] = sry;
					}
				}else{ // edge pixels
					if( ys < 0 )
						sry = src;
					else if( ys > miy )
						sry = src + miy * BytesPerLine;
					else
						sry = src + ys  * BytesPerLine;
					
					for(i = 0; i < n; i++){	
						xs = xc +1 - n2 ; // smallest x-index used for interpolation
						if( wrap_x ){
							while( xs < 0 )  xs += (mix+1);
							while( xs > mix) xs -= (mix+1);
						}
						if( xs < 0 )
							 sr = sry;
						else if( xs > mix )
							sr = sry + mix *BytesPerPixel;
						else
							sr = sry + BytesPerPixel * xs;
					
						rgb[i] = cdata + i * n * BytesPerPixel;
						for(k = 0; k < n; k++ ){
							memcpy( &(rgb[i][k * BytesPerPixel]), sr, BytesPerPixel);
							xs++;
							if( wrap_x ){
								while( xs < 0 )  xs += (mix+1);
								while( xs > mix) xs -= (mix+1);
							}
							if( xs < 0 )
							 	sr = sry;
							else if( xs > mix )
								sr = sry + mix *BytesPerPixel;
							else
								sr = sry + BytesPerPixel * xs;
						}
						 ys++;
						 if( ys > 0 && ys <= miy )
						 	sry +=  BytesPerLine; 
					}

				}
				
				switch( FirstColorByte ){
					case 1: dest[ coeff++ ] = UCHAR_MAX;		 // Set alpha channel
							break;
					case 2: *((USHORT*)(dest + coeff)) = USHRT_MAX; coeff+=2;
							break;
					default: break;
				}
						
				intp( &(dest[ coeff ]), rgb, Dx, Dy, color, SamplesPerPixel ); 

			}else{  // not valid
				//Fix: Correct would use incorrect correction values if different factors were set for each color channel
				//Kekus.Begin: March.2004
				//was:
				//memset( &(dest[ coeff ]), 0 ,BytesPerPixel );
				//now:
				if(color)
				{
					char*   ptr = &(dest[ coeff ]);
					ptr += FirstColorByte + (color - 1)*BytesPerSample;
					memset( ptr, 0 , BytesPerSample ); //Kekus_test
				}	
				else
					memset( &(dest[ coeff ]), 0 ,BytesPerPixel ); //Kekus_test
				//Kekus.End: March.2004
			}

		}
	}

//	if(TrPtr->mode & _show_progress){
//		Progress( _disposeProgress, percent );
//	}	
	TrPtr->success = 1;


Trform_exit:
	if( rgb ) 		free( rgb );
	if( cdata ) 		free( cdata );
	if( glu.DeGamma )	free( glu.DeGamma ); 	glu.DeGamma 	= NULL;
	if( glu.Gamma )		free( glu.Gamma );	glu.Gamma 	= NULL;

	// FS+
	if( ax != NULL ) free( ax );
	if( ay != NULL ) free( ay );
	if( trinum != NULL ) free( trinum);
	if( avalid != NULL ) free( avalid );

	if( evaluateError ) {
		FILE *fp;
		fp = fopen( "Errors.txt", "a+t" );
		fprintf( fp, "%f  %d\n", maxErrX, destRect.top );
		fprintf( fp, "%f\n", maxErrY );
		fclose( fp );
	}
	// FS-

	return;
}
	
/*This function was added by Kekus Digital on 18/9/2002. This function takes the parameter 'imageNum' which repesents the index of the image that has to be converted.*/
void MyTransForm( TrformStr *TrPtr, fDesc *fD, int color, int imageNum){
	register int 		x, y;		// Loop through destination image
	register int     	i, k; 	 	// Auxilliary loop variables
	int 			skip = 0;	// Update progress counter
	unsigned char 		*dest,*src,*sry;// Source and destination image data
	register unsigned char 		*sr;	// Source  image data
	char			progressMessage[30];// Message to be displayed by progress reporter
	char                	percent[8];	// Number displayed by Progress reporter
	int			valid;		// Is this pixel valid? (i.e. inside source image)
	long			coeff;		// pixel coefficient in destination image
	long			cy;		// rownum in destimage
	int			xc,yc;

	double 			x_d, y_d;	// Cartesian Coordinates of point ("target") in Destination image
	double 		  	Dx, Dy;		// Coordinates of target in Source image
	int 			xs, ys;	

	unsigned char		**rgb  = NULL, 
				*cdata = NULL;	// Image data handed to sampler

	double			max_x = (double) TrPtr->src->width; // Maximum x values in source image
	double			max_y = (double) TrPtr->src->height; // Maximum y values in source image
	double			min_x =  -1.0;//0.0; // Minimum x values in source image
	double			min_y =  -1.0;//0.0; // Minimum y values in source image

	int			mix	  = TrPtr->src->width - 1; // maximum x-index src
	int			mix2;
	int			miy	  = TrPtr->src->height - 1;// maximum y-index src
	int			miy2;

	// Variables used to convert screen coordinates to cartesian coordinates

		
	double 			w2 	= (double) TrPtr->dest->width  / 2.0 - 0.5;  // Steve's L
	double 			h2 	= (double) TrPtr->dest->height / 2.0 - 0.5;
	double 			sw2 = (double) TrPtr->src->width   / 2.0 - 0.5;
	double 			sh2 = (double) TrPtr->src->height  / 2.0 - 0.5;
	
	int			BytesPerLine	= TrPtr->src->bytesPerLine;
	int			BytesPerPixel, FirstColorByte, SamplesPerPixel, BytesPerSample;

	int			n, n2;		// How many pixels should be used for interpolation	
	intFunc 		intp; 		// Function used to interpolate
	int 			lu = 0;		// Use lookup table?
	int			wrap_x = FALSE;
	double			theGamma;	// gamma handed to SetUpGamma()

	// Selection rectangle
	PTRect			destRect;
	if( TrPtr->dest->selection.bottom == 0 && TrPtr->dest->selection.right == 0 ){
		destRect.left 	= 0;
		destRect.right	= TrPtr->dest->width;
		destRect.top	= 0;
		destRect.bottom = TrPtr->dest->height;
	}else{
		memcpy( &destRect, &TrPtr->dest->selection, sizeof(PTRect) );
	}


	switch( TrPtr->src->bitsPerPixel ){
		case 64:	FirstColorByte = 2; BytesPerPixel = 8; SamplesPerPixel = 4; BytesPerSample = 2; break;
		case 48:	FirstColorByte = 0; BytesPerPixel = 6; SamplesPerPixel = 3; BytesPerSample = 2; break;
		case 32:	FirstColorByte = 1; BytesPerPixel = 4; SamplesPerPixel = 4; BytesPerSample = 1; break;
		case 24:	FirstColorByte = 0; BytesPerPixel = 3; SamplesPerPixel = 3; BytesPerSample = 1; break;
		case  8:	FirstColorByte = 0; BytesPerPixel = 1; SamplesPerPixel = 1; BytesPerSample = 1; break;
		default:	PrintError("Unsupported Pixel Size: %d", TrPtr->src->bitsPerPixel);
					TrPtr->success = 0;
					return;
	}
	
	// Set interpolator etc:
	switch( TrPtr->interpolator ){
		case _poly3:// Third order polynomial fitting 16 nearest pixels
			if( BytesPerSample == 1 ) intp = poly3; else intp = poly3_16;		
			n = 4;
			break;
		case _spline16:// Cubic Spline fitting 16 nearest pixels
			if( BytesPerSample == 1 ) intp = spline16; else intp = spline16_16;		
			n = 4;
			break;
		case _spline36:	// Cubic Spline fitting 36 nearest pixels
			if( BytesPerSample == 1 ) intp = spline36; else intp = spline36_16;		
			n = 6;
			break;
		case _spline64:	// Cubic Spline fitting 64 nearest pixels
			if( BytesPerSample == 1 ) intp = spline64; else intp = spline64_16;	
			n = 8;
			break;
		case _sinc256:	// sinc windowed to 256 (2*8)^2 pixels
			if( BytesPerSample == 1 ) intp = sinc256; else intp = sinc256_16;	
			n = 16;
			break;
		case _sinc1024:	// sinc windowed to 1024 (2*16)^2 pixels
			if( BytesPerSample == 1 ) intp = sinc1024; else intp = sinc1024_16;	
			n = 32;
			break;
		case _bilinear:	// Bilinear fit using 4 nearest points
			if( BytesPerSample == 1 ) intp = bil; else intp = bil_16;	
			n = 2;
			break;
		case _nn:// nearest neighbor fit using 4 nearest points
			if( BytesPerSample == 1 ) intp = nn; else intp = nn_16;	
			n = 1;
			break;
		default: 
			PrintError( "Invalid Interpolator selected" );
			TrPtr->success = 0;
			return;
	}

	// Set up arrays that hold color data for interpolators

	rgb 	= (unsigned char**) malloc( n * sizeof(unsigned char*) );
	cdata	= (unsigned char*)  malloc( n * n * BytesPerPixel * sizeof( unsigned char ) );
	
	
	if( rgb == NULL || cdata == NULL ){
		PrintError( "Not enough Memory" );
		TrPtr->success = 0;
		goto Trform_exit;
	}
		
	n2 = n/2 ;
	mix2 = mix +1 - n;
	miy2 = miy +1 - n;

	dest = *TrPtr->dest->data;
	src  = *TrPtr->src->data; // is locked

	if(TrPtr->mode & _show_progress){
		switch(color){
			case 0:
                        { 
                            char title[30];
#if BROKEN
                            int the_Num;
                            NumToString(imageNum, the_Num);
                            p2cstr(the_Num);
                            strcpy(title, "Converting Image #");
                            strcat(title, (char *)the_Num);
#else
                            sprintf(title, "Converting Image #%d", imageNum);
#endif
                            strcpy(progressMessage, title);	
                            //progressMessage = "Image Conversion"; 	
                        }
                        break;
			case 1:	switch( TrPtr->src->dataformat){
						case _RGB: 	strcpy(progressMessage,"Red Channel"); break;
						case _Lab:	strcpy(progressMessage, "Lightness"); break;
					} break;
			case 2:	switch( TrPtr->src->dataformat){
						case _RGB: 	strcpy(progressMessage, "Green Channel"); break;
						case _Lab:	strcpy(progressMessage, "Color A"); break;
					} break; 
			case 3:	switch( TrPtr->src->dataformat){
						case _RGB: 	strcpy(progressMessage, "Blue Channel"); break;
						case _Lab:	strcpy(progressMessage, "Color B"); break;
					} break; 
			default: strcpy(progressMessage, "Something is wrong here");
		}
		Progress( _initProgress, progressMessage );
	}

	if(TrPtr->mode & _wrapX)
		wrap_x = TRUE;

	if( TrPtr->src->dataformat == _RGB )	// Gamma correct only RGB-images
		theGamma = TrPtr->gamma;
	else
		theGamma = 1.0;
	
	if( SetUpGamma( theGamma, BytesPerSample) != 0 ){
		PrintError( "Could not set up lookup table for Gamma Correction" );
		TrPtr->success = 0;
		goto Trform_exit;
	}

	for(y=destRect.top; y<destRect.bottom; y++){
		// Update Progress report and check for cancel every 5 lines.
		skip++;
		if( skip == 5 ){
			if(TrPtr->mode & _show_progress){	
				sprintf( percent, "%d", (int) (y * 100)/ TrPtr->dest->height);
				if( ! Progress( _setProgress, percent ) ){
					TrPtr->success = 0;
					goto Trform_exit;
				}
			}else{
				if( ! Progress( _idleProgress, 0) ){
					TrPtr->success = 0;
					goto Trform_exit;
				}
			}
			skip = 0;
		}
		
		// y-coordinate in dest image relative to center		
		y_d = (double) y - h2 ;
		cy  = (y-destRect.top) * TrPtr->dest->bytesPerLine;	
		
		for(x=destRect.left; x<destRect.right; x++){
			// Calculate pixel coefficient in dest image just once

			coeff = cy  + BytesPerPixel * (x-destRect.left);		

			// Convert destination screen coordinates to cartesian coordinates.			
			x_d = (double) x - w2 ;
			
			// Get source cartesian coordinates 
			fD->func( x_d, y_d , &Dx, &Dy, fD->param);

			// Convert source cartesian coordinates to screen coordinates 
			Dx += sw2;
			Dy =  sh2 + Dy ;
			

			// Is the pixel valid, i.e. from within source image?
			if( (Dx >= max_x)   || (Dy >= max_y) || (Dx < min_x) || (Dy < min_y)  )
				valid = FALSE;
			else
				valid = TRUE;

			// Convert only valid pixels
			if( valid ){
				// Extract integer and fractions of source screen coordinates
				xc 	  =  (int)floor( Dx ) ; Dx -= (double)xc;
				yc 	  =  (int)floor( Dy ) ; Dy -= (double)yc;
				
				// if alpha channel marks valid portions, set valid 
				if(TrPtr->mode & _honor_valid)
				switch( FirstColorByte ){
					case 1:{
						int xn = xc, yn = yc;
						if( xn < 0 ) xn = 0; //  -1  crashes Windows
						if( yn < 0 ) yn = 0; //  -1  crashes Windows
						if( src[ yn * BytesPerLine + BytesPerPixel * xn] == 0 )
							valid = FALSE;
						}
						break;
					case 2:{
						int xn = xc, yn = yc;
						if( xn < 0 ) xn = 0; //  -1  crashes Windows
						if( yn < 0 ) yn = 0; //  -1  crashes Windows
						if( *((USHORT*)(src + yn * BytesPerLine + BytesPerPixel * xn)) == 0 )
							valid = FALSE;
						}
						break;
					default: break;
				}
			}
			
			if( valid ){	
				ys = yc +1 - n2 ; // smallest y-index used for interpolation
				xs = xc +1 - n2 ; // smallest x-index used for interpolation
					
				// y indices used: yc-(n2-1)....yc+n2
				// x indices used: xc-(n2-1)....xc+n2
					
				if( ys >= 0 && ys <= miy2 && xs >= 0 && xs <= mix2 ){  // all interpolation pixels inside image
					sry = src + ys * BytesPerLine + xs * BytesPerPixel;
					for(i = 0;  i < n;  i++, sry += BytesPerLine){
						rgb[i] = sry;
					}
				}else{ // edge pixels
					if( ys < 0 )
						sry = src;
					else if( ys > miy )
						sry = src + miy * BytesPerLine;
					else
						sry = src + ys  * BytesPerLine;
					
					for(i = 0; i < n; i++){	
						xs = xc +1 - n2 ; // smallest x-index used for interpolation
						if( wrap_x ){
							while( xs < 0 )  xs += (mix+1);
							while( xs > mix) xs -= (mix+1);
						}
						if( xs < 0 )
							 sr = sry;
						else if( xs > mix )
							sr = sry + mix *BytesPerPixel;
						else
							sr = sry + BytesPerPixel * xs;
					
						rgb[i] = cdata + i * n * BytesPerPixel;
						for(k = 0; k < n; k++ ){
							memcpy( &(rgb[i][k * BytesPerPixel]), sr, BytesPerPixel);
							xs++;
							if( wrap_x ){
								while( xs < 0 )  xs += (mix+1);
								while( xs > mix) xs -= (mix+1);
							}
							if( xs < 0 )
							 	sr = sry;
							else if( xs > mix )
								sr = sry + mix *BytesPerPixel;
							else
								sr = sry + BytesPerPixel * xs;
						}
						 ys++;
						 if( ys > 0 && ys <= miy )
						 	sry +=  BytesPerLine; 
					}

				}
				
				switch( FirstColorByte ){
					case 1: dest[ coeff++ ] = UCHAR_MAX;		 // Set alpha channel
							break;
					case 2: *((USHORT*)(dest + coeff)) = USHRT_MAX; coeff+=2;
							break;
					default: break;
				}
						
				intp( &(dest[ coeff ]), rgb, Dx, Dy, color, SamplesPerPixel ); 

			}else{  // not valid
				//Fix: Correct would use incorrect correction values if different factors were set for each color channel
				//Kekus.Begin: March.2004
				//was:
				//memset( &(dest[ coeff ]), 0 ,BytesPerPixel );
				//now:
				if(color)
				{
					char*   ptr = &(dest[ coeff ]);
					ptr += FirstColorByte + (color - 1)*BytesPerSample;
					memset( ptr, 0 , BytesPerSample ); //Kekus_test
				}	
				else
					memset( &(dest[ coeff ]), 0 ,BytesPerPixel ); //Kekus_test
				//Kekus.End: March.2004
			}

		}
	}

//	if(TrPtr->mode & _show_progress){
//		Progress( _disposeProgress, percent );
//	}	
	TrPtr->success = 1;


Trform_exit:
	if( rgb ) 		free( rgb );
	if( cdata ) 		free( cdata );
	if( glu.DeGamma )	free( glu.DeGamma ); 	glu.DeGamma 	= NULL;
	if( glu.Gamma )		free( glu.Gamma );	glu.Gamma 	= NULL;
	return;
}
	



#if 0

// An unused lookup version of sinc256
// Somewhat  faster on non-floating point machines like Intel

static double*   SetUpWeights(  );


// Weigths for sinc function

#define	NUM_WEIGHTS	256

static double *wt = NULL;




#define		SINC256( x, a )						\
	if( wt == NULL ) wt = SetUpWeights( );		\
	if( wt != NULL )							\
	{											\
		int xn = x * NUM_WEIGHTS;				\
		a[15]	= wt[ 8*NUM_WEIGHTS - xn ];		\
		a[14]	= wt[ 7*NUM_WEIGHTS - xn ];		\
		a[13]	= wt[ 6*NUM_WEIGHTS - xn ];		\
		a[12]	= wt[ 5*NUM_WEIGHTS - xn ];		\
		a[11]	= wt[ 4*NUM_WEIGHTS - xn ];		\
		a[10]	= wt[ 3*NUM_WEIGHTS - xn ];		\
		a[ 9]	= wt[ 2*NUM_WEIGHTS - xn ];		\
		a[ 8]	= wt[ 1*NUM_WEIGHTS - xn ];		\
		a[ 7]	= wt[ 0*NUM_WEIGHTS + xn ];		\
		a[ 6]	= wt[ 1*NUM_WEIGHTS + xn ];		\
		a[ 5]	= wt[ 2*NUM_WEIGHTS + xn ];		\
		a[ 4]	= wt[ 3*NUM_WEIGHTS + xn ];		\
		a[ 3]	= wt[ 4*NUM_WEIGHTS + xn ];		\
		a[ 2]	= wt[ 5*NUM_WEIGHTS + xn ];		\
		a[ 1]	= wt[ 6*NUM_WEIGHTS + xn ];		\
		a[ 0]	= wt[ 7*NUM_WEIGHTS + xn ];		\
	}											\

// Create Weights for A * NUM_WEIGHTS positions

static double* SetUpWeights(  )
{
#define A 	8
	int i,k,id;
	double dx = 1.0 / (double)NUM_WEIGHTS;
	double *w;
	
	w = (double*)malloc( A * NUM_WEIGHTS * sizeof(double) );
	if( w )
	{
		for( k=0; k < A ; k++ )
		{
			id = k * NUM_WEIGHTS;
			for( i=0; i<NUM_WEIGHTS; i++ )
			{
				w[id + i] = sinc8( (double)k + i*dx );
			}
		}
	}
	return w;
}
		
#undef A


#endif		




#if 0
/////////////// Results of calc for poly3  ////////////////////////////////////////////////////////////	

Equations:

1:  y0 = -a3 + a2 - a1 + a0 
2:  y1 = a0
3:  y2 = a3 + a2 + a1 + a0
4:  y3 = 8 a3 + 4 a2 + 2 a1 + a0


--- Emacs Calculator Mode ---
4:  a1 = y2 - y3 / 6 - y0 / 3 - y1 / 2
3:  6 a3 = y3 - y0 + 3 y1 - 3 y2
2:  2 a2 = y2 + y0 - 2 y1
1:  a0 = y1


/////////////// Results of Calc for Spline 16 ////////////////////////////////////////////////////////////	

1.  y0 = -c3 + c2 - c1 + c0
2.  y1 = c0
3.  y1 = a0
4.  y2 = a3 + a2 + a1 + a0
5.  y2 = b3 + b2 + b1 + b0
6.  y3 = 8 b3 + 4 b2 + 2 b1 + b0
7.  c1 = a1
8.  3 a3 + 2 a2 + a1 = 3 b3 + 2 b2 + b1
9.  2 c2 = 2 a2
10. 6 a3 + 2 a2 = 6 b3 + 2 b2
11. -6 c3 + 2 c2 = 0
12. 12 b3 + 2 b2 = 0

/////////////// Results of Calc for Spline 36 ////////////////////////////////////////////////////////////	

Equations:

--- Emacs Calculator Mode ---
20: 18 c3 + 2 c2 = 0
19: 2 e2 - 12 e3 = 0
18: 12 b3 + 2 b2 = 12 c3 + 2 c2
17: 6 a3 + 2 a2 = 6 b3 + 2 b2
16: 2 d2 = 2 a2
15: 2 e2 - 6 e3 = 2 d2 - 6 d3
14: 12 b3 + 4 b2 + b1 = 12 c3 + 4 c2 + c1
13: 3 a3 + 2 a2 + a1 = 3 b3 + 2 b2 + b1
12: d1 = a1
11: 3 e3 - 2 e2 + e1 = 3 d3 - 2 d2 + d1
10: 27 c3 + 9 c2 + 3 c1 + c0 = y5
9:  8 c3 + 4 c2 + 2 c1 + c0 = y4
8:  y4 = 8 b3 + 4 b2 + 2 b1 + b0
7:  y3 = b3 + b2 + b1 + b0
6:  y3 = a3 + a2 + a1 + a0
5:  y0 = 4 e2 - 8 e3 - 2 e1 + e0
4:  y1 = e2 - e3 - e1 + e0
3:  y1 = d2 - d3 - d1 + d0
2:  y2 = d0
1:  y2 = a0

--- Emacs Calculator Mode ---
4:  11 a3 = 6 y4 - y5 - 13 y3 - 6 y1 + y0 + 13 y2
3:  209 a1 
      = 7 y5 + 168 y3 - 42 y4 - 3 y2 + 26 y0 
          - 156 y1
2:  a2 = 12:209 y5 + 288:209 y3 - 72:209 y4 
           - 45:209 y0 - 453:209 y2 + 270:209 y1
1:  a0 = y2


/////////////// Results of Calc for Spline 64 ////////////////////////////////////////////////////////////	

Equations:

1:   y0 = -27 g3 + 9 g2 - 3 g1 + g0
2:   y1 = -8 g3 + 4 g2 - 2 g1 + g0
3:   y1 = -8 f3 + 4 f2 - 2 f1 + f0
4:   y2 = -f3 + f2 - f1 + f0
5:   y2 = -e3 + e2 - e1 + e0
6:   y3 = e0
7:   y3 = a0
8:   y4 = a3 + a2 + a1 + a0
9:   y4 = b3 + b2 + b1 + b0
10:  y5 = 8 b3 + 4 b2 + 2 b1 + b0
11:  y5 = 8 c3 + 4 c2 + 2 c1 + c0
12:  y6 = 27 c3 + 9 c2 + 3 c1 + c0
13:  y6 = 27 d3 + 9 d2 + 3 d1 + d0
14:  y7 = 64 d3 + 16 d2 + 4 d1 + d0
15:  12 g3 - 4 g2 + g1 = 12 f3 - 4 f2 + f1
16:  3 f3 - 2 f2 + f1 = 3 e3 - 2 e2 + e1
17:  e1 = a1
18   3 a3 + 2 a2 + a1 = 3 b3 + 2 b2 + b1
19   12 b3 + 4 b2 + b1 = 12 c3 + 4 c2 + c1
20   27 c3 + 6 c2 + c1 = 27 d3 + 6 d2 + d1
21   -12 g3 + 2 g2 = -12 f3 + 2 f2
22   -6 f3 + 2 f2 = -6 e3 + 2 e2 
23   2 e2 = 2 a2
24   6 a3 + 2 a2 = 6 b3 + 2 b2
25   12 b3 + 2 b2 = 12 c3 + 2 c2
26   18 c3 + 2 c2 = 18 d3 + 2 d2
27   -18 g3 + 2 g2 = 0
28   24 d3 + 2 d2 = 0

--- Emacs Calculator Mode ---
4:  41 a3 
      = y7 + 24 y5 - 6 y6 - 49 y4 - 24 y2 - y0 
          + 49 y3 + 6 y1
3:  a2 = 270:2911 y6 + 4050:2911 y4 - 45:2911 y7 
           - 1080:2911 y5 + 168:2911 y0 
           + 4032:2911 y2 - 1008:2911 y1 
           - 6387:2911 y3
2:  2911 a1 
      = 156 y6 + 2340 y4 - 26 y7 - 624 y5 + 582 y1 
          - 3 y3 - 2328 y2 - 97 y0
1:  a0 = y3



#endif


