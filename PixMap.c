/* File PixMap.c
	PixMap manipulation routines, implementation.
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


#include "PixMap.h"
#include <Memory.h>
#include <OSUtils.h>
#include <ToolUtils.h>
#include <FixMath.h>
#include <Math.h>
#include <Gestalt.h>
#include <ImageCompression.h>
#include <PictUtils.h>
#include <String.h>


PixMapHandle MakePixMap(short width, short height, CTabHandle clut) {
	long row_bytes, depth;
	Ptr raster_data;
	long bytecount;
	CTabHandle colours;
	PixMapHandle newpix;
	PixMapPtr pixptr;
	
		/* calculate required depth */
	if (clut == NULL)
		depth = 8;
	else if ((*clut)->ctSize < 2)
		depth = 1;
	else if ((*clut)->ctSize < 4)
		depth = 2;
	else if ((*clut)->ctSize < 16)
		depth = 4;
	else depth = 8;
	
		/* set up initial values for locals */
	row_bytes = ((depth * ((long) width) + 31) >> 5) << 2;
	bytecount = row_bytes * ((long) height);
	colours = NULL;
	raster_data = NULL;
	newpix = NULL;
	
		/* check for valid rowbytes */
	if (row_bytes > 0x00003FFF) goto bail;

		/* allocate the raster data */
	raster_data = NewPtrClear(bytecount);
	if (raster_data == NULL) goto bail;

		/* copy the colour table */
	if (clut == NULL) {
		colours = GetCTable(8);
		if (colours == NULL) goto bail;
	} else {
		colours = clut;
		if (HandToHand((Handle*) &colours) != noErr) {
			colours = NULL;
			goto bail;
		}
	}
		/* create the pixmap handle */
	newpix = (PixMapHandle) NewHandleClear(sizeof(PixMap));
	if (newpix == NULL) goto bail;
	
		/* set up the pixmap fields */
	pixptr = *newpix;
	pixptr->baseAddr = raster_data;
	pixptr->rowBytes = (row_bytes | 0x8000);
	pixptr->bounds.right = width;
	pixptr->bounds.bottom = height;
	pixptr->hRes = 0x00480000; /* resolution = 72 dots per inch */
	pixptr->vRes = 0x00480000;
	pixptr->pixelSize = depth;
	pixptr->cmpCount = 1;
	pixptr->cmpSize = depth;
	pixptr->pmTable = colours;

		/* done */
	return newpix;

bail:
	if (raster_data != NULL) DisposePtr(raster_data);
	if (newpix != NULL) DisposeHandle((Handle) newpix);
	if (colours != NULL) DisposeHandle((Handle) colours);
	return NULL;
}

PixMapHandle Make16BitPixMap(short width, short height) {
	long row_bytes;
	Ptr raster_data;
	long bytecount, response;
	CTabHandle colours;
	PixMapHandle newpix;
	PixMapPtr pixptr;
	unsigned char *rowptr;
	unsigned short *pixelptr;
	short y, x;

		/* is it possible? */
	if (Gestalt(gestaltQuickdrawVersion, &response) != noErr) response = 0;
	if (response < gestalt32BitQD) return NULL;

		/* set up initial values for locals */
	row_bytes = ((16 * ((long) width) + 31) >> 5) << 2;
	bytecount = row_bytes * ((long) height);
	colours = NULL;
	raster_data = NULL;
	newpix = NULL;
	
		/* check for valid rowbytes */
	if (row_bytes > 0x00003FFF) goto bail;
	
		/* allocate the raster data */
	raster_data = NewPtr(bytecount);
	if (raster_data == NULL) goto bail;
	rowptr = (unsigned char *) raster_data;
	for (y=0; y< height; y++, rowptr += row_bytes)
		for (pixelptr = (unsigned short *) rowptr, x=0; x<width; x++)
			*pixelptr++ = 0x7FFF;

		/* dummy colour table */
	colours = (CTabHandle) NewHandleClear(8);
	if (colours == NULL) goto bail;
	(*colours)->ctSeed = 15;
	(*colours)->ctSize = -1;

		/* create the pixmap handle */
	newpix = (PixMapHandle) NewHandleClear(sizeof(PixMap));
	if (newpix == NULL) goto bail;
	
		/* set up the pixmap fields */
	pixptr = *newpix;
	pixptr->baseAddr = raster_data;
	pixptr->rowBytes = (row_bytes | 0x8000);
	pixptr->bounds.right = width;
	pixptr->bounds.bottom = height;
	pixptr->hRes = 0x00480000; /* resolution = 72 dots per inch */
	pixptr->vRes = 0x00480000;
	pixptr->pixelSize = 16;
	pixptr->pixelType = RGBDirect;
	pixptr->cmpCount = 3;
	pixptr->cmpSize = 5;
	pixptr->pmTable = colours;

		/* done */
	return newpix;

bail:
	if (raster_data != NULL) DisposePtr(raster_data);
	if (newpix != NULL) DisposeHandle((Handle) newpix);
	if (colours != NULL) DisposeHandle((Handle) colours);
	return NULL;
}


