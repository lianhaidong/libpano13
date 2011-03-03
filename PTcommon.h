/*
 *  PTcommon.h
 *
 *  Many of the routines are based on the program PTStitcher by Helmut
 *  Dersch.
 * 
 *  Copyright Helmut Dersch and Daniel M. German
 *  
 *  Dec 2006
 *
 *  This program is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU General Public
 *  License as published by the Free Software Foundation; either
 *  version 2 of the License, or (at your option) any later version.
 *
 *  This software is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public
 *  License along with this software; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 *
 *  Author: Daniel M German dmgerman at uvic doooot ca
 * 
 */

#ifndef __PTcommon_h__

#define __PTcommon_h__

#include "panorama.h"
#include "pt_stdint.h"
#include "file.h"

typedef struct {
  uint16_t samplesPerPixel;
  uint16_t bitsPerSample;
  uint32_t imageLength;
  uint32_t imageWidth;
  int bytesPerLine;
  int bitsPerPixel;
  uint32_t rowsPerStrip;
  uint16_t compression;
  uint16_t predictor;
} pt_tiff_parms;


extern int ptQuietFlag;

int panoVerifyTiffsAreCompatible(fullPath *tiffFiles, int filesCount, int optionalCheck);
int panoAddStitchingMasks(fullPath *inputFiles, fullPath *outputFiles, int numberImages, int featherSize);

/*  defined in ptpicker.c, but never exported */

int panoFlattenTIFF(fullPath *fullPathImages, int counterImageFiles, fullPath *outputFileName, int removeOriginals);



extern int quietFlag;

int  panoPSDCreate(  fullPath *fullPathImages, int, fullPath*, pano_flattening_parms*);
int  panoCreateLayeredPSD(  fullPath *fullPathImages, int, fullPath*, pano_flattening_parms*);

int panoCreatePanorama(fullPath ptrImageFileNames[], int counterImageFiles, fullPath *panoFileName, fullPath *scriptFileName);
void ARGtoRGBAImage(Image *im);
void panoReplaceExt(char* filename, char *extension);
int panoUnCropTiff(char *inputFile, char *outputFile);


int 	StringtoFullPath	(fullPath *path, char *filename);
void InsertFileName( fullPath *fp, char *fname );
int ApplyFeather(fullPath * inputFile, fullPath * outputFile,
		 int featherSize);

/*****************/
#define PANO_CROPPING_UNCROP 1
#define PANO_CROPPING_CROP   2

int panoCroppingMain(int argc,char *argv[], int operation, char *version,char *usage, char *defaultPrefix);

void panoPrintImage(char *msg, Image *im);

#ifndef min
   #define min(a,b) ((a) <= (b) ? (a) : (b))
#endif

#endif
