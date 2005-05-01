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

static void 	ShiftImage(TrformStr *TrPtr, int xoff, int yoff);
static int 		getFrame( Image *im, int *xoff, int *yoff, int width, int height, int showprogress );



void 	correct	(TrformStr *TrPtr, cPrefs *prefs) 
{

	int 	i=0,j,k, kstart, kend, color;

	double 	scale_params[2];			// scaling factors for resize;
	double 	shear_params[2];			// shear values
	double	radial_params[6];			// coefficients for polynomial correction (0,...3)
										// and source width/2 (4) and correctionradius (5)
	double  lum_params[2];				// parameters to correct luminance variation	

	struct  fDesc stack[10];			// Parameters for execute stack function

	int		destwidth, destheight;
	int 	xoff = 0, yoff = 0;
	int		sizesqr;
	
	double  xdoff, ydoff;
	Image	im, *dest, *src;
	fDesc	fD;

	im.data = NULL; 

	src			= TrPtr->src;
	dest		= TrPtr->dest;

	// Apply filters, if required

	TrPtr->success = 1;
	
	if( prefs->luminance )
	{


		destwidth 		= TrPtr->src->width;
		destheight 		= TrPtr->src->height;

		// Allocate memory for destination image
		// If no further Xform: simply use SetDest
		
		if( !( 	prefs->resize 		||
				prefs->shear		||
				prefs->horizontal	||
				prefs->vertical 	||
				prefs->radial 		||
				prefs->cutFrame		||
				prefs->fourier)	)			
		{
			if( SetDestImage(TrPtr, destwidth, destheight) != 0 ) 
			{
				TrPtr->success = 0;
				PrintError( "Not enough Memory.");
				return;	
			}
		}
		else // Further Xform requested: use separate new image
		{
			memcpy( &im, TrPtr->src, sizeof(Image) );
			im.data = (unsigned char**) mymalloc( im.dataSize );
			if( im.data == NULL )
			{
				TrPtr->success = 0;
				PrintError( "Not enough Memory.");
				return;	
			}
			TrPtr->dest 	= &im;
		}
			
		// JMW 2003/08/25 Use the smaller of the width or height to 
		// calculate luminance so that the same change is made to portrait and 
		// landscape images.  
		// MRDL 2003/08/25 Makes behavior consistent with lens distortion correction
		// algorithm
        if( TrPtr->src->width < TrPtr->src->height )
            sizesqr = (TrPtr->src->width/2.0) * (TrPtr->src->width/2.0);
        else
            sizesqr = (TrPtr->src->height/2.0) * (TrPtr->src->height/2.0);

		if( prefs->lum_params[0] ==  prefs->lum_params[1] &&
			prefs->lum_params[1] ==  prefs->lum_params[2] )  // Color independent
		{
			lum_params[0] =  - prefs->lum_params[0] / sizesqr;
			lum_params[1] =    prefs->lum_params[0] / 2.0 ;
			if( TrPtr->success != 0 )
			{
				filter( TrPtr, radlum, radlum16, (void*) lum_params, 0 );
			}
		}
		else // Color dependent
		{
			for(k=1; k<4; k++)
			{	
				lum_params[0] =  - prefs->lum_params[k-1] / sizesqr;
				lum_params[1] =   prefs->lum_params[k-1] / 2.0 ;
				if( TrPtr->success != 0 )
				{
					filter( TrPtr, radlum, radlum16, (void*) lum_params, k );
				}
			}
		}
	}
	
	if( TrPtr->success	&&  prefs->fourier )	// Fourier filtering required
	{
		if( prefs->luminance )
		{
			CopyImageData( src, &im );
			TrPtr->src = src;
		}
		
		if( prefs->fourier_mode == _fresize )
		{
			if( prefs->width == 0 && prefs->height == 0 )
			{
				TrPtr->success = 0;
				PrintError( "Zero Destination Image Size" );
				return;
			}
			
			if( prefs->width  )
				destwidth 		= prefs->width;
			else
				destwidth 		= (double)TrPtr->src->width * (double)prefs->height / (double)TrPtr->src->height;
			
			if( prefs->height)
				destheight 		= prefs->height;
			else
				destheight		= (double)TrPtr->src->height * (double)prefs->width / (double)TrPtr->src->width;
		}
		else
		{
			destwidth 		= TrPtr->src->width;
			destheight 		= TrPtr->src->height;
		}

		// Allocate memory for destination image
		// If no further Xform: simply use SetDest
		
		if( !( 	prefs->resize 		||
				prefs->shear		||
				prefs->horizontal	||
				prefs->vertical 	||
				prefs->radial 		||
				prefs->cutFrame	)	)			
		{
			if( SetDestImage(TrPtr, destwidth, destheight) != 0 ) 
			{
				TrPtr->success = 0;
				PrintError( "Not enough Memory.");
				return;	
			}
		}
		else // Further Xform requested: use separate new image
		{
			if( prefs->luminance )  // since then we have allocated im already
			{
				if( im.data ) myfree( (void**)im.data );
			}
			memcpy( &im, TrPtr->src, sizeof(Image) );
			im.width = destwidth; im.height = destheight;
			im.bytesPerLine = im.width * im.bitsPerPixel/8;
			im.dataSize = im.height * im.bytesPerLine;
			im.data = (unsigned char**) mymalloc( im.dataSize );
			if( im.data == NULL )
			{
				TrPtr->success = 0;
				PrintError( "Not enough Memory.");
				return;
			}
			TrPtr->dest 	= &im;
		}
		
		fourier( TrPtr, prefs );
	}


		
		

	if( TrPtr->success		&&
			( 	prefs->resize 		||
				prefs->shear		||
				prefs->horizontal	||
				prefs->vertical 	||
				prefs->radial 		||
				prefs->cutFrame)	)			// Displacement Xform requested
	{

		// First check whether recent luminance or fourier filtering
		if( prefs->luminance || prefs->fourier )
			TrPtr->src	= &im;
		
		TrPtr->dest = dest;

		// Set destination image parameters
			
		// most Xforms: dest = src
		
		destwidth 		= TrPtr->src->width; 	destheight 		= TrPtr->src->height;

		
		if( prefs->cutFrame )
		{
			if( getFrame( TrPtr->src, &xoff, &yoff, prefs->fwidth, prefs->fheight, TrPtr->mode & _show_progress ) != 0 )
			{
				TrPtr->success = 0;
				return;
			}
			//PrintError("x= %d, y= %d", xoff, yoff);
			destwidth 		=  prefs->fwidth ; 	destheight 		=  prefs->fheight ;
		}
				

		if(prefs->resize)
		{
			if( prefs->width == 0 && prefs->height == 0 )
			{
				TrPtr->success = 0;
				PrintError( "Zero Destination Image Size" );
				return;
			}
			
			if( prefs->width  )
				destwidth 		= prefs->width;
			else
				destwidth 		= (double)TrPtr->src->width * (double)prefs->height / (double)TrPtr->src->height;
			
			if( prefs->height)
				destheight 		= prefs->height;
			else
				destheight		= (double)TrPtr->src->height * (double)prefs->width / (double)TrPtr->src->width;
		}
		
		if( destwidth <= 0 || destheight <= 0 )
		{
			TrPtr->success = 0;
			PrintError( "Zero Destination Image Size" );
			return;
		}



		// Allocate memory for destination image

		if( SetDestImage(TrPtr, destwidth, destheight) != 0 ) 
		{
			TrPtr->success = 0;
			PrintError( "Not enough Memory.");
			goto _correct_exit;
		}

		
		
		if( isColorSpecific( prefs ) )  // Color dependent
		{
			kstart 	= 1;
			kend	= 4;
		}
		else // Color independent
		{
			kstart	= 0;
			kend	= 1;
		}

		for(k=kstart;k<kend;k++)
		{
			color = k-1;
			if( color<0 ) color = 0;
			i = 0;

			if( prefs->resize )
			{
				if(prefs->cutFrame)
				{
					if( prefs->width )
						scale_params[0] = ((double)prefs->fwidth)/ prefs->width;
					else
						scale_params[0] = ((double)prefs->fheight)/ prefs->height;

					if( prefs->height )
						scale_params[1] = ((double)prefs->fheight)/ prefs->height;
					else
						scale_params[1] = scale_params[0];
				}
				else
				{
					if( prefs->width )
						scale_params[0] = ((double)TrPtr->src->width)/ prefs->width;
					else
						scale_params[0] = ((double)TrPtr->src->height)/ prefs->height;
						
					if( prefs->height )
						scale_params[1] = ((double)TrPtr->src->height)/ prefs->height;
					else
						scale_params[1] = scale_params[0];
				}
				SetDesc(stack[i],resize,scale_params); i++;
			}

			if( prefs->shear )
			{
				shear_params[0] = prefs->shear_x / TrPtr->src->height;
				shear_params[1] = prefs->shear_y / TrPtr->src->width;
				SetDesc(stack[i],shear,shear_params); i++;
			}

			if (prefs->horizontal)
			{
				SetDesc(stack[i],horiz,&(prefs->horizontal_params[color]) ); i++;
			}

			if (prefs->vertical)
			{
				SetDesc(stack[i],vert,&(prefs->vertical_params[color])); i++;
			}


			if( prefs->radial )
			{
				switch( prefs->correction_mode)
				{
					case correction_mode_radial:    SetDesc(stack[i],radial, radial_params); i++;
												    radial_params[4] = ( (double)( TrPtr->src->width < TrPtr->src->height ?
													   TrPtr->src->width : TrPtr->src->height) ) / 2.0;
												    break;
					case correction_mode_vertical:	SetDesc(stack[i],vertical, radial_params); i++;
													radial_params[4] = ((double)TrPtr->src->height) / 2.0;
													break;
					case correction_mode_deregister:SetDesc(stack[i],deregister, radial_params); i++;
													radial_params[4] = ((double)TrPtr->src->height) / 2.0;
													break;
				}
				for(j=0;j<4;j++)
					radial_params[j] = prefs->radial_params[color][j];
				radial_params[5] = prefs->radial_params[color][4];
			}
			if( prefs->cutFrame )
			{
				if( xoff != 0 )
				{
					xdoff = (double) xoff + 0.5 * ( prefs->fwidth - TrPtr->src->width ) ;
					SetDesc(stack[i],horiz, &xdoff ); i++;
				}

				if( yoff != 0 )
				{
					ydoff = (double)yoff + 0.5 * ( prefs->fheight - TrPtr->src->height) ;
					SetDesc(stack[i],vert,&ydoff); i++;
				}
				

			}
			stack[i].func = (trfn)NULL;

			if( 	!prefs->resize 		&&
					!prefs->shear		&&
					!prefs->horizontal	&&
					!prefs->vertical 	&&
					!prefs->radial 		&&
					prefs->cutFrame )	// Only cutframe
			{
					ShiftImage(TrPtr, xoff, yoff);
			}
			else if( TrPtr->success != 0 && i != 0 )
			{
				fD.func = execute_stack; fD.param = stack;
				transForm( TrPtr,  &fD, k);
			}

		}
	}
	

	if( !prefs->luminance && !prefs->fourier && !prefs->cutFrame && i == 0 ) // We did nothing!
	{
		TrPtr->success = 0;
	}	

	if( TrPtr->success == 0 && ! (TrPtr->mode & _destSupplied))
		myfree( (void**)TrPtr->dest->data );

_correct_exit:
		
	TrPtr->src 		= src;
	TrPtr->dest 	= dest;


	if( im.data != NULL )
		myfree((void**)im.data);
		

}