PixMapHandle Make32BitPixMap(short width, short height) {
	long row_bytes;
	Ptr raster_data;
	long bytecount, response;
	CTabHandle colours;
	PixMapHandle newpix;
	PixMapPtr pixptr;
	unsigned char *rowptr;
	unsigned long *pixelptr;
	short y, x;

		/* is it possible? */
	if (Gestalt(gestaltQuickdrawVersion, &response) != noErr) response = 0;
	if (response < gestalt32BitQD) return NULL;

		/* set up initial values for locals */
	row_bytes = ((32 * ((long) width) + 31) >> 5) << 2;
	bytecount = row_bytes * ((long) height);
	colours = NULL;
	raster_data = NULL;
	newpix = NULL;
	
		/* check for valid rowbytes */
	if (row_bytes > 0x00003FFF) goto bail;

		/* allocate the raster data */
	raster_data = NewPtr(bytecount);
	if (raster_data == NULL) goto bail;
	rowptr = (unsigned char *) raster_data;
	for (y=0; y< height; y++, rowptr += row_bytes)
		for (pixelptr = (unsigned long *) rowptr, x=0; x<width; x++)
			*pixelptr++ = 0x00FFFFFF;

		/* dummy colour table */
	colours = (CTabHandle) NewHandleClear(8);
	if (colours == NULL) goto bail;
	(*colours)->ctSeed = 24;
	(*colours)->ctSize = -1;

		/* create the pixmap handle */
	newpix = (PixMapHandle) NewHandleClear(sizeof(PixMap));
	if (newpix == NULL) goto bail;
	
		/* set up the pixmap fields */
	pixptr = *newpix;
	pixptr->baseAddr = raster_data;
	pixptr->rowBytes = (row_bytes | 0x8000);
	pixptr->bounds.right = width;
	pixptr->bounds.bottom = height;
	pixptr->hRes = 0x00480000; /* resolution = 72 dots per inch */
	pixptr->vRes = 0x00480000;
	pixptr->pixelSize = 32;
	pixptr->pixelType = RGBDirect;
	pixptr->cmpCount = 3;
	pixptr->cmpSize = 8;
	pixptr->pmTable = colours;

		/* done */
	return newpix;

bail:
	if (raster_data != NULL) DisposePtr(raster_data);
	if (newpix != NULL) DisposeHandle((Handle) newpix);
	if (colours != NULL) DisposeHandle((Handle) colours);
	return NULL;
}


PixMapHandle MakeScreenLikePixMap(Rect *globalRect) {
	long maxArea, area;
	PixMapHandle device_pix;
	GDHandle device;
	short width, height;
	Rect sect, device_box;
		/* search for maximum intersecting device */
	device_pix = NULL;
	maxArea = 0;
	for (device = GetDeviceList(); device; device = GetNextDevice(device)) {
		device_box = (*device)->gdRect;
		if (TestDeviceAttribute(device, screenDevice)
		  && TestDeviceAttribute(device, screenActive)
		  && SectRect(globalRect, &device_box, &sect)) {
			area = (sect.right - sect.left) * (sect.bottom - sect.top);
			if (area > maxArea) {
				device_pix = (*device)->gdPMap;
				maxArea = area;
			}
		}
	}
		/* create a pixmap */
	if (device_pix != NULL) {
		width = globalRect->right - globalRect->left;
		height = globalRect->bottom - globalRect->top;
		if ((*device_pix)->pixelSize <= 8)
			return MakePixMap(width, height, (*device_pix)->pmTable);
		else if ((*device_pix)->pixelSize == 16)
			return Make16BitPixMap(width, height);
		else if ((*device_pix)->pixelSize == 32)
			return Make32BitPixMap(width, height);
	}
	return NULL;
}


void KillPixMap(PixMapHandle pix) {
	DisposePtr((*pix)->baseAddr);
	DisposeHandle((Handle) (*pix)->pmTable);
	DisposeHandle((Handle) pix);
}

long PixMapSize(PixMapHandle pix) {
	if (pix == NULL) return 0;
	return GetPtrSize((*pix)->baseAddr)
		+ GetHandleSize((Handle) (*pix)->pmTable)
		+ GetHandleSize((Handle) pix);
}

void SetPixMapPixel(PixMapHandle pix, short h, short v, long value) {
	long row_bytes;
	unsigned char *dst;
	row_bytes = (*pix)->rowBytes & 0x00003FFF;
	dst = (unsigned char *) (*pix)->baseAddr;
	switch ((*pix)->pixelSize) {
		case 1:
			dst += ((long) v) * row_bytes;
			if (value != 0) BitSet(dst, h); else BitClr(dst, h);
			break;
		case 2:
			dst += ((long) v) * row_bytes + ((long) (h>>2));
			switch (h&3) {
				case 0:
					*dst = ((*dst) & 0x3F) | (value<<6);
					break;
				case 1:
					*dst = ((*dst) & 0xCF) | ((value&0x03)<<4);
					break;
				case 2:
					*dst = ((*dst) & 0xF3) | ((value&0x03)<<2);
					break;
				case 3:
					*dst = ((*dst) & 0xFC) | (value&0x03);
					break;
			}
			break;
		case 4:
			dst += ((long) v) * row_bytes + ((long) (h>>1));
			if ((h&1) == 0) {
				*dst = ((*dst) & 0x0F) | (value<<4);
			} else {
				*dst = ((*dst) & 0xF0) | (value & 0x0F);
			}
			break;
		case 8:
			dst += ((long) v) * row_bytes + ((long) h);
			*dst = (unsigned char) value;
			break;
		case 16:
			dst += ((long) v) * row_bytes + (((long) h) * 2);
			* ((short*) dst) = (short) value;
			break;
		case 32:
			dst += ((long) v) * row_bytes + (((long) h) * 4);
			* ((long*) dst) = value;
			break;
	}
}


