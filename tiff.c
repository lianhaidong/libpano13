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

// Functions to read tiff format


#include "filter.h"

#include "tiffio.h"

int readplanarTIFF(Image *im, TIFF* tif);
void RGBAtoARGB(UCHAR* buf, int width, int bitsPerPixel);
void ARGBtoRGBA(UCHAR* buf, int width, int bitsPerPixel);
int	readtif(Image *im, TIFF* tif);

// image is allocated, but not image data

int readtif(Image *im, TIFF* tif){
	short BitsPerSample, tPhotoMetric, config;
    	long w, h;
    	unsigned long  **hdl_raster;

	if(tif == NULL || im == NULL) return -1;

	TIFFGetField(tif, TIFFTAG_IMAGEWIDTH, &w);
  	TIFFGetField(tif, TIFFTAG_IMAGELENGTH, &h);
   	TIFFGetField(tif, TIFFTAG_BITSPERSAMPLE, &BitsPerSample);
	TIFFGetField(tif, TIFFTAG_PHOTOMETRIC, &tPhotoMetric);
	TIFFGetField(tif, TIFFTAG_PLANARCONFIG, &config);
		
	SetImageDefaults( im );
	im->width 			= w;
	im->height 			= h;
	im->bitsPerPixel 	= 32 * BitsPerSample/8;
	im->bytesPerLine	= im->width * im->bitsPerPixel/8;
	im->dataSize		= im->bytesPerLine * im->height;

		
    	hdl_raster 		= (unsigned long**) mymalloc(im->dataSize);
    	if( hdl_raster == NULL){
		PrintError("Not enough memory");
 		return -1;
	}
		
	im->data = (UCHAR**)hdl_raster;

	if( tPhotoMetric == PHOTOMETRIC_RGB && config == PLANARCONFIG_CONTIG){
	   	return readplanarTIFF(im, tif);
	}
	
	if (TIFFReadRGBAImage(tif, w, h, (unsigned long*)*(im->data), 0)) {
		// Convert agbr to argb; flip image vertically
		unsigned char *cline, *ct, *cb;
		int h2 = im->height/2, y;
					
		cline = (unsigned char*)malloc(im->bytesPerLine);
		if(cline==NULL){
			PrintError("Not enough memory");
			return -1;
		}

		ct = *im->data;
		cb = *im->data + (im->height-1)*im->bytesPerLine;

		for(y=0; y<h2; y++,ct+=im->bytesPerLine,cb-=im->bytesPerLine){
			RGBAtoARGB(ct, im->width, im->bitsPerPixel);
			RGBAtoARGB(cb, im->width, im->bitsPerPixel);
			memcpy(cline,ct,im->bytesPerLine);
			memcpy(ct,cb,im->bytesPerLine);
			memcpy(cb,cline,im->bytesPerLine);
		}
		if(im->height != 2*h2){ // odd number of scanlines
			RGBAtoARGB(*im->data+y*im->bytesPerLine, im->width, im->bitsPerPixel);
		}
		if(cline) free(cline);
	}else{
		PrintError("Could not read tiff-data");
		return -1;
	}
 	return 0;
}

	

int readTIFF(Image *im, fullPath *sfile){
	char filename[512];
	TIFF* tif;
	int result = 0;
	
#ifdef __Mac__
	unsigned char the_pcUnixFilePath[512];//added by Kekus Digital
	Str255 the_cString;
	Boolean the_bReturnValue;
	CFStringRef the_FilePath;
	CFURLRef the_Url;//till here
#endif

	if(FullPathtoString( sfile, filename )){
		PrintError("Could not get filename");
		return -1;
	}

#ifdef __Mac__
	CopyCStringToPascal(filename,the_cString);//Added by Kekus Digital
	the_FilePath = CFStringCreateWithPascalString(kCFAllocatorDefault, the_cString, kCFStringEncodingUTF8);
	the_Url = CFURLCreateWithFileSystemPath(kCFAllocatorDefault, the_FilePath, kCFURLHFSPathStyle, false);
	the_bReturnValue = CFURLGetFileSystemRepresentation(the_Url, true, the_pcUnixFilePath, 512);

	strcpy(filename, the_pcUnixFilePath);//till here
#endif
    
	tif = TIFFOpen(filename, "r");
    
	if ( !tif){
		PrintError("Could not open tiff-file");
		return -1;
	}
	result = readtif(im, tif);
    	TIFFClose(tif);
	return result;
}