int 	cutTheFrame	( Image *dest, Image *src, int width, int height, int showprogress )
{
	int xoff, yoff;
	
	if( width > src->width || height > src->height )
	{
		PrintError("Image smaller than Rectangle to cut");
		return -1;
	}

	if( getFrame( src, &xoff, &yoff, width, height, showprogress ) == 0 )
	{
		TrformStr	TrCrop;
		
		memcpy( dest, src, sizeof( Image ) );
		dest->width 			= width;
		dest->height 			= height;
		dest->bytesPerLine 		= dest->width * dest->bitsPerPixel/8;
		dest->dataSize 			= dest->height * dest->bytesPerLine ;
		dest->data 				= (unsigned char**) mymalloc( dest->dataSize );
		if( dest->data == NULL )
		{
			PrintError("Could not allocate %ld bytes", dest->dataSize );
			return -1;
		}
		TrCrop.src 	= src;
		TrCrop.dest = dest;
		TrCrop.success = 0;
		
		
		ShiftImage( &TrCrop, xoff, yoff);
		if( TrCrop.success == 1 )
		{
			return 0;
		}
		else
		{
			myfree( (void**)dest->data );
			return -1;
		}
	}
	else
		return -1;
}




static int getFrame( Image *im, int *xoff, int *yoff, int width, int height, int showprogress )
{
	int xul, yul,x,y;
	int xm=0,ym=0;
	double obr = 0.0, br,brx;
	int dy = im->height - height;
	int dx = im->width - width;
	int bpp = im->bitsPerPixel/8;
	int fcb = bpp - 3;
	register unsigned char *sr;
	unsigned char *sry, *srx, *sl, *st, *sb;
	register double result = 0.0;
	char  percent[8];		// Number displayed by Progress reporter
	int 			skip = 0;	// Update progress counter
	
	if(height > im->height || width > im->width)
	{
		PrintError("Cut Frame: Wrong Parameters");
		return -1;
	}

	sry = *(im->data);
	
	if( showprogress )
		Progress( _initProgress, "Finding brightest rectangle" );

	result = 0.0;	
	// Get upper left rectangle
	for( y = 0; y < height; y++)
	{
		sr = sry + y * im->bytesPerLine;
		for( x= 0; x < width; x++ )
		{
			sr += fcb;
			result += *(sr++);
			result += *(sr++);
			result += *(sr++);
		}
	}

	brx = br = obr = result;

	srx = sry;

	for( xul = 0; xul <= dx; srx += bpp)
	{
		// Update Progress report and check for cancel every 2%.
		skip++;
		if( skip == (int)ceil(dx/50.0) ){
			if( showprogress )
			{	
				sprintf( percent, "%d", (int) (xul * 100)/(dx>0?dx:1));
				if( ! Progress( _setProgress, percent ) )
				{
					return -1;	
				}
			}
			else
			{				
				if( ! Progress( _idleProgress, 0) )
				{
						return -1;
				}
			}
			skip = 0;
		}

		br = brx;
		st = srx;
		
		for( yul = 0; yul <= dy;  yul++)
		{
			if( br > obr )
			{
				obr = br;
				xm = xul;
				ym = yul;
			}
			if( yul < dy )			// subtract top row, add bottom row
			{
				st = srx + yul * im->bytesPerLine;
				sb = st + height * im->bytesPerLine;
				for( x = 0; x < width; x++)
				{
			 		st += fcb;
					br  -= *(st++);
					br  -= *(st++);
					br  -= *(st++);
					
					sb += fcb;
					br  += *(sb++);
					br  += *(sb++);
					br  += *(sb++);
				}
			}
		}
		
		xul++;
		if( xul < dx )			// subtract left column, add right column
		{
			sl = srx + fcb;
			sr = srx + width * bpp + fcb;
			for( y = 0; y < height; y++, sl += im->bytesPerLine, sr += im->bytesPerLine)
			{
				brx -= sl[0];
				brx -= sl[1];
				brx -= sl[2];
				brx += sr[0];
				brx += sr[1];
				brx += sr[2];
			}
		}
	}
	
	*xoff = xm;
	*yoff = ym;
	if(showprogress)
	{
		Progress( _disposeProgress, percent );
	}	
	return 0;

}