long GetPixMapPixel(PixMapHandle pix, short h, short v) {
	long row_bytes, the_value;
	unsigned char *src;
	row_bytes = ((*pix)->rowBytes & 0x00003FFF);
	src = (unsigned char *) (*pix)->baseAddr;
	switch ((*pix)->pixelSize) {
		case 1:
			src += ((long) v) * row_bytes;
			if (BitTst(src, h)) the_value = 1; else the_value = 0;
			break;
		case 2:
			src += ((long) v) * row_bytes + (((long) h) >> 2);
			switch (h&3) {
				case 0:
					the_value = (long) (((*src) >> 6) & 0x03);
					break;
				case 1:
					the_value = (long) (((*src) >> 4) & 0x03);
					break;
				case 2:
					the_value = (long) (((*src) >> 2) & 0x03);
					break;
				case 3:
					the_value = (long) ((*src) & 0x03);
					break;
			}
			break;
		case 4:
			src += ((long) v) * row_bytes + (((long) h) >> 1);
			if ((h&1) == 0) {
				the_value = (long) (((*src) >> 4) & 0x0F);
			} else {
				the_value = (long) ((*src) & 0x0F);
			}
			break;
		case 8:
			src += ((long) v) * row_bytes + ((long) h);
			the_value = (long) ((*src) & 0x000000FF);
			break;
		case 16:
			src += ((long) v) * row_bytes + (((long) h) << 1);
			the_value = (long) ((* ((short*) src)) & 0x0000FFFF);
			break;
		case 32:
			src += ((long) v) * row_bytes + (((long) h) << 2);
			the_value = * ((long*) src);
			break;
	}
	return the_value;
}


#if defined(powerc) || defined(__powerc)

static short pixround(float x) {
	float low, high;
	low = floor(x);
	high = ceil(x);
	if (fabs(low - x) < fabs(high - x))
		return (short) low;
	else return (short) high;
}

PixMapHandle RotatePixMap(PixMapHandle pix, short cx, short cy, float angle) {
	unsigned long srcrow, dstrow;
	long x, y, width, height;
	unsigned char *srcbase, *dstbase, *d;
	float rad, crad, srad, zx, zy;
	short hpos, vpos;
	PixMapHandle new_pix;
	if ((*pix)->pixelSize <= 8)
		new_pix = MakePixMap((*pix)->bounds.right, (*pix)->bounds.bottom, (*pix)->pmTable);
	else if ((*pix)->pixelSize == 16)
		new_pix = Make16BitPixMap((*pix)->bounds.right, (*pix)->bounds.bottom);
	else if ((*pix)->pixelSize == 32)
		new_pix = Make32BitPixMap((*pix)->bounds.right, (*pix)->bounds.bottom);
	else return NULL;
	if (new_pix != NULL) {
		rad = 0.017453 * angle;
		crad = cos(rad);
		srad = sin(rad);
		srcrow = ((*pix)->rowBytes & 0x3FFF);
		dstrow = ((*new_pix)->rowBytes & 0x3FFF);
		srcbase = (unsigned char*) (*pix)->baseAddr;
		dstbase = (unsigned char*) (*new_pix)->baseAddr;
		height = (*pix)->bounds.bottom;
		width = (*pix)->bounds.right;
		switch ((*pix)->pixelSize) {
			case 1:
			case 2:
			case 4:
				for (y = 0; y < height; y++)
					for (x = 0; x < width; x++) {
						zx = (float) (x - cx);
						zy = (float) (y - cy);
						vpos = pixround((zy*crad) - (zx*srad) + ((float) cy));
						if (vpos >= 0 && vpos < height) {
							hpos = pixround((zx*crad) + (zy*srad) + ((float) cx));
							if (hpos >= 0 && hpos < width)
								SetPixMapPixel(new_pix, x, y, GetPixMapPixel(pix, hpos, vpos));
						}
					}
				break;
			case 8:
				for (y = 0; y < height; y++, dstbase += dstrow)
					for (d = dstbase, x = 0; x < width; x++, d++) {
						zx = (float) (x - cx);
						zy = (float) (y - cy);
						vpos = pixround((zy*crad) - (zx*srad) + ((float) cy));
						if (vpos >= 0 && vpos < height) {
							hpos = pixround((zx*crad) + (zy*srad) + ((float) cx));
							if (hpos >= 0 && hpos < width)
								*d = *(srcbase + vpos * srcrow + hpos);
						}
					}
				break;
			case 16:
				for (y = 0; y < height; y++, dstbase += dstrow)
					for (d = dstbase, x = 0; x < width; x++, d += 2) {
						zx = (float) (x - cx);
						zy = (float) (y - cy);
						vpos = pixround((zy * crad) - (zx * srad) + ((float) cy));
						if (vpos >= 0 && vpos < height) {
							hpos = pixround((zx * crad) + (zy * srad) + ((float) cx));
							if (hpos >= 0 && hpos < width)
								* ((short*) d) = * ((short*) (srcbase + vpos * srcrow + (hpos<<1)));
						}
					}
				break;
			case 32:
				for (y = 0; y < height; y++, dstbase += dstrow)
					for (d = dstbase, x = 0; x < width; x++, d += 4) {
						zx = (float) (x - cx);
						zy = (float) (y - cy);
						vpos = pixround((zy * crad) - (zx * srad) + ((float) cy));
						if (vpos >= 0 && vpos < height) {
							hpos = pixround((zx * crad) + (zy * srad) + ((float) cx));
							if (hpos >= 0 && hpos < width)
								* ((long*) d) = * ((long*) (srcbase + vpos * srcrow + (hpos<<2)));
						}
					}
				break;
		}
	}
	return new_pix;
}