int readplanarTIFF(Image *im, TIFF* tif){
   	UCHAR* buf;
   	int y;
	short SamplesPerPixel;
	
	TIFFGetField(tif, TIFFTAG_SAMPLESPERPIXEL, &SamplesPerPixel);
 	if( SamplesPerPixel > 4 ) return -1;
	if( SamplesPerPixel == 3 ){
		im->bitsPerPixel = im->bitsPerPixel * 3 / 4;
		im->bytesPerLine = im->bytesPerLine * 3 / 4;
	}
 
 	buf = (UCHAR*)malloc(TIFFScanlineSize(tif));
   	if( buf == NULL ){
   		PrintError("Not enough memory");
		return -1;
	}
 	
	for (y = 0; y < im->height; y++){
    		TIFFReadScanline(tif, buf, y, 0);
		RGBAtoARGB( buf, im->width, im->bitsPerPixel);
		memcpy( *(im->data) + y*im->bytesPerLine, buf, im->bytesPerLine );
	}
	free( buf );
	ThreeToFourBPP( im );
	return 0;

}

//begin Rik's mask-from-focus hacking (ZComb <stuff>)

// This is experimental code introduced by Rik Littlefield,
// rj.littlefield@computer.org .  See postings on ptX archives
// http://www.email-lists.org/pipermail/ptx/
// and Panorama Tools forum
// http://groups.yahoo.com/group/PanoTools/
// on 6/18/2004 and surrounding for more info. 

#define ZCOMBLOGFILENAME "c:\\Rikslog.txt"

static struct {  // ZComb parameters
	int enabled;
	int passNum;
	int fnameSet;
	int width;
	int height;
	int currentImageNum;
	char firstFname[1024];   // name of first file
	float *accumFocus;       // array width*height of estimated focus accumulated across all images
	float *estFocus;         // array width*height of estimated focus for current image
	int *bestLevel;          // array width*height of image number of best focus
	int masktype;
	int focusWindowHalfwidth;
	int smoothingWindowHalfwidth;
} ZComb;

void ZCombSetDisabled() {
	ZComb.enabled = 0;
}

void ZCombSetEnabled() {
	ZComb.enabled = 1;
	ZComb.masktype = 2;
	ZComb.focusWindowHalfwidth = 4;
	ZComb.smoothingWindowHalfwidth = 4;
}

void ZCombSetMaskType(int mt) {
	ZComb.masktype = mt;
}

void ZCombSetFocusWindowHalfwidth(int fwh) {
	ZComb.focusWindowHalfwidth = fwh;
}

void ZCombSetSmoothingWindowHalfwidth(int swh) {
	ZComb.smoothingWindowHalfwidth = swh;
}

void ZCombLogMsg(char *fmt, char *sarg) {
	FILE* logfile;
	char* logfileName = ZCOMBLOGFILENAME;
	if ((logfile = fopen(logfileName, "a")) == NULL) {
		PrintError("can't open %s\n", logfileName);
		return;
	}
	fprintf (logfile,fmt,sarg);
	fclose (logfile);
}

void ZCombInit() {
	ZComb.fnameSet = 0;
	ZComb.passNum = 1;
	ZComb.currentImageNum = 0;
}

int ZCombInitStats(int width, int height) {
	int row, col;
	ZComb.width = width;
	ZComb.height = height;
	if (ZComb.accumFocus != NULL) {
		free(ZComb.accumFocus);
		free(ZComb.estFocus);
		free(ZComb.bestLevel);
	}
	ZComb.accumFocus = malloc(width*height*sizeof(float));
	ZComb.estFocus   = malloc(width*height*sizeof(float));
	ZComb.bestLevel  = malloc(width*height*sizeof(int));
	if (ZComb.accumFocus == NULL || ZComb.estFocus == NULL || ZComb.bestLevel == NULL) {
		PrintError("Not enough memory");
		ZCombLogMsg("Insufficient memory in ZCombInitStats\n",NULL);
		return -1;
	}
	for (row=0; row < height; row++) {
		for (col=0; col < width; col++) {
			ZComb.accumFocus[row*width+col] = 0.0;
			ZComb.bestLevel[row*width+col] = 1;
		}
	}
	return 0;
}

#define VARIANCEFUDGE 0.0 // increase in variance that we require to say that we have better focus.
                          // early experiments show better results with variance fudge = 0,
                          // so this is not (yet) brought out to be runtime settable
