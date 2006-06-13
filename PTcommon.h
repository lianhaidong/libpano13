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

int VerifyTiffsAreCompatible(fullPath *tiffFiles, int filesCount);
int AddStitchingMasks(fullPath *inputFiles, fullPath *outputFiles, int numberImages, int featherSize);

/*  defined in ptpicker.c, but never exported */
void InsertFileName( fullPath *fp, char *fname );

int FlattenTIFF(fullPath *fullPathImages, int counterImageFiles, fullPath *outputFileName, int removeOriginals);
int  CreatePSD(  fullPath *fullPathImages, int, fullPath*);


extern int quietFlag;
	
int 	StringtoFullPath	(fullPath *path, char *filename);
int CreatePanorama(fullPath ptrImageFileNames[], int counterImageFiles, fullPath *panoFileName, fullPath *scriptFileName);

void ReplaceExt(char* filename, char *extension);

#endif