#else

PixMapHandle RotatePixMap(PixMapHandle pix, short cx, short cy, float angle) {
	unsigned long srcrow, dstrow;
	long x, y, i, width, height;
	unsigned char *srcbase, *dstbase, *d, *s;
	Fixed rad, crad, srad, zx, zy, cxx, cyy;
	short hpos, vpos;
	PixMapHandle new_pix;
	if ((*pix)->pixelSize <= 8)
		new_pix = MakePixMap((*pix)->bounds.right, (*pix)->bounds.bottom, (*pix)->pmTable);
	else if ((*pix)->pixelSize == 16)
		new_pix = Make16BitPixMap((*pix)->bounds.right, (*pix)->bounds.bottom);
	else if ((*pix)->pixelSize == 32)
		new_pix = Make32BitPixMap((*pix)->bounds.right, (*pix)->bounds.bottom);
	else return NULL;
	if (new_pix != NULL) {
		rad = FixMul(0x00000478, X2Fix(angle));
		crad = FracCos(rad);
		srad = FracSin(rad);
		srcrow = ((*pix)->rowBytes & 0x3FFF);
		dstrow = ((*new_pix)->rowBytes & 0x3FFF);
		srcbase = (unsigned char*) (*pix)->baseAddr;
		dstbase = (unsigned char*) (*new_pix)->baseAddr;
		height = (*pix)->bounds.bottom;
		width = (*pix)->bounds.right;
		cxx = FixRatio(cx, 1);
		cyy = FixRatio(cy, 1);
		switch ((*pix)->pixelSize) {
			case 1:
			case 2:
			case 4:
				for (y = 0; y < height; y++)
					for (x = 0; x < width; x++) {
						zx = FixRatio(x, 1) - cxx;
						zy = FixRatio(y, 1) - cyy;
						vpos = FixRound(FracMul(zy, crad) - FracMul(zx, srad) + cyy);
						if (vpos >= 0 && vpos < height) {
							hpos = FixRound(FracMul(zx, crad) + FracMul(zy, srad) + cxx);
							if (hpos >= 0 && hpos < width)
								SetPixMapPixel(new_pix, x, y, GetPixMapPixel(pix, hpos, vpos));
						}
					}
				break;
			case 8:
				for (y = 0; y < height; y++, dstbase += dstrow)
					for (d = dstbase, x = 0; x < width; x++, d++) {
						zx = FixRatio(x, 1) - cxx;
						zy = FixRatio(y, 1) - cyy;
						vpos = FixRound(FracMul(zy, crad) - FracMul(zx, srad) + cyy);
						if (vpos >= 0 && vpos < height) {
							hpos = FixRound(FracMul(zx, crad) + FracMul(zy, srad) + cxx);
							if (hpos >= 0 && hpos < width)
								*d = *(srcbase + vpos * srcrow + hpos);
						}
					}
				break;
			case 16:
				for (y = 0; y < height; y++, dstbase += dstrow)
					for (d = dstbase, x = 0; x < width; x++, d += 2) {
						zx = FixRatio(x, 1) - cxx;
						zy = FixRatio(y, 1) - cyy;
						vpos = FixRound(FracMul(zy, crad) - FracMul(zx, srad) + cyy);
						if (vpos >= 0 && vpos < height) {
							hpos = FixRound(FracMul(zx, crad) + FracMul(zy, srad) + cxx);
							if (hpos >= 0 && hpos < width)
								* ((short*) d) = * ((short*) (srcbase + vpos * srcrow + (hpos<<1)));
						}
					}
				break;
			case 32:
				for (y = 0; y < height; y++, dstbase += dstrow)
					for (d = dstbase, x = 0; x < width; x++, d += 4) {
						zx = FixRatio(x, 1) - cxx;
						zy = FixRatio(y, 1) - cyy;
						vpos = FixRound(FracMul(zy, crad) - FracMul(zx, srad) + cyy);
						if (vpos >= 0 && vpos < height) {
							hpos = FixRound(FracMul(zx, crad) + FracMul(zy, srad) + cxx);
							if (hpos >= 0 && hpos < width)
								* ((long*) d) = * ((long*) (srcbase + vpos * srcrow + (hpos<<2)));
						}
					}
				break;
		}
	}
	return new_pix;
}

#endif


