/*
 *  PTStitcher
 *
 *  Based on the program  by Helmut Dersch's panorama-tools
 *  to duplicate the functionality of original program
 *
 *  Dec 2005
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
 *
 * 
 */


/* Global variables */

#ifndef __PTmender_h__

#define __PTmender_h__


extern stBuf global5640;
extern int   quietFlag;

extern VRPanoOptions defaultVRPanoOptions;
extern int jpegQuality;
extern int jpegProgressive;



/* These functions are already implemented */



void ReplaceExt(char *, char*);
char* Filename(fullPath* path);
int CreatePanorama(fullPath ptrImageFileNames[], int counterImageFiles, fullPath *panoFileName, fullPath *scriptFileName);
void ARGtoRGBAImage(Image *im);


/* These functions need to be implemented */

void Clear_Area_Outside_Selected_Region(Image *currentImagePtr);
void Colour_Brightness(  fullPath *fullPathImages, int p1, int p2, int p3);
int CreateStitchingMasks(  fullPath *fullPathImages, int);
int FlattenTIFF(  fullPath *fullPathImages, int);
int  CreatePSD(  fullPath *fullPathImages, int, fullPath*);
int Unknown01(Image *, fullPath*);
int Unknown02(Image *, fullPath*);
int Unknown03(Image *, fullPath*);
int Unknown04(Image *, fullPath*);
int Unknown05(Image *, fullPath*);
int Unknown07(Image *, fullPath*);
int Create_LP_ivr(Image *, fullPath*);



/*  defined in ptpicker.c, but never exported */
void InsertFileName( fullPath *fp, char *fname );


#endif

