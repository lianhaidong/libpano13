/* File PixMap.h Copyright (C) 1997 by John R. Montbriand.  All Rights Reserved. */

#ifndef __PIXMAP__
#define __PIXMAP__

/* File PixMap.h
	PixMap manipulation routines, definitions.
	Copyright (c) 1996, 1997 by John Montbriand.  All Rights Reserved.
	Permission hereby granted for public use.
    	Distribute freely in areas where the laws of copyright apply.
	USE AT YOUR OWN RISK.
	DO NOT DISTRIBUTE MODIFIED COPIES.
	Comments/questions/postcards* to the author at the address:
	  John Montbriand
	  P.O. Box. 1133
	  Saskatoon Saskatchewan Canada
	  S7K 3N2
	or by email at:
	  tinyjohn@sk.sympatico.ca
	*if you mail a postcard, then I will provide you with technical support
	regarding questions you may have about this file.

*/

#include <Types.h>
#include <QuickDraw.h>

#ifdef __cplusplus
extern "C" { 
#endif


/* version number for this interface (version 1) */

#define kPixMapVersion 1


/* MakePixMap creates a new pixmap handle with the requested
	dimensions.  If there is not enough memory to allocate the pixmap,
	MakePixMap will return NULL.  if clut is NULL, then the default
	256 colour table (clut = 8) is used.  if clut is not NULL, then a
	copy of it is used in the pixmap.  The number of colours in the clut
	determines the pixel depth of the created pixmap as follows:
		2 colours -> 1 pit per pixel,
		3 to 4 colours -> 2 bits per pixel,
		5 to 16 colours -> 4 bits per pixel,
		17 to 256 colours -> 8 bits per pixel. */

PixMapHandle MakePixMap(short width, short height, CTabHandle clut);


/* Make16BitPixMap creates a new pixmap handle with the requested
	dimensions.  If there is not enough memory to allocate the pixmap,
	Make16BitPixMap will return NULL.  The created pixmap utilizes 16 bit
	colour and allocates 16 bits per each pixel.  Make32BitPixMap is
	identical however it allocates 32 bits per pixel.  */

PixMapHandle Make16BitPixMap(short width, short height);
PixMapHandle Make32BitPixMap(short width, short height);


/* MakeScreenLikePixMap creates a pixmap mirroring the attributes of the
	pixmap used to draw largest area of *globalRect on the screen */ 
PixMapHandle MakeScreenLikePixMap(Rect *globalRect);


/* KillPixMap disposes of a pixmap allocated by one of the
	routines herein. */
void KillPixMap(PixMapHandle pix);


/* PixMapSize returns the number of bytes occupied by the pixmap. */
long PixMapSize(PixMapHandle pix);


/* SetPixMapPixel and GetPixMapPixel are for getting or setting
	individual pixel values.  */

void SetPixMapPixel(PixMapHandle pix, short h, short v, long value);
long GetPixMapPixel(PixMapHandle pix, short h, short v);


/* RotatePixRight creates a new pixmap containing the image
	stored in the parameter pixmap rotated 90 degrees to the right.
	The resulting pixmap is appropriately sized:  i.e. if the source
	pixmap is 100 pixels wide and 200 pixels tall,  then the result
	pixmap pointer will be 200 pixels wide and 100 pixels tall.  */
	
PixMapHandle RotatePixRight(PixMapHandle pix);


/* RotatePixLeft creates a new pixmap containing the image stored in
	the parameter bitmap pointer rotated 90 degrees to the left.  The
	resulting pixmap is appropriately sized:  i.e. if the source pixmap
	is 100 pixels wide and 200 pixels tall,  then the result pixmap
	pointer will be 200 pixels wide and 100 pixels tall.   */
	
PixMapHandle RotatePixLeft(PixMapHandle pix);


/* FlipPixVertical creates a new pixmap containing the image stored
	in the parameter pixmap flipped upside down.  The resulting
	pixmap will be the same size as the original image.  */
	
PixMapHandle FlipPixVertical(PixMapHandle pix);


/* FlipPixHorizontal creates a new pixmap containing the image stored
	in the parameter pixmap flipped horizontally. The resulting pixmap
	will be the same size as the original image.  */
	
PixMapHandle FlipPixHorizontal(PixMapHandle pix);


/* RotatePixMap creates a new pixmap containing the image from
	the parameter pixmap rotated angle degrees about the center (cx, cy).
	The resultant pixmap will have the same dimensions as the parameter
	pixmap regardless of the angle specified. */
	
PixMapHandle RotatePixMap(PixMapHandle pix, short cx, short cy, float angle);


/* DuplicatePixMap creates a copy of the pixmap parameter. */

PixMapHandle DuplicatePixMap(PixMapHandle pix);


/* PICTToBitMap creates a new pixmap the using the size information
	provided in the QuickDraw picture pointer parameter and draws
	the picture in the pixmap before returning the pixmap. */
	
PixMapHandle PICTToPixMap(PicHandle pic, CTabHandle clut);
PixMapHandle PICTTo16BitPixMap(PicHandle pic);
PixMapHandle PICTTo32BitPixMap(PicHandle pic);


/* PixMapToPICT returns a QuickDraw picture that will draw the
	image stored in the pixmap.  PixMapToPICT is the inverse
	of PICTToPixMap. */
	
PicHandle PixMapToPICT(PixMapHandle pix);


/* PixMapToCompressedPICT returns a QuickDraw picture that will draw the
	image stored in the pixmap.  PixMapToCompressedPICT is the inverse
	of PICTToPixMap. */
	
PicHandle PixMapToCompressedPICT(PixMapHandle pix);



/* ROUTINES FOR DRAWING TO... */

/* PixMapPort data structure managed by the routines NewBMP and
	DisposeBMP.  You should never have to access the fields of this
	record directly as it's all taken care of by NewBMP and DisposeBMP. */

typedef struct {
	CGrafPort pport;		/* the grafport record for drawing into the pixmap */
	GDHandle pdevice;		/* gdevice for drawing into the pixmap */
	GDHandle saved_device;	/* saved gdevice */
	GrafPtr saved_port;		/* saved grafport for later restoration */
	PixMapHandle pix;		/* the pixmap */
} PixMapPort;


/* NewBMP is called by the WithBitMap macro and you should never have to
	call it directly yourself.  What it does is it locks the pixmap, saves the
	original grafport, and creates a new grafport suitable for drawing into
	the pixmap.  NewBMP should be followed by a call to DisposeBMP which
	will restore the original grafport, unlock the pixmap, and dispose of the
	new grafport. */
	
PixMapPort* NewPxMP(PixMapHandle pix);



/* DisposePxMP unlocks the pixmap pointer passed to NewPxMP, deallocates
	the grafport allocated for it, and restores the original grafport.
	the macro WithBitMap calls this routine automatically and you will
	not normally have to call it directly. */
	
void DisposePxMP(PixMapPort* px);



/* WithPixMap is a macro facility that sets up the drawing environment
	such that any drawing commands in the statement following the
	macro instantiation will draw into the pixmap handle provided as
	the first parameter.  The parameters are declared as follows:
		PixMapHandle pix;
		PixMapPort* pxmp; */
	
#define WithPixMap(pix, pxmp) \
	for (pxmp = NewPxMP(pix); \
		pxmp != NULL; \
		DisposePxMP(pxmp), pxmp = NULL)




/* ROUTINES FOR DRAWING FROM... */

/* PlotPixMap provides a simple interface for drawing a pixmap in
	the current grafport.  The pixmap is drawn with the top left corner
	aligned with the point (h,v) using the indicated transfer mode. */

void PlotPixMap(PixMapHandle pix, short h, short v, short mode);


/* PixMapCopy copies bits from the pixmap to the current port
	from the src rectangle to the destination using the indicated
	copy mode. */

void PixMapCopy(PixMapHandle pix, Rect *src, Rect *dst, short mode);


/* PixMap2PixPat converts a pixmap to a pixel pattern resource.  The
	pixmap's dimensions should be some power of two
	(i.e. 8, 16, 32, 64, or 128). */

PixPatHandle PixMap2PixPat(PixMapHandle pix, Pattern *the_pattern);


/* CalcPixMapColours calculates a colour table for the colours
	used in the pixmap.  For indexed images, CalcPixMapColours returns
	a copy of the pixmap's colour table, otherwise, for direct
	images, it calls GetPixMapInfo. */

CTabHandle CalcPixMapColours(PixMapHandle pix);


#ifdef __cplusplus
};
#endif

#endif

/* end of File PixMap.h */