PixMapHandle RotatePixRight(PixMapHandle pix) {
	unsigned long srcrow, dstrow, x, y, dstcol, width, height;
	unsigned char *srcbase, *dstbase, *s, *d;
	PixMapHandle new_pix;
		
		/* establish the next pixmap */
	if ((*pix)->pixelSize <= 8)
		new_pix = MakePixMap((*pix)->bounds.bottom, (*pix)->bounds.right, (*pix)->pmTable);
	else if ((*pix)->pixelSize == 16)
		new_pix = Make16BitPixMap((*pix)->bounds.bottom, (*pix)->bounds.right);
	else if ((*pix)->pixelSize == 32)
		new_pix = Make32BitPixMap((*pix)->bounds.bottom, (*pix)->bounds.right);
	else return NULL;
	
	if (new_pix == NULL) return NULL;
	
		/* set up locals */
	srcrow = ((*pix)->rowBytes & 0x3FFF);
	dstrow = ((*new_pix)->rowBytes & 0x3FFF);
	srcbase = (unsigned char*) (*pix)->baseAddr;
	dstbase = (unsigned char*) (*new_pix)->baseAddr;
	dstcol = height = (*pix)->bounds.bottom;
	width = (*pix)->bounds.right;
		
		/* rotate the image */
	switch ((*pix)->pixelSize) {
		case 1:
		case 2:
		case 4:
			for (y = 0; y < height; y++)
				for (x=0; x < width; x++)
					SetPixMapPixel(new_pix, height - y - 1, x, GetPixMapPixel(pix, x, y));
			break;
		case 8:
			for (y = 0; y < height; y++, srcbase += srcrow)
				for (s = srcbase, d = dstbase + (--dstcol), x=0; x < width; x++, d += dstrow)
					*d = *s++;
			break;
		case 16:
			for (y = 0; y < height; y++, srcbase += srcrow)
				for (s = srcbase, d = dstbase + ((--dstcol) << 1), x=0; x < width; x++, s += 2, d += dstrow)
					* ((short*) d) = * ((short*) s);
			break;
		case 32:
			for (y = 0; y < height; y++, srcbase += srcrow)
				for (s = srcbase, d = dstbase + ((--dstcol) << 2), x=0; x < width; x++, s += 4, d += dstrow)
					* ((long*) d) = * ((long*) s);
			break;
	}
	
		/* done */
	return new_pix;
}


PixMapHandle RotatePixLeft(PixMapHandle pix) {
	unsigned long srcrow, dstrow, x, y, dstcol, width, height;
	unsigned char *srcbase, *dstbase, *s, *d, *dststart;
	PixMapHandle new_pix;
		
		/* get the next pixmap */
	if ((*pix)->pixelSize <= 8)
		new_pix = MakePixMap((*pix)->bounds.bottom, (*pix)->bounds.right, (*pix)->pmTable);
	else if ((*pix)->pixelSize == 16)
		new_pix = Make16BitPixMap((*pix)->bounds.bottom, (*pix)->bounds.right);
	else if ((*pix)->pixelSize == 32)
		new_pix = Make32BitPixMap((*pix)->bounds.bottom, (*pix)->bounds.right);
	else return NULL;
	if (new_pix == NULL) return NULL;
	
		/* set up the locals */
	srcrow = ((*pix)->rowBytes & 0x3FFF);
	dstrow = ((*new_pix)->rowBytes & 0x3FFF);
	srcbase = (unsigned char*) (*pix)->baseAddr;
	dstbase = (unsigned char*) (*new_pix)->baseAddr;
	height = (*pix)->bounds.bottom;
	width = (*pix)->bounds.right;
	dststart = dstbase + ((width-1) * dstrow);
	dstcol = 0;
	
		/* rotate the image */
	switch ((*pix)->pixelSize) {
		case 1:
		case 2:
		case 4:
			for (y = 0; y < height; y++)
				for (x=0; x < width; x++)
					SetPixMapPixel(new_pix, y, width - x - 1, GetPixMapPixel(pix, x, y));
			break;
		case 8:
			for (y = 0; y < height; y++, srcbase += srcrow)
				for (s = srcbase, d = dststart + (dstcol++), x = 0; x < width; x++, d -= dstrow)
					*d = *s++;
			break;
		case 16:
			for (y = 0; y < height; y++, srcbase += srcrow)
				for (s = srcbase, d = dststart + ((dstcol++) << 1), x = 0; x < width; x++, s += 2, d -= dstrow)
					* ((short*) d) = * ((short*) s);
			break;
		case 32:
			for (y = 0; y < height; y++, srcbase += srcrow)
				for (s = srcbase, d = dststart + ((dstcol++) << 2), x = 0; x < width; x++, s += 4, d -= dstrow)
					* ((long*) d) = * ((long*) s);
			break;
	}
	return new_pix;
}