static void ShiftImage(TrformStr *TrPtr, int xoff, int yoff)
{
	register int x,y;
	int cdy, csy;
	unsigned char *dest, *src;
	int bpp = TrPtr->src->bitsPerPixel/8;
	int BitsPerChannel,channels,fcb;

	GetBitsPerChannel( TrPtr->src, BitsPerChannel );
	GetChannels( TrPtr->src, channels);
	fcb = channels-3;
	
	// Some checks:
	
	if( TrPtr->dest->width + xoff > TrPtr->src->width ||
	    TrPtr->dest->height + yoff > TrPtr->src->height ||
		TrPtr->src->bitsPerPixel != TrPtr->dest->bitsPerPixel )
	{
		PrintError( "Parameter Error");
		TrPtr->success = 0;
		return;
	}
	
	dest = *(TrPtr->dest->data);
	src  = *(TrPtr->src->data);

	if( BitsPerChannel == 8 )
	{
		unsigned char *d,*s;
		for(y=0; y<TrPtr->dest->height; y++)
		{
			cdy = y * TrPtr->dest->bytesPerLine;
			csy = (y+yoff) * TrPtr->src->bytesPerLine;
			for(x=0; x<TrPtr->dest->width; x++)
			{
				d = dest + cdy + x*bpp;
				s = src  + csy + (x+xoff)*bpp;
				if(fcb)
				{
					*(d++) = *(s++);
				}
				*(d++) = *(s++);
				*(d++) = *(s++);
				*(d++) = *(s++);
			}
		}
	}
	else // 16
	{
		USHORT *dus, *sus;
		for(y=0; y<TrPtr->dest->height; y++)
		{
			cdy = y * TrPtr->dest->bytesPerLine;
			csy = (y+yoff) * TrPtr->src->bytesPerLine;
			for(x=0; x<TrPtr->dest->width; x++)
			{
				dus = (USHORT *)(dest + cdy + x*bpp);
				sus = (USHORT *)(src  + csy + (x+xoff)*bpp);
				if(fcb)
				{
					*(dus++) = *(sus++);
				}
				*(dus++) = *(sus++);
				*(dus++) = *(sus++);
				*(dus++) = *(sus++);
			}
		}
	}
	
	TrPtr->success = 1;
}