void ZCombAccumEstFocus() {
	int row, col;
	int width = ZComb.width;
	int height = ZComb.height;
	for (row=0; row < height; row++) {
		for (col=0; col < width; col++) {
			if (ZComb.accumFocus[row*width+col] < ZComb.estFocus[row*width+col] - VARIANCEFUDGE) {
				ZComb.accumFocus[row*width+col] = ZComb.estFocus[row*width+col];
				ZComb.bestLevel[row*width+col] = ZComb.currentImageNum;
			}
		}
	}
}

float ZCombGetSmoothedLevel(int row, int col) {
	int n;             /* number of pixels used in kernel */
	int sumLevels;
	int kr, kc;
	int height = ZComb.height;
	int width = ZComb.width;
	int khw = ZComb.smoothingWindowHalfwidth;

	sumLevels = 0;
	n = 0;
	for (kr = row-khw; kr <= row+khw; kr++) {
		for (kc = col-khw; kc <= col+khw; kc++) {
			if (kr < 0 || kr >= height || kc < 0 || kc >= width) {
				int junk = 0;  /* do nothing, allow place for breakpoint */
			} else {
				sumLevels += ZComb.bestLevel[kr*width+kc];
				n++;
			}
		}
	}
	if (n == 0) {
		PrintError("ZCombGetSmoothedLevel: n==0");
		return 0.0;
	}
	return (float)sumLevels / (float)n;
}

/* Estimate focus for current image,
   produce result as float array in ZComb.estFocus.
   In this version, the estimated focus is computed from whatever
   pixel values are sitting in "red" channel of the ARGB
   input image.  These appear to actually be the red channel of
   the RGB image, although the green and blue channels are not
   those of the RGB data.  The estimate is the local variance.
   */
void ZCombEstimateFocus(Image *im) {
	int row;
	int col;
#define KERNELSPARSENESS 1  // make larger than 1 to run faster but more approximate
	int n;             /* number of pixels used in kernel */
	UCHAR *pg;         /* pointer to gray (red) value */
	UCHAR g;           /* gray value */
	UCHAR *pa;         /* pointer to alpha value */
	int sumg, sumgsq;  /* sum of gray values and gray values squared */
	int ming, maxg;    /* minimum and maximum gray levels in each region */
	int kr, kc;
	int height = im->height;
	int width = im->width;
	int khw = ZComb.focusWindowHalfwidth;  /* kernel halfwidth */

	for (row = 0; row < height; row++) {
		for (col = 0; col < width; col++) {
			sumg = 0;
			sumgsq = 0;
			n = 0;
			ming = 256;
			maxg = 0;
			for (kr = row-khw; kr <= row+khw; kr+=KERNELSPARSENESS) {
				for (kc = col-khw; kc <= col+khw; kc+=KERNELSPARSENESS) {
					if (kr < 0 || kr >= height || kc < 0 || kc >= width) {
						int junk = 0;  /* do nothing, allow place for breakpoint */
					} else {
						pg = *(im->data) + kr*(im->bytesPerLine) + kc*4 + 1;  /* ARGB */
						g = *pg;    /* pixel value */
						// if (g < ming) ming = g;
						// if (g > maxg) maxg = g;
						pa = *(im->data) + kr*(im->bytesPerLine) + kc*4 + 2;  /* ARGB */
						if (*pa != 0) { // include only valid pixels in contrast estimate
							sumg += g;
							sumgsq += g*g;
							n++;
						}
					}
				}
			}
			pa = *(im->data) + row*(im->bytesPerLine) + col*4 + 2;  /* ARGB */
			if (*pa == 0) { // do not store contrast estimate at invalid pixels
				ZComb.estFocus[row*width+col] = 0.0;
			}
			else
			if (n <= 1 || maxg == ming) {
				ZComb.estFocus[row*width+col] = 0.0;
			} else {
				/* variance squared */
				ZComb.estFocus[row*width+col] = ((n*sumgsq - sumg*sumg) / (float)(n*(n-1)));
//											   /((maxg-ming)*(maxg-ming));
			}
		}
	}
}

void ZCombCopyEstFocusToBlue(Image *im) {
	int row;
	int col;
	UCHAR *pg;         /* pointer to gray (blue) value */
	UCHAR g;           /* gray value */
	int height = im->height;
	int width = im->width;

	float maxEst = 0.0;
	for (row = 0; row < height; row++) {
		for (col = 0; col < width; col++) {
			if (ZComb.estFocus[row*width+col] > maxEst) {
				maxEst = ZComb.estFocus[row*width+col];
			}
		}
	}

	for (row = 0; row < height; row++) {
		for (col = 0; col < width; col++) {
			pg = *(im->data) + row*(im->bytesPerLine) + col*4 + 3;  /* ARGB */
			g = (int) (255.0*ZComb.estFocus[row*width+col]/maxEst);
			*pg = g;
		}
	}
}