PixMapHandle FlipPixVertical(PixMapHandle pix) {
	unsigned long srcrow, dstrow, y, width, height;
	unsigned char *srcbase, *dstbase;
	PixMapHandle new_pix;
	
		/* get the next pixmap */
	if ((*pix)->pixelSize <= 8)
		new_pix = MakePixMap((*pix)->bounds.right, (*pix)->bounds.bottom, (*pix)->pmTable);
	else if ((*pix)->pixelSize == 16)
		new_pix = Make16BitPixMap((*pix)->bounds.right, (*pix)->bounds.bottom);
	else if ((*pix)->pixelSize == 32)
		new_pix = Make32BitPixMap((*pix)->bounds.right, (*pix)->bounds.bottom);
	else return NULL;
	if (new_pix == NULL) return NULL;
	
		/* set up the locals */
	srcrow = ((*pix)->rowBytes & 0x3FFF);
	dstrow = ((*new_pix)->rowBytes & 0x3FFF);
	srcbase = (unsigned char*) (*pix)->baseAddr;
	dstbase = (unsigned char*) (*new_pix)->baseAddr;
	height = (*pix)->bounds.bottom;
	width = (*pix)->bounds.right;
	dstbase += (height * dstrow);
	
		/* flip the image */
	for (y=0; y < height; y++) {
		dstbase -= dstrow;
		BlockMoveData(srcbase, dstbase, dstrow);
		srcbase += srcrow; 
	}
	return new_pix;
}

PixMapHandle FlipPixHorizontal(PixMapHandle pix) {
	unsigned long srcrow, dstrow, x, y, width, height;
	unsigned char *srcbase, *dstbase, *s, *d;
	PixMapHandle new_pix;
	
		/* get the next pixmap */
	if ((*pix)->pixelSize <= 8)
		new_pix = MakePixMap((*pix)->bounds.right, (*pix)->bounds.bottom, (*pix)->pmTable);
	else if ((*pix)->pixelSize == 16)
		new_pix = Make16BitPixMap((*pix)->bounds.right, (*pix)->bounds.bottom);
	else if ((*pix)->pixelSize == 32)
		new_pix = Make32BitPixMap((*pix)->bounds.right, (*pix)->bounds.bottom);
	else return NULL;
	if (new_pix == NULL) return NULL;
	
		/* set up the locals */
	srcrow = ((*pix)->rowBytes & 0x3FFF);
	dstrow = ((*new_pix)->rowBytes & 0x3FFF);
	srcbase = (unsigned char*) (*pix)->baseAddr;
	dstbase = (unsigned char*) (*new_pix)->baseAddr;
	height = (*pix)->bounds.bottom;
	width = (*pix)->bounds.right;
	
		/* flip the image */
	switch ((*pix)->pixelSize) {
		case 1:
		case 2:
		case 4:
			for (y = 0; y < height; y++)
				for (x=0; x < width; x++)
					SetPixMapPixel(new_pix, width - x - 1, y, GetPixMapPixel(pix, x, y));
			break;
		case 8:
			for (y = 0; y < height; y++, srcbase += srcrow, dstbase += dstrow)
				for(s = srcbase + width, d = dstbase, x = 0; x < width; x++)
					*d++ = *--s;
			break;
		case 16:
			for (y = 0; y < height; y++, srcbase += srcrow, dstbase += dstrow)
				for(s = srcbase + (width << 1) - 2, d = dstbase, x = 0; x < width; x++, d += 2, s -= 2)
					* ((short*) d) = * ((short*) s);
			break;
		case 32:
			for (y = 0; y < height; y++, srcbase += srcrow, dstbase += dstrow)
				for(s = srcbase + (width << 2) - 4, d = dstbase, x = 0; x < width; x++, d += 4, s -= 4)
					* ((long*) d) = * ((long*) s);
			break;
	}
	return new_pix;
}


PixMapHandle DuplicatePixMap(PixMapHandle pix) {
	unsigned long srcrow, dstrow, y, width, height;
	unsigned char *srcbase, *dstbase;
	PixMapHandle new_pix;
	
		/* get the next pixmap */
	if ((*pix)->pixelSize <= 8)
		new_pix = MakePixMap((*pix)->bounds.right, (*pix)->bounds.bottom, (*pix)->pmTable);
	else if ((*pix)->pixelSize == 16)
		new_pix = Make16BitPixMap((*pix)->bounds.right, (*pix)->bounds.bottom);
	else if ((*pix)->pixelSize == 32)
		new_pix = Make32BitPixMap((*pix)->bounds.right, (*pix)->bounds.bottom);
	else return NULL;
	if (new_pix == NULL) return NULL;
	
		/* set up the locals */
	srcrow = ((*pix)->rowBytes & 0x3FFF);
	dstrow = ((*new_pix)->rowBytes & 0x3FFF);
	srcbase = (unsigned char*) (*pix)->baseAddr;
	dstbase = (unsigned char*) (*new_pix)->baseAddr;
	height = (*pix)->bounds.bottom;
	width = (*pix)->bounds.right;
	
		/* copy the image */
	for (y=0; y < height; y++) {
		BlockMoveData(srcbase, dstbase, dstrow);
		srcbase += srcrow; 
		dstbase += dstrow;
	}
	return new_pix;
}


PixMapHandle PICTToPixMap(PicHandle pic, CTabHandle clut) {
	Rect bounds;
	PixMapHandle pix;
	PixMapPort *pxmp;
	bounds = (*pic)->picFrame;
	OffsetRect(&bounds, -bounds.left, -bounds.top);
	pix = MakePixMap(bounds.right, bounds.bottom, clut);
	if (pix != NULL)
		WithPixMap(pix, pxmp) {
			DrawPicture(pic, &bounds);
		}
	return pix;
}

PixMapHandle PICTTo16BitPixMap(PicHandle pic) {
	Rect bounds;
	PixMapHandle pix;
	PixMapPort *pxmp;
	bounds = (*pic)->picFrame;
	OffsetRect(&bounds, -bounds.left, -bounds.top);
	pix = Make16BitPixMap(bounds.right, bounds.bottom);
	if (pix != NULL)
		WithPixMap(pix, pxmp) {
			DrawPicture(pic, &bounds);
		}
	return pix;
}


