/*
 *  ColourBrightness
 *
 *  Based on the program PTStitcher by Helmut Dersch.
 *  
 *  It is intended to duplicate the functionality of original program
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
 *  Author: Daniel M German dmgerman at uvic doooot ca
 * 
 */




#ifndef __COLOUR_BRIGHTNESS__

#include "PTmender.h"


typedef double (*calla_function)(double [], double, int);
typedef double *magnolia_array;

typedef struct {
  int components; // number of components
  magnolia_array fieldx04[6]; // pointer to arrays  double[fieldx00]
  calla_function function; // a function //offset 0x1c 
} magnolia_struct;


typedef int *(histogram_type[6]);


typedef struct  {
  int overlappingPixels; //initialy zero
  int bytesPerSample; // bytesPerSample
  int numberDifferentValues; //used as a size to allocate pointers below
  int baseImageNumber; // image number
  int otherImageNumber;
  histogram_type ptrBaseHistograms; // array of pointers 6 
  // it seems to get the "raw" histogram
  histogram_type ptrOtherHistograms; // array of 6 pointers
  // this has some processing done
} histograms_struct;

typedef struct {
  fullPath * fullPathImages; //<- the struct starts here (size 20)
  int numberImages;   // int counterImages
  int indexReferenceImage;   // contains nfix
  histograms_struct *ptrHistograms;   // seems to be a pointer to an array of pointers, returned by Unknown34
  magnolia_struct *magnolia;  // This looks like a pointer to  array of counterImages * magnolia_struct,	
  // returned by Initialize_Magnolia
} calla_struct;


histograms_struct*ReadHistograms (fullPath *fullPathImages, int counterImages);
int               ComputeColourBrightnessCorrection(calla_struct *calla);
int               CorrectFileColourBrightness(fullPath *path, magnolia_struct *magnolia, int parm3);
int               FindNextCandidate(int candidates[], calla_struct *calla);

magnolia_struct   *InitializeMagnolia(int numberImages, int size, calla_function parm2);
void              ColourBrightness(  fullPath *fullPathImages, int counterImages, int indexReferenceImage, int parm3);
void              CorrectImageColourBrigthness(Image *image, magnolia_struct *magnolia, int parm3);
void              FreeHistograms(histograms_struct *ptrHistograms, int count);
void              RemapHistogram(int *histogram, double *array, magnolia_struct *magnolia, int channel);
void              ComputeAdjustmentCurve(double *sourceHistogram, double *targetHistogram, double *curve) ;
unsigned char Unknown47(unsigned char parm0, unsigned char parm1, unsigned char parm2);
unsigned char Unknown48(unsigned char parm0, unsigned char parm1, unsigned char parm2);
unsigned char Unknown49(unsigned char parm0, unsigned char parm1, unsigned char parm2);

double            MapFunction(double p[], double x, int n);
int               Unknown40(int value, double mapTable[]) ;

unsigned char Peach (unsigned char parm0, unsigned char parm1, unsigned char parm2);
unsigned char Cherry (unsigned char parm0, unsigned char parm1, unsigned char parm2);
unsigned char Apple (unsigned char parm0, unsigned char parm1, unsigned char parm2);





#endif