void SetCorrectDefaults( cPrefs *prefs )
{
	int i,k;
			
	prefs->magic 				= 20L; // from ANSI-version
	prefs->radial 				= FALSE;
	prefs->vertical = prefs->horizontal 	= FALSE;
	for(i=0;i<3;i++){
		prefs->radial_params[i][0] 	= 1.0;
		prefs->radial_params[i][4] 	= 1000.0; // Correction radius
		prefs->vertical_params[i] 	= 0.0;
		prefs->horizontal_params[i] 	= 0.0;
		for(k=1;k<4;k++)
			prefs->radial_params[i][k] = 0.0;		
		prefs->lum_params[i]		= 0.0;
	}
	prefs->shear 	= prefs->resize 	= FALSE;
	prefs->shear_x 	= prefs->shear_y 	= 0.0;
	prefs->width 	= prefs->height 	= 0;
	prefs->luminance			= FALSE;
	prefs->correction_mode			= correction_mode_radial;
	prefs->cutFrame				= FALSE;
	prefs->fwidth				= 100;
	prefs->fheight				= 100;
	prefs->frame				= 0;
	prefs->fourier				= 0;
	prefs->fourier_mode			= _fremoveBlurr;
	prefs->fourier_nf			= _nf_internal;
	memset( &(prefs->psf), 0, sizeof( fullPath ));
	memset( &(prefs->nff), 0, sizeof( fullPath ));
	prefs->filterfactor			= 1.0;
	prefs->fourier_frame			= 0.0;
}