void ZCombSetMaskFromFocusData(Image *im) {
	int row;
	int col;
	UCHAR *pg;         /* pointer to gray (blue) value */
	UCHAR g;           /* gray value */
	int height = im->height;
	int width = im->width;
	float flev;

	for (row = 0; row < height; row++) {
		for (col = 0; col < width; col++) {
			pg = *(im->data) + row*(im->bytesPerLine) + col*4 + 0;  /* ARGB */
			if (ZComb.masktype == 0 || ZComb.masktype == 1) {
				// generate hard-edged masks using unsmoothed depth level -- select strictly pixels belonging to best-focused image
				if ((ZComb.masktype == 0 && ZComb.currentImageNum == ZComb.bestLevel[row*width+col])  // generate non-nested masks
				 || (ZComb.masktype == 1 && ZComb.currentImageNum <= ZComb.bestLevel[row*width+col])  // generate a stack of nested masks
				) {
					*pg = 255;
				} else {
					*pg = 0;
				}
			} else if (ZComb.masktype == 2) {
				// generate blending masks using smoothed depth level -- typically makes much better looking images
				flev =  ZCombGetSmoothedLevel(row,col);
				if (ZComb.currentImageNum <= flev+0.01) {
					*pg = 255;
				} else if (ZComb.currentImageNum > flev+1.01) {
					*pg = 0;
				} else {
					*pg = 255 - (UCHAR) (255.0*(ZComb.currentImageNum - (flev+0.01)));
				}
			} else {
				*pg = 255;  // unrecognized masktype, just keep from crashing
			}
			// following is a hack that forces nominally binary masks to actually have a single
			// intermediate value, which keeps Jim Watter's PSD modifications from incorrectly
			// assigning shape mask = visibility mask.
			if (row == 0 && col == 0) {
				if (*pg == 0) *pg = 1;
				if (*pg == 255) *pg = 254;
			}
		}
	}
}

void ZCombSetGreenTo255(Image *im) {
	int row;
	int col;
	UCHAR *pg;         /* pointer to gray (green) value */
	int height = im->height;
	int width = im->width;

	for (row = 0; row < height; row++) {
		for (col = 0; col < width; col++) {
			pg = *(im->data) + row*(im->bytesPerLine) + col*4 + 2;  /* ARGB */
			*pg = 255;
		}
	}
}

int ZCombSeeImage(Image *im, char *filename) {
	if (!ZComb.enabled) {
		// ZCombLogMsg ("Z-combining disabled\n",NULL);
		return 0;
	}
	ZCombLogMsg ("Z-combining enabled\n",NULL);
	ZCombLogMsg ("writeTIFF called on file %s\n",filename);
	ZCombLogMsg ("   image name = %s\n",im->name);
	if (ZComb.fnameSet && strcmp(ZComb.firstFname,filename) == 0) {
		ZCombLogMsg ("Starting second pass\n",NULL);
		ZComb.passNum = 2;
		ZComb.currentImageNum = 0;
	}
	if (!ZComb.fnameSet) {
		ZComb.fnameSet = 1;
		ZComb.passNum = 1;
		strcpy(ZComb.firstFname,filename);
		ZCombLogMsg("   initialFname set to %s\n",ZComb.firstFname);
		if (ZCombInitStats(im->width,im->height)) {
			return -1;
		}
	}
	ZComb.currentImageNum++;
	if (ZComb.passNum == 1) {
		ZCombEstimateFocus(im);
		ZCombAccumEstFocus();
		// ZCombCopyEstFocusToBlue(im); // for debugging purposes
		// ZCombSetGreenTo255(im);
	}
	if (ZComb.passNum == 2) {
		ZCombEstimateFocus(im);
		ZCombSetMaskFromFocusData(im);
	}
	return 0;
}

// end Rik's mask-from-focus hacking