PixMapHandle PICTTo32BitPixMap(PicHandle pic) {
	Rect bounds;
	PixMapHandle pix;
	PixMapPort *pxmp;
	bounds = (*pic)->picFrame;
	OffsetRect(&bounds, -bounds.left, -bounds.top);
	pix = Make32BitPixMap(bounds.right, bounds.bottom);
	if (pix != NULL)
		WithPixMap(pix, pxmp) {
			DrawPicture(pic, &bounds);
		}
	return pix;
}


PicHandle PixMapToPICT(PixMapHandle pix) {
	PicHandle pic;
	GrafPtr saved;
	CGrafPort port;
	Rect bounds;
	bounds = (*pix)->bounds;
	GetPort(&saved);
	OpenCPort(&port);
	pic = OpenPicture(&bounds);
	ClipRect(&bounds);
	PlotPixMap(pix, 0, 0, srcCopy);
	ClosePicture();
	SetPort(saved);
	ClosePort((GrafPtr) &port);
	if (GetHandleSize((Handle) pic) == sizeof(Picture)) {
		KillPicture(pic);
		pic = NULL;
	}
	return pic;
}


PicHandle PixMapToCompressedPICT(PixMapHandle pix) {
	GrafPtr saved;
	CGrafPort port;
	PicHandle pic;
	Rect bounds;
	OSErr err;
	long buffersize, response;
	Ptr compBuffer;
	ImageDescriptionHandle iDesc;
	Boolean port_open;
	
		/* can it be done? */
	if (Gestalt(gestaltCompressionMgr, &response) != noErr) return NULL;
	
		/* initial state */
	compBuffer = NULL;
	iDesc = NULL;
	pic = NULL;
	port_open = false;

		/* draw the picture, make sure we have a port */
	GetPort(&saved);
	OpenCPort(&port);
	ClipRect(&bounds);
	port_open = true;
	
		/* calculate sizes */
	bounds = (*pix)->bounds;
	err = GetMaxCompressionSize(pix, &bounds, 0, codecHighQuality, 'jpeg', anyCodec, &buffersize);
	if (err != noErr) goto bail;
	
		/* allocate storage */
	compBuffer = NewPtr(buffersize);
	if ((err = MemError()) != noErr) goto bail;
	iDesc = (ImageDescriptionHandle) NewHandleClear(sizeof(ImageDescription));
	if ((err = MemError()) != noErr) goto bail;
	
		/* compress the image */
	err = CompressImage(pix, &bounds, codecHighQuality, 'jpeg', iDesc, compBuffer);
	if (err != noErr) goto bail;
		
		/* put the compressed image in a picture */
	pic = OpenPicture(&bounds);
	ClipRect(&bounds);
	err = DecompressImage(compBuffer, iDesc, pix, &bounds, &bounds, srcCopy, NULL);
	ClosePicture();
	
		/* verify that the picture is ok */
	if (err != noErr || GetHandleSize((Handle) pic) == sizeof(Picture)) goto bail;
	
		/* clean up and go */
	SetPort(saved);
	ClosePort((GrafPtr) &port);
	DisposePtr(compBuffer);
	DisposeHandle((Handle) iDesc);
	return pic;

bail:
	if (port_open)  {
		SetPort(saved);
		ClosePort((GrafPtr) &port);
	}
	if (pic != NULL) KillPicture(pic);
	if (compBuffer != NULL) DisposePtr(compBuffer);
	if (iDesc != NULL) DisposeHandle((Handle) iDesc);
	return NULL;
}


PixMapPort* NewPxMP(PixMapHandle pix) {
	GDHandle the_device;
	ITabHandle inverse_table;
	PixMapPort* pxmp;
	Boolean device_active;
	Rect r;
	
		/* set up initial variable states */
	the_device = NULL;
	inverse_table = NULL;
	pxmp = NULL;
	device_active = false;
	
		/* allocate the pxmp */
	pxmp = (PixMapPort*) NewPtr(sizeof(PixMapPort));
	if (pxmp == NULL) goto bail;
	
		/* create the graphics device */
	inverse_table = (ITabHandle) NewHandleClear(2);
	if (inverse_table == NULL) goto bail;
	if ((*pix)->pixelSize <= 8) { /* set it up only for indexed devices */
		MakeITable((*pix)->pmTable, inverse_table, 4);
		if (QDError() != noErr) goto bail;
	}
	
		/* create the graphics device */
	the_device = (GDHandle) NewHandleClear(sizeof(GDevice));
	if (the_device == NULL) goto bail;
	if ((*pix)->pixelSize <= 8)
		(*the_device)->gdType = clutType;
	else (*the_device)->gdType = directType;
	(*the_device)->gdITable = inverse_table;
	(*the_device)->gdResPref = 4;
	(*the_device)->gdPMap = pix;
	(*the_device)->gdRect = (*pix)->bounds;
	(*the_device)->gdMode = -1;
	SetDeviceAttribute(the_device, gdDevType, true);
	SetDeviceAttribute(the_device, noDriver, true);

		/* save the current grafport and device */
	GetPort(&pxmp->saved_port);
	pxmp->saved_device = GetGDevice();

		/* set the current device to the new one*/
	SetGDevice((pxmp->pdevice = the_device));
	device_active = true;

		/* Open the new grafport*/
	OpenCPort(&pxmp->pport);
	SetPort((GrafPtr) &pxmp->pport);
	r = (*pix)->bounds;
	PortSize(r.right, r.bottom);
	ClipRect(&r);
	RectRgn(pxmp->pport.visRgn, &r);

		/* done */
	return pxmp;

bail:
	if (device_active) SetGDevice(pxmp->saved_device);
	if (inverse_table != NULL) DisposeHandle((Handle) inverse_table);
	if (the_device != NULL) DisposeHandle((Handle) the_device);
	if (pxmp != NULL) DisposePtr((Ptr) pxmp);
	return NULL;
}