// Check if colorspecific correction requested

int isColorSpecific( cPrefs *cp )
{
	int result = FALSE;
	int i;

	if( cp->radial )
	{
		for( i=0; i<4; i++ )
		{
			if( cp->radial_params[0][i] != cp->radial_params[1][i] || cp->radial_params[1][i] != cp->radial_params[2][i] )
				result = TRUE;
		}
	}

	if( cp->vertical )
	{
		if( cp->vertical_params[0] != cp->vertical_params[1] || cp->vertical_params[1] != cp->vertical_params[2])
				result = TRUE;
	}

	if( cp->horizontal )
	{
		if( cp->horizontal_params[0] != cp->horizontal_params[1] || cp->horizontal_params[1] != cp->horizontal_params[2])
				result = TRUE;
	}
	
	return result;
}

// Set all color dependent values to color 0

void SetEquColor( cPrefs *cP )
{
	int col,i;
	
	for(col = 1; col < 3; col++)
	{
		for(i=0; i<4; i++)
			cP->radial_params[col][i] 	= cP->radial_params[0][i];
		cP->vertical_params	[col] 		= cP->vertical_params[0];
		cP->horizontal_params[col] 		= cP->horizontal_params[0];
	}
}


// Restrict radial correction to monotonous interval

void SetCorrectionRadius( cPrefs *cP )
{
	double a[4];
	int i,k;
	
	for( i=0; i<3; i++ )
	{
		for( k=0; k<4; k++ )
		{
			a[k] = 0.0;//1.0e-10;
			if( cP->radial_params[i][k] != 0.0 )
			{
				a[k] = (k+1) * cP->radial_params[i][k];
			}
		}
		cP->radial_params[i][4] = smallestRoot( a );
	}
}