int writeTIFF(Image *im, fullPath *sfile){
	char string[512];
	TIFF *tif;
	UCHAR* buf;
	int bufsize,y;

#ifdef __Mac__
	unsigned char the_pcUnixFilePath[512];//added by Kekus Digital
	Str255 the_cString;
	Boolean the_bReturnValue;
	CFStringRef the_FilePath;
	CFURLRef the_Url;//till here
#endif

	if(FullPathtoString( sfile, string )){
		PrintError("Could not get filename");
		return -1;
	}
    
#ifdef __Mac__
	CopyCStringToPascal(string,the_cString);//added by Kekus Digital
	the_FilePath = CFStringCreateWithPascalString(kCFAllocatorDefault, the_cString, kCFStringEncodingUTF8);
	the_Url = CFURLCreateWithFileSystemPath(kCFAllocatorDefault, the_FilePath, kCFURLHFSPathStyle, false);
	the_bReturnValue = CFURLGetFileSystemRepresentation(the_Url, true, the_pcUnixFilePath, 512);

	strcpy(string, the_pcUnixFilePath);//till here
#endif
    
	tif = TIFFOpen(string, "w");
	if( !tif ){
		PrintError("Could not create TIFF-file");
		return -1;
	}
	
	// Rik's mask-from-focus hacking
	if (ZCombSeeImage(im,string)) {
		PrintError("failed ZCombSeeImage");
	}
    // end Rik's mask-from-focus hacking (for the moment...)

	TIFFSetField(tif, TIFFTAG_IMAGEWIDTH, im->width);
  	TIFFSetField(tif, TIFFTAG_IMAGELENGTH, im->height);
   	TIFFSetField(tif, TIFFTAG_BITSPERSAMPLE, im->bitsPerPixel < 48 ? 8 : 16 );
	TIFFSetField(tif, TIFFTAG_PHOTOMETRIC, PHOTOMETRIC_RGB);
	TIFFSetField(tif, TIFFTAG_PLANARCONFIG, PLANARCONFIG_CONTIG);
	TIFFSetField(tif, TIFFTAG_SAMPLESPERPIXEL, im->bitsPerPixel == 24 || im->bitsPerPixel == 48 ? 3 : 4);
	TIFFSetField(tif, TIFFTAG_COMPRESSION, COMPRESSION_PACKBITS );	
	TIFFSetField(tif, TIFFTAG_ORIENTATION, ORIENTATION_TOPLEFT );
	TIFFSetField(tif, TIFFTAG_ROWSPERSTRIP, im->height );
	
	bufsize = TIFFScanlineSize(tif);
	if(bufsize < im->bytesPerLine) bufsize = im->bytesPerLine;
 	buf = (UCHAR*)malloc(bufsize);
   	if( buf == NULL ){
   		PrintError("Not enough memory");
		return -1;
	}

	for (y = 0; y < im->height; y++){
		memcpy( buf, *(im->data) + y*im->bytesPerLine, im->bytesPerLine );
		ARGBtoRGBA( buf, im->width, im->bitsPerPixel);
		TIFFWriteScanline(tif, buf, y, 1);		
	}
	free( buf );
	TIFFClose( tif );
	return 0;

}	




void RGBAtoARGB(UCHAR* buf, int width, int bitsPerPixel){
	int x;
	switch(bitsPerPixel){ 
		case 32: {
				UCHAR pix;
				for(x=0; x<width; x++,buf+=4){
					pix = buf[3];
					buf[3] = buf[2];
					buf[2] = buf[1];
					buf[1] = buf[0];
					buf[0] = pix;
				}
			}
			break;
		case 64: {
				USHORT *bufs=(USHORT*)buf, pix;
				for(x=0; x<width; x++,bufs+=4){
					pix = bufs[3];
					bufs[3] = bufs[2];
					bufs[2] = bufs[1];
					bufs[1] = bufs[0];
					bufs[0] = pix;
				}
			}
			break;
	}
}

void ARGBtoRGBA(UCHAR* buf, int width, int bitsPerPixel){
	int x;
	switch(bitsPerPixel){
		case 32:{
				UCHAR pix;
				for(x=0; x<width; x++, buf+=4){
					pix = buf[0];
					buf[0] = buf[1];
					buf[1] = buf[2];
					buf[2] = buf[3];
					buf[3] = pix;
				}
			}
			break;
		case 64:{
				USHORT *bufs = (USHORT*)buf, pix;
				for(x=0; x<width; x++, bufs+=4){
					pix = bufs[0];
					bufs[0] = bufs[1];
					bufs[1] = bufs[2];
					bufs[2] = bufs[3];
					bufs[3] = pix;
				}
			}
			break;
	}
}