void DisposePxMP(PixMapPort* px) {
	if (px->pdevice == GetGDevice()) {
		SetGDevice(px->saved_device);
		SetPort(px->saved_port);
	}
	CloseCPort(&px->pport);
	DisposeHandle((Handle) ((*px->pdevice)->gdITable));
	DisposeHandle((Handle) (px->pdevice));
	DisposePtr((Ptr) px);
}

void PlotPixMap(PixMapHandle pix, short h, short v, short mode) {
	GrafPtr port;
	Rect src, dst;
	char hstate;
	GetPort(&port);
	hstate = HGetState((Handle) pix);
	HLock((Handle) pix);
	src = dst = (*pix)->bounds;
	OffsetRect(&dst, h, v);
	CopyBits((BitMap *) (*pix), &port->portBits, &src, &dst, mode, NULL);
	HSetState((Handle) pix, hstate);
}

void PixMapCopy(PixMapHandle pix, Rect *src, Rect *dst, short mode) {
	GrafPtr port;
	char hstate;
	GetPort(&port);
	hstate = HGetState((Handle) pix);
	HLock((Handle) pix);
	CopyBits((BitMap *) (*pix), &port->portBits, src, dst, mode, NULL);
	HSetState((Handle) pix, hstate);
}

PixPatHandle PixMap2PixPat(PixMapHandle pix, Pattern *the_pattern) {
	PixPat ppat;
	PixMap pmap;
	PixPatHandle the_pixpat;
	long bytecount;
	char ctstate;
	short width, height;
	OSErr err;
	
		/* recoverable state */
	the_pixpat = NULL;

		/* check parameters */
	if ((*pix)->pixelSize > 8) goto bail;
	width = (*pix)->bounds.right;
	if (width != 8 && width != 16 && width != 32 && width != 64 && width != 128)
		goto bail;
	height = (*pix)->bounds.bottom;
	if (height != 8 && height != 16 && height != 32 && height != 64 && height != 128)
		goto bail;	
	
		/* set variables to correct initial settings */
	bytecount = ((long) ((*pix)->rowBytes & 0x3FFF)) * ((long) (*pix)->bounds.bottom);
	ppat.patType = 1;
	ppat.patMap = (PixMapHandle) sizeof(PixPat);
	ppat.patData = (Handle) (sizeof(PixPat) + sizeof(PixMap));
	ppat.patXData = NULL;
	ppat.patXValid = -1;
	ppat.patXMap = NULL;
	ppat.pat1Data = *the_pattern;
	pmap = **pix;
	pmap.baseAddr = NULL;
	pmap.pmTable = (CTabHandle) (sizeof(PixPat) + sizeof(PixMap) + bytecount);

		/* add the PixPat header */
	err = PtrToHand(&ppat, (Handle*) &the_pixpat, sizeof(ppat));
	if (err != noErr) goto bail;
	
		/* ... then the PixMap */
	err = PtrAndHand(&pmap, (Handle) the_pixpat, sizeof(PixMap));
	if (err != noErr) goto bail;
	
		/* ... then the raster data */
	err = PtrAndHand((*pix)->baseAddr, (Handle) the_pixpat, bytecount);
	if (err != noErr) goto bail;
	
		/* ... and finally the colour table */
	ctstate = HGetState((Handle) (*pix)->pmTable);
	HLock((Handle) (*pix)->pmTable);
	err = PtrAndHand(*((*pix)->pmTable), (Handle) the_pixpat, GetHandleSize((Handle) (*pix)->pmTable));
	HSetState((Handle) (*pix)->pmTable, ctstate);
	if (err != noErr) goto bail;
	
		/* done */
	return the_pixpat;

bail:
	if (the_pixpat != NULL) DisposeHandle((Handle) the_pixpat);
	return NULL;
}


CTabHandle CalcPixMapColours(PixMapHandle pix) {
	CTabHandle clut;
	OSErr err;
	clut = NULL;
	if ((*pix)->pixelSize <= 8) {
		clut = (*pix)->pmTable;
		if (HandToHand((Handle *) &clut) != noErr) clut = NULL;
	} else {
		PictInfo pictinfo;
		memset(&pictinfo, 0, sizeof(pictinfo));
		err = GetPixMapInfo(pix, &pictinfo,
			returnColorTable+suppressBlackAndWhite,
			254, medianMethod, 0);
		if (err == noErr) clut = pictinfo.theColorTable;
	}
	return clut;
}



