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

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <dirent.h>
#include <unistd.h>
#include <stdint.h>
#include <math.h>

#include "tiffio.h"
#include "filter.h"
#include "panorama.h"

#include "PTcommon.h"

#include "ColourBrightness.h"

FILE *debugFile;


#ifdef __TESTING__
#include <dmalloc.h>
#endif


magnolia_struct *InitializeMagnolia(int numberImages, int size, calla_function parm2)
{
  int j;
  int i; 
  magnolia_struct *magnolia = NULL;
  double *ptrDouble;
  int var04;
  double var16;
  int ecx;

  if ((magnolia = malloc(numberImages * sizeof(magnolia_struct) )) == 0) {
    
    return 0;
    
  }
  
  var04 = size - 1;
  
  var16 = (size -1 ) / 255.0; /// shouldn't this be a 256?
  
  for (i=0; i < numberImages; i++) {

    magnolia[i].components = size;

    magnolia[i].function = parm2;

    for (j=0; j<6 ; j++) { 

      if ((ptrDouble = calloc(size, sizeof(double))) == NULL) {
	return NULL;
      }

      assert( magnolia[i].components == size);

      for (ecx = 0;  ecx < size; ecx ++) {
	
	ptrDouble[ecx] = ecx * var16;
	
      } // if 

      magnolia[i].fieldx04[j] = ptrDouble;
      
    } //    for (j=0; j<6; j++) { 

  } // end   for (i=0; i < numberImages; i++) {

  return magnolia;

}


void DisplayHistogramsError(int numberHistograms,   histograms_struct * ptrHistograms)
{
  int index;
  histogram_type *    ptrOtherHistograms;
  histogram_type *    ptrBaseHistograms;
  int currentColour;
  int i;

  for (index = 0; index < numberHistograms; index++ ) {
    
    // if the number of overlapping pixels is less than 1k then it ignores the images
    
    if ( ptrHistograms[index].overlappingPixels  > 999 ) {
      
      fprintf(debugFile, "Histogram %d Images %d %d, %d Pixels: ", index , 
	      ptrHistograms[index].baseImageNumber, 
	      ptrHistograms[index].otherImageNumber,
	      ptrHistograms[index].overlappingPixels);
      
      ptrBaseHistograms = &(ptrHistograms[index].ptrBaseHistograms); 
      ptrOtherHistograms = &(ptrHistograms[index].ptrOtherHistograms);
      
      for (currentColour = 0; currentColour < 3; currentColour++) {
	
	double sum = 0;
	int *var192;
	int *var200;
	
	var192 = (*ptrBaseHistograms)[currentColour];
	
	var200 = (*ptrOtherHistograms)[currentColour];
	
	for (i =0; i < 0x100; i++) {
	  
	  int diff = var192[i] - var200[i];
	  
	  sum += diff * diff;
	  
	} 
	
	fprintf(debugFile, "  %g", sum * 1.0 /ptrHistograms[index].overlappingPixels);
	
      } //for (currentColour = 0; currentColour < 3; currentColour++ {
      
      fprintf(debugFile, "\n");
      
    } // if ( > 999) 
  } //   for (index = 0; index < numberHistograms; index++ ) {
  
} 



void ColourBrightness(  fullPath *fullPathImages,  fullPath *outputFullPathImages, 
			 int counterImages, int indexReferenceImage, int parm3)
{

  histograms_struct * ptrHistograms;
  histograms_struct * ptrHistograms2;
  int numberHistograms;
  int index;
  calla_struct calla;  
  char string[128];
  int i;
  extern FILE *debugFile;  // 0x8054600

  numberHistograms = ((counterImages-1) * counterImages)/2;

  debugFile = fopen("Debug.txt", "w");
  //  debugFile = stderr;

  fprintf(debugFile, "Entering function \"Colour_Brightness\" with %d images, nfix =%d\n", counterImages, indexReferenceImage);

  calla.ptrHistograms = ReadHistograms(fullPathImages, counterImages);

  if ( calla.ptrHistograms == 0 )
    return ;

  ptrHistograms = calla.ptrHistograms;

  fprintf(debugFile, "\nQuality before optimization:\n");

  index = 0;

  DisplayHistogramsError(numberHistograms, calla.ptrHistograms);

  ///////////////////////////////////////////
  
  calla.fullPathImages = fullPathImages;
  
  calla.numberImages = counterImages;
  
  calla.indexReferenceImage = indexReferenceImage; // 
  
  calla.magnolia = InitializeMagnolia(counterImages, 0x100, MapFunction);

  if (calla.magnolia == 0 )
    return ;

  if (ComputeColourBrightnessCorrection(&calla) == 0) 
    return ;

  fprintf(debugFile, "\nResults of Optimization:");

  for (index=0;  index < counterImages; index++ ) {
    int edi;
    magnolia_struct *magnolias;

    fprintf(debugFile, "\nImage %d:\nRed Channel:   ", index);

    magnolias = calla.magnolia; // ecx

    for (edi = 0;  edi < magnolias[index].components; edi++)  {

      double *array;

      array = magnolias[index].fieldx04[0];

      fprintf(debugFile, "%g ", array[edi]);

    } //    while (( %edi < (%ecx+ebx) ) {

    fprintf(debugFile, "\nGreen Channel: ");
    

    for (edi = 0; edi < magnolias[index].components; edi++) {
      
      double *array;
      array = magnolias[index].fieldx04[1];

      fprintf(debugFile, "%g ", array[edi]);
    }

    fprintf(debugFile, "\nBlue Channel:  ");

    for (edi = 0; edi < magnolias[index].components; edi++) {
      
      double *array;
      array = magnolias[index].fieldx04[2];

      fprintf(debugFile, "%g ", array[edi]);
    }

  } //  for (index=0;  index < counterImages; index++ ) {


  if ( ptQuietFlag == 0 )

    Progress(_initProgress, "Adjusting Colour and Brightness");
  
  index = 0;

  for (index = 0;  index <counterImages; index++ ) {

    sprintf(string, "%d", index * 100 / counterImages);

    if ( ptQuietFlag == 0 ) {

      if (Progress(_setProgress, string) == 0) 
	return ;
      
    } //if ( ptQuietFlag == $0x0 )
    
    
    if (strcmp(fullPathImages[index].name,
	       outputFullPathImages[index].name) != 0  ||
	index != indexReferenceImage ) {

      // Do the correction if the input and output filename are different
      // or if it is not the reference image
      
      if (CorrectFileColourBrightness(&fullPathImages[index], &outputFullPathImages[index],
				      &calla.magnolia[index], parm3) != 0)
	return;
    }  
  } //  for (index = 0;  index <counterImages; index++ ) {
  
  
  ptrHistograms2 = ReadHistograms(fullPathImages, counterImages);
    ////////////////////////////
  
  fprintf(debugFile, "\nQuality after optimization:");
  
  DisplayHistogramsError(numberHistograms, ptrHistograms2);

  ////////////////////////////////////
  
  FreeHistograms(calla.ptrHistograms, numberHistograms);
  FreeHistograms(ptrHistograms2, numberHistograms);
  
  for (i = 0; i <counterImages; i++ ) {
    
    for (index = 0; index < 6; index ++) 
      free(calla.magnolia[i].fieldx04[index]);
    
  }
  free(calla.magnolia);
  
}

void FreeHistograms(histograms_struct *ptrHistograms, int count)
{
  int i;
  int index;
  histograms_struct *savedPtrHistograms = ptrHistograms;

  for (i = 0; i < count; i++ ) {
    for (index = 0; index < 6; index ++) {
      free(ptrHistograms->ptrBaseHistograms[index]);
      free(ptrHistograms->ptrOtherHistograms[index]);
    }
    ptrHistograms++;
  } //     for (i = 0; i < numberHistograms; i++ ) {
  free(savedPtrHistograms);
}



int CorrectFileColourBrightness(fullPath *inPath, fullPath *outPath, magnolia_struct *magnolia, int parm3)
{
  Image image;
  char tempString[512];
  if (readTIFF (&image, inPath) != 0) {
    sprintf(tempString, "Could not read TIFF file %s", inPath->name);
    PrintError(tempString);
    return -1;
  }   
  
  CorrectImageColourBrigthness(&image, magnolia, parm3);
  
  writeTIFF(&image, outPath);

  myfree((void**)image.data);
  return(0);

}



int FindNextCandidate(int candidates[], calla_struct *calla)
{

  // Find  image with maximum overlap not yet corrected.

  // The algorithm is simple.

  // For each daisy
  //   if one of the images is in, but no the other
  //   edi[not in image] += overlap

  // return image with largest overlap
  // if not found return -1

  int *overlapping;
  int i;
  int max;
  int overlappingPixels;
  int baseImage;
  int otherImage;
  int returnValue;
  int numberDaisies;

  histograms_struct *ptrHistograms;


  numberDaisies = calla->numberImages * (calla->numberImages -1)/2;

  if ((overlapping = malloc(calla->numberImages * sizeof(int))) == 0) {
    PrintError("Not enough memory\n");
    return -1; // not more memory
  }

  for (i = 0; i < calla->numberImages; i++) {

    overlapping[i] = 0;

  } //  for (i = 0; i < calla->numberImages; i++) {


  for (i = 0; i < numberDaisies; i++) {

    ptrHistograms = calla->ptrHistograms;

    overlappingPixels = ptrHistograms[i].overlappingPixels;
    baseImage = ptrHistograms[i].baseImageNumber;
    otherImage = ptrHistograms[i].otherImageNumber;

    assert(baseImage < calla->numberImages);
    assert(otherImage < calla->numberImages);
    assert(baseImage >= 0);
    assert(otherImage >= 0);
    assert(baseImage != otherImage);

    if ( overlappingPixels > 1000 ) {

      
      if (candidates[baseImage] == 0 ||
	  candidates[otherImage] != 0 ) {
	
	// baseImageNumber not in OR  other in

	// Here we have four alternatives:

	// 1. baseImageNumber not AND  other not      <- we need to skip this case
	// 2. baseImageNumber not AND  other in   

	// 3. baseImageNumber in AND other in         <- we need to skip this case
	// 4. baseImageNumber not AND other in


	// so the condition is: not baseImageNumber and otherIn

	
	if (candidates[otherImage] != 0  &&
	    candidates[baseImage] == 0) {

	  overlapping[baseImage]+=overlappingPixels;
	}
	
      } else {
      
	// This code is executed if:
	
	// NOT (baseImageNumber == 0 OR otherImageNumber == 1) =>
	// NOT (baseImageNumber == 0 ) AND NOT (otherImageNumber == 1) =>
	// baseImageNumber in AND otherImageNumber is not in

	// if the base is in, AND not the other, then add it to the other

	overlapping[otherImage]+=overlappingPixels;

      }
      
    } //    if ( overlappingPixels > 1000 ) {


  } //  for (i = 0; i < numberDaisies; i++) {


  returnValue = -1;

  // Find maximum
  max = 0;
  for (i =0; i < calla->numberImages ; i++ ) {
  
    if ( overlapping[i] > max ) {

      max = overlapping[i];

      returnValue = i;
  
    }
  
  } //  for (i =0; i < calla->numberImages ; i++ ) {

  free(overlapping);

  // We need to assert the value 
  
  if(returnValue >= 0) {
    assert(returnValue < calla->numberImages);
    assert(candidates[returnValue] == 0);
  }
    

  return returnValue;


}



int ComputeColourBrightnessCorrection(calla_struct *calla)
{


  double *remappedSourceHistogram;
  double *accumToCorrectHistogram;
  double *accumSourceHistogram;
  int *processedImages;

  int currentImageNumber;
  int channel;
  int numberIntersections;
  histograms_struct *currentHistogram;
  int j;

  int **ptrHistogram;
  int *array;
  int i;

  numberIntersections = ((calla->numberImages - 1) * calla->numberImages)/2;

  processedImages = calloc(calla->numberImages, sizeof(int));


  accumToCorrectHistogram = malloc(0x100 * sizeof(double));

  accumSourceHistogram = malloc(0x100 * sizeof(double));


  remappedSourceHistogram = malloc(0x100 * sizeof(double));


  if ( processedImages == 0 )
    return 0;
  
  if ( accumToCorrectHistogram == 0 ) 
    return 0;
  
  if (accumSourceHistogram == 0 ) 
    return 0;
  
  if (remappedSourceHistogram == 0 )
    return 0;
  
  // Mark starting image as done
  processedImages[calla->indexReferenceImage] = 1;

  while ((currentImageNumber = FindNextCandidate(processedImages, calla)) != -1) {

    // We have a candidate image: currentImageNumber

    assert(currentImageNumber >= 0);
    assert(currentImageNumber < calla->numberImages);
    assert(processedImages[currentImageNumber] == 0);

    // For every channel it does the correction independently.
    // clean the accum histograms
    
    for (channel = 0; channel < 6; channel++) {

      int currentIntersection;
      
      for (i =0; i <= 0xff;  i ++) {
	
	accumSourceHistogram[i] = 0;
	
	accumToCorrectHistogram[i] = 0;

      }

      // for each intersection between 2 images

      for (currentIntersection = 0;   currentIntersection < numberIntersections ; currentIntersection++) {

	currentHistogram = &(calla->ptrHistograms[currentIntersection]);


	// DO it only if the overlap is bigger than 1k pixels

	if ( currentHistogram->overlappingPixels > 1000 ) { // it is not consistent XXX
	
	  if ( currentHistogram->baseImageNumber == currentImageNumber && 
	       processedImages[currentHistogram->otherImageNumber] != 0 ) {

	    // this means the otherImage has been already processed
	    // but not baseImageNumber
	    
	    // REMAP histogram according to current mapping function

	    //	    fprintf(stderr, "Original histogram Channel %d [%d,Base %d]: \n", channel, currentImageNumber, currentHistogram->otherImageNumber);
	    //	    for (j = 0; j<0x100; j++) {
	    //	      fprintf(stderr, " %d:%d", j, currentHistogram->ptrOtherHistograms[channel][j]);
	    //	    }
	    //	    fprintf(stderr, "\n");



	    // Remap histogram of other image

	    RemapHistogram(currentHistogram->ptrOtherHistograms[channel],
			   remappedSourceHistogram,  
			   &(calla->magnolia[currentHistogram->otherImageNumber]), channel);

	    //	    fprintf(stderr, "\n");

	    //	    fprintf(stderr, "Remapped histogram :\n");

	    //for (j = 0; j<0x100; j++) {
	    //fprintf(stderr, " %d:%g", j, remappedSourceHistogram[j]);
	    //}
	    //fprintf(stderr, "\n");

	    // Add source Histogram to the Accumulated

	    for (j = 0; ( j <= 0xff ); j ++) {
  
	      accumSourceHistogram[j] += remappedSourceHistogram[j];

	    }

	    // Add target histogram to its accumulated
	    // This guarantees that both accum histograms have the same total
	    // number of points


	    ptrHistogram = calla->ptrHistograms[currentIntersection].ptrBaseHistograms;
  
	    for (j = 0; j <= 0xff; j ++) {
  
	      array = ptrHistogram[channel];
	      accumToCorrectHistogram[j] += array[j];

	    }

	    continue;

  
	  } else { //	

	    assert(currentHistogram == &calla->ptrHistograms[currentIntersection]);

	    if (currentHistogram->otherImageNumber == currentImageNumber  &&
		processedImages[currentHistogram->baseImageNumber] != 0 ) {

	      // MIRROR version of the code above.
	      // In this case baseImageNumber is done
	      // and otherImageNumber is to be done

	      //fprintf(stderr, "Original histogram Channel %d [Base %d,%d]: \n", channel, currentImageNumber, currentHistogram->baseImageNumber);
	      
	      //for (j = 0; j<0x100; j++) {
	      //fprintf(stderr, " %d:%d", j, currentHistogram->ptrBaseHistograms[channel][j]);
	      //}
	      //fprintf(stderr, "\n");
	      
	      // Remap source histogram
	      RemapHistogram(currentHistogram->ptrBaseHistograms[channel],
			     remappedSourceHistogram,
			     &(calla->magnolia[currentHistogram->baseImageNumber]), 
			     channel);
	      
	      //fprintf(stderr, "Remapped histogram: \n");
	      
	      //for (j = 0; j<0x100; j++) {
	      //	fprintf(stderr, " %d:%g", j, remappedSourceHistogram[j]);
	      //}
	      //fprintf(stderr, "\n");
	      
	      // Add it to the accumulated
	      for (j = 0; j <= 0xff; j ++) {
		
		accumSourceHistogram[j] += remappedSourceHistogram[j];
		
	      } //      for (j = 0; j <= 0xff; j ++) {
	      
	      ptrHistogram = currentHistogram->ptrOtherHistograms;
	      
	      
	      // add target histogram to its accumulated
	      // This guarantees that both accum histograms have the same total
	      // number of points
	      for (j = 0; j <= 0xff; j ++) {
		
		array = ptrHistogram[channel];
		accumToCorrectHistogram[j] += array[j];
		
	      } // for (j = 0; j <= 0xff; j ++) {
	      
	    } // end of if

	  } // end of else
	} // end of if 
	  
      } //    for (currentIntersection = 0;   %currentIntersection < numberIntersections ; currentIntersection++) {

  
      ComputeAdjustmentCurve(accumToCorrectHistogram,
			     accumSourceHistogram,
			     (calla->magnolia[currentImageNumber].fieldx04)[channel]);
      
    } //    for (channel = 0; var < 6; var++) {



     processedImages[currentImageNumber] = 1;

  } // while (Unknown43 (...) != -1);

  for (i =0 ; i< calla->numberImages; i++) {
    // are all the images opimized?
    assert(processedImages[i]);
  }


  free(processedImages);

  free(remappedSourceHistogram);

  free(accumToCorrectHistogram);

  free(accumSourceHistogram);
  
  return 1;
  
}




// Returns an array of (n * n-1) /2 daisies, with the histograms

histograms_struct *ReadHistograms (fullPath *fullPathImages, int numberImages)
{

  int value;

  unsigned char *ptrCurrentLineBuffer;
  unsigned char *ptrCurrentPixelLineBuffer;
  histograms_struct *ptrHistograms; //     << arrays of n * (n-1)/2 daisies
  int bytesPerLine;
  int  bitsPerPixel;
  unsigned char *imagesDataBuffer; // numberOfImages * bytesPerLine
  int bytesPerPixel;
  int  currentPixel;
  int currentRow;
  int otherImage;
  int currentImage;
  TIFF **ptrTIFFs;
  uint16 samplesPerPixel;
  uint16 bitsPerSample;
  uint32 imageLength;
  uint32 imageWidth;
  char  tempString[512];
  char  tempString2[512];
  int *ptrInt;
  histograms_struct * currentHistogram;
  histograms_struct * saveReturnValue;

  int temp;

  int i;

  int totalPixels = 0;

  //  esi = fullPathImages;

  // (n * n-1)/2

  if ( numberImages == 0 )
    return 0;

  if ( ptQuietFlag == 0 ) {
    
    Progress(_initProgress, "Reading Histograms");
    
  }

  saveReturnValue = ptrHistograms = calloc(numberImages * (numberImages-1)/2, sizeof(histograms_struct)); // Allocates one per every intersection n * (n-1) /2

  if ( ptrHistograms == NULL )
    return 0;
  
  ptrTIFFs = calloc(numberImages , sizeof(TIFF*));
  
  if ( ptrTIFFs == NULL )
    return 0;
  
  currentImage = 0;

  // Open TIFFs
  for (currentImage = 0;  currentImage < numberImages ; currentImage ++ ) {

    if (GetFullPath(&fullPathImages[currentImage],tempString) != 0) {
      PrintError("Could not get filename");
      return(0);
    }
    
    if ((ptrTIFFs[currentImage] = TIFFOpen(tempString, "r")) == NULL) {
      sprintf(tempString2, "Coult not open TIFF file [%s]", tempString);
      PrintError(tempString2);
      return NULL;
    }
    
  } // for loop
  
  // Set defaults for all images (assumes they have all the same
  TIFFGetField(ptrTIFFs[0], TIFFTAG_IMAGEWIDTH, &imageWidth);
  TIFFGetField(ptrTIFFs[0], TIFFTAG_IMAGELENGTH, &imageLength);
  TIFFGetField(ptrTIFFs[0], TIFFTAG_BITSPERSAMPLE, &bitsPerSample);
  TIFFGetField(ptrTIFFs[0], TIFFTAG_SAMPLESPERPIXEL, &samplesPerPixel); // 0x11c

  bitsPerPixel = samplesPerPixel * bitsPerSample;

  bytesPerLine = TIFFScanlineSize(ptrTIFFs[0]);

  bytesPerPixel = (bitsPerPixel + 7 ) / 8;

  imagesDataBuffer = calloc(numberImages, bytesPerLine);

  if ( imagesDataBuffer == 0 ) {
    PrintError("Not enough memory");
    return NULL;
  }

  currentHistogram = ptrHistograms;
  currentImage = 0;

  // Initializes the data structure

  for (currentImage = 0; currentImage < numberImages; currentImage++) {
    
    for (otherImage = currentImage + 1; otherImage < numberImages ; otherImage++, currentHistogram++) {
      
      currentHistogram->overlappingPixels = 0;
      
      currentHistogram->bytesPerSample = (bitsPerSample+7)/8;
      
      currentHistogram->numberDifferentValues = 0x100;
      
      currentHistogram->baseImageNumber = currentImage;
      
      currentHistogram->otherImageNumber = otherImage;
      
      
      for (i = 0 ;  i < 6 ; i++) {
        
        if ((currentHistogram->ptrBaseHistograms[i] = calloc(currentHistogram->numberDifferentValues, sizeof(int))) == NULL)
	  return 0;
        
        if ((currentHistogram->ptrOtherHistograms[i] = calloc(currentHistogram->numberDifferentValues,sizeof(int))) == NULL)
          return 0;
        
      } // for
      
    } //for (otherImage = currentImage + 1; otherImage < numberImages ; otherImage++, currentHistogram++) 
    
  } // for (currentImage = 0; currentImage < numberImages; currentImage++) {
  
  //  fprintf(stderr,"Width %d Length %d BytesPerPixel %d per line%d\n", imageWidth, imageLength, bytesPerPixel, bytesPerLine);

  for (currentRow = 0; currentRow < imageLength; currentRow ++) {

    if (currentRow * 2 == (int)(currentRow / 5.0) * 10) {

      // This probably means every 2 percent update progress
      
      sprintf(tempString, "%d", (int)(currentRow * 100/imageLength));
      
      if ( ptQuietFlag == 0 ) {
        if (Progress(_setProgress, tempString) == 0) {
          
          for (currentImage = 0 ;  currentImage < numberImages ; currentImage++) {
            
            TIFFClose(ptrTIFFs[currentImage]);
            
          } //for
          return 0;

        } // progresss
        
      } // quiet flag
      
    }//    if ( %ecx == %eax ) {
    

    ptrCurrentLineBuffer = imagesDataBuffer;

    // Read the current row from each image
    for (currentImage=0; currentImage < numberImages; currentImage++) {
      
      TIFFReadScanline(ptrTIFFs[currentImage], ptrCurrentLineBuffer, currentRow, 0);
      
      RGBAtoARGB(ptrCurrentLineBuffer, imageWidth, bitsPerPixel);
      
      ptrCurrentLineBuffer+= bytesPerLine;
      
    } // while (currentImage < numberImages) 
    
    ptrCurrentPixelLineBuffer = imagesDataBuffer;
    
    //for each pixel in the current line...

    for (currentPixel = 0;  currentPixel < imageWidth; currentPixel++, ptrCurrentPixelLineBuffer+= bytesPerPixel ) {

      unsigned char *ptrPixel;
      unsigned char *ptrOtherPixel;
      int ecx;

      // We process each currentHistogram
      currentHistogram = ptrHistograms;

      currentImage = 0;

      ptrPixel = ptrCurrentPixelLineBuffer;
      assert(ptrPixel < imagesDataBuffer + numberImages * bytesPerLine);

      ecx = numberImages;

      // for each pixel of each line of each image...
      for (currentImage = 0; currentImage < numberImages ; currentImage++, ptrPixel+= bytesPerLine) {

	assert(ptrPixel < imagesDataBuffer + numberImages * bytesPerLine);

	ptrOtherPixel = ptrPixel + bytesPerLine;

	// for each pixel of each line of each current image, and images 'above' the current

	for (otherImage = currentImage + 1;  otherImage < numberImages; otherImage++, ptrOtherPixel += bytesPerLine) {

	  assert(ptrOtherPixel < imagesDataBuffer + numberImages * bytesPerLine);

	  assert(ptrPixel < ptrOtherPixel);
	  assert(((int)(ptrOtherPixel - ptrPixel)) % bytesPerLine == 0);

	  /* Only process if the alpha channel is not zero in both pixels*/

	  if (0 != ptrPixel[0]  &&  0 != ptrOtherPixel[0] ) {

	    totalPixels ++;
	    currentHistogram->overlappingPixels++;
	      
	    // esi == ptrPixel
	    // ebx == ptrOtherPixel


	    // This seems to record the frequency of every pixels of every channel one of 3 channels of every image
	    for (i = 0;  i < 3; i++) {
	      
	      // First byte is alpha channel

	      value  = (unsigned char)ptrPixel[i+1];
	      assert(value >= 0 && value <= 0xff);

	      ptrInt = currentHistogram->ptrBaseHistograms[i];
		
	      ptrInt[value] ++;
	      
	      value = (unsigned char)ptrOtherPixel[i + 1];
	      assert(value >= 0 && value <= 0xff);

	      ptrInt = currentHistogram->ptrOtherHistograms[i] ; // eax = ptrInt
	      
	      ptrInt[value] ++;
	      
	    } 

	    // compute the other 6 histograms	      
	    temp = Cherry(ptrPixel[1], ptrPixel[2], ptrPixel[3]);
	    assert(temp >= 0 && temp <= 0xff);
	    ptrInt = currentHistogram->ptrBaseHistograms[3];
	    ptrInt[temp] ++;

	    temp = Cherry(ptrOtherPixel[1], ptrOtherPixel[2], ptrOtherPixel[3]);
	    assert(temp >= 0 && temp <= 0xff);
	    ptrInt = currentHistogram->ptrOtherHistograms[3];
	    ptrInt[temp] ++;

	    temp = Apple(ptrPixel[1], ptrPixel[2], ptrPixel[3]);
	    assert(temp >= 0 && temp <= 0xff);
	    ptrInt = currentHistogram->ptrBaseHistograms[4];
	    ptrInt[temp] ++;

	    temp = Apple(ptrOtherPixel[1], ptrOtherPixel[2], ptrOtherPixel[3]);
	    assert(temp >= 0 && temp <= 0xff);
	    ptrInt = currentHistogram->ptrOtherHistograms[4];
	    ptrInt[temp] ++;

	    temp = Peach(ptrPixel[1], ptrPixel[2], ptrPixel[3]);
	    assert(temp >= 0 && temp <= 0xff);
	    ptrInt = currentHistogram->ptrBaseHistograms[5];
	    ptrInt[temp] ++;

	    temp = Peach(ptrOtherPixel[1], ptrOtherPixel[2], ptrOtherPixel[3]);
	    assert(temp >= 0 && temp <= 0xff);
	    ptrInt = currentHistogram->ptrOtherHistograms[5];
	    ptrInt[temp] ++;

	  } // if ( 0 != *ptrPixel  and  0 != *ptrOtherPixel ) {


	  currentHistogram++;  //edi = edi + 0x44; //68 ?? again, this should be edi ++;
	  
	}  //	for (otherImage = currentImage + 1;  otherImage < numberImages; otherImage++ ) {

      }//      for (currentImage = 0; currentImage >=numberImages ;... 
      
    } //    for (currentPixel = 0;  currentPixel < imageWidth; currentPixel++... ) {
    
  } //for (currentRow = 0; currentRow < imageLength; currentRow ++) {

  
  //  for (i = 0; i< numberImages * (numberImages-1)/2; i++) {
  //    fprintf(stderr, "Histogram %d Images %d %d, %d Pixels\n", i , 
  //	    ptrHistograms[i].baseImageNumber, 
  //	    ptrHistograms[i].otherImageNumber,
  //	    ptrHistograms[i].overlappingPixels);
  //    totalPixels2 += ptrHistograms[i].overlappingPixels;
  //  }

  //  assert(totalPixels2 == totalPixels);

  for (currentImage=0;  currentImage < numberImages; currentImage++) {

    TIFFClose(ptrTIFFs[currentImage]);

  }

  free(ptrTIFFs);
  free(imagesDataBuffer);

  return(saveReturnValue);
}

unsigned char Cherry (unsigned char parm0, unsigned char parm1, unsigned char parm2)
{

  unsigned temp = (parm0 + parm1 + parm2)/3;

  assert(temp >= 0 && temp <= 255);
  
  return temp;
}

unsigned char Peach (unsigned char parm0, unsigned char parm1, unsigned char parm2)
{
  unsigned temp;

  temp =  ((parm0 - parm2) + 0x100)/2;

  if (temp < 0) {
    return 0;
  }
  if (temp > 0xff ) {
    return 0xff;
  }
  return temp;
}


unsigned char Apple (unsigned char parm0, unsigned char parm1, unsigned char parm2)
{
  int temp;

  temp = ((parm0 - parm1) + 0x100)/2;
  if (temp < 0) {
    return(0);
  }
  
  if (temp > 0xff) {
    return(0xff);
  }
  return temp;
    
}



//int ComputeColourBrightnessCorrection(calla_struct *calla)
//{
//
//
//  // var32  0xffffffe0(%ebp) var32
//  // var28  0xffffffe4(%ebp) double *var28
//  // var24  0xffffffe8(%ebp) var24
//  // var20  0xffffffec(%ebp) int *correctedImages
//  // var16  0xfffffff0(%ebp) currentImageNumber
//  // var12  0xfffffff4(%ebp) channel
//  // var08  0xfffffff8(%ebp) int numberIntersections
//
//
//  numberIntersections = (calla->numberImages - 1) * calla->numberImages;
//
//
//  correctedImages = calloc(calla->numberImages, sizeof(int));
//
//
//  edi = malloc(0x100 * sizeof(double));
//  var24 = malloc(0x100 * sizeof(double));
//  var28 = malloc(0x100 * sizeof(double));
//
//
//  if ( correctedImages == 0 )
//    return 0;
//  
//  
//  if ( var24 == 0 ) 
//    return 0;
//  
//  
//  if ( edi == 0 ) 
//    return 0;
//  
//  
//  
//  eax = calla->indexReferenceImage;
//
//  
//  correctedImages[calla->indexReferenceImage] = 1;
//
//  while ((currentImageNumber = Unknown43(correctedImages, calla)) != -1) {
//
//    assert(currentImageNumber > 0);
//    assert(currentImageNumber < calla->numberImages);
//    assert(correctedImages[currentImageNumber] == 0);
//    
//    channel = 0;
//
//    for (channel = 0; var < 6; var++) {
//
//
//      for (ecx =0; ecx < 0xff;  ecx ++) {
//
//	edi[ecx] = 0;
//	var24[ecx] = 0;
//
//      }
//
//
//      // for each daisy records (histograms)
//      for (esi = 0;   esi < numberIntersections ; esi++) {
//
//	currentHistogram = &calla->ptrHistograms[esi];
//
//      
//	if ( currentHistogram->overlappingPixels > 1000 ) { // it is not consistent XXX
//	
//	  if ( currentHistogram->baseImageNumber == currentImageNumber && 
//
//	       correctedImages[currentHistogram->otherImageNumber] != 0 ) {
//
//
//	    // Do something with the already corrected histogram
//	    Unknown37(currentHistogram->ptrOtherHistograms[channel],
//		      var28,  
//		      calla->magnolia[currentHistogram->otherImageNumber], channel);
//
//
//	    for (ecx = 0; ( %ecx <= $0xff ); ecx ++) {
//  
//	      edi[ecx] += var28[ecx];
//  
//	    }
//
//	    // Now process the current histogram
//
//	    ptrHistogram = calla->ptrHistograms[esi].ptrBaseHistograms;
//  
//	    for (ecx = 0; ecx <= 0xff; ecx ++) {
//  
//	      array = ptrHistogram[channel];
//	      var24[ecx] += array[ecx];
//
//	    }
//	    
//	  } else { //	if ( 0xc(%ecx, edx, 1) == %ebx ) {
//
//
//	    currentHistogram = &calla->ptrHistograms[esi];
//
//
//	    if (currentHistogram->otherImageNumber == currentImageNumber ) { 
//  
//	      if ( correctedImages[currentHistogram->baseImageNumber] != 0 ) {
//
//		Unknown37(currentHistogram->ptrBaseHistograms[channel],
//			  var28,
//			  calla->magnolia[currentHistogram->baseImageNumber], channel);
//
//		for (ecx = 0; ecx <= 0xff; ecx ++) {
//
//		  edi[ecx] += var28[ecx];
//
//  
//		} //      for (ecx = 0; ecx <= 0xff; ecx ++) {
//
//		
//		currentHistogram = calla->ptrHistograms[esi];
//		
//		ptrHistogram = currentHistogram->ptrOtherHistogram;
//
//  
//		for (ecx = 0; ecx <= 0xff; ecx ++) {
//
//		  array = ptrHistogram[channel];
//
//		  var24[ecx] += array[ecx];
//
//
//		} //      for (ecx = 0; ecx <= 0xff; ecx ++) {
//
//	      } //	  if ( (%ebx,eax,4) != 0 ) {
//
//	    } //	if ( 0x10(%ecx,edx,1) == %ebx ) {
//	  } // end of else
//	} //if ( (%ecx, edx, 1) > $0x3e8 ) {  
//	  
//      } //    for (esi = 0;   %esi < numberIntersections ; esi++) {
//
//      // This should be responsible for updating the current correction table
//      // for the currentimage
//      
//      Unknown41(var24,
//		edi,
//		(calla->magnolia[currentImageNumber].fieldx04)[channel]);
//
//
//
//    } //    for (channel = 0; var < 6; var++) {
//
//  
//    correctedImages[currentImageNumber] = 1;
//
//  } // while (Unknown43 (...) != -1);
//
//  for (i =0 ; i< calla->numberImages; i++) {
//    // are all the images opimized?
//    assert(correctedImages[i]);
//  }
//
//  free(var20intar);
//
//  free(var28);
//
//  free(var24);
//
//  free(edi);
//
//  return 0;  
//
//  
//}
//
//


void CorrectImageColourBrigthness(Image *image, magnolia_struct *magnolia, int parm3)
{

  int currentRow;
  int currentPixel;
  unsigned char *pixel;
  int edi;
  double * (mappingCurves[6]);
  int edx;
  unsigned char *ptrPixel;
  int channel;
  int level;
  
  for (channel = 0; channel < 6; channel++ ) {
    
    if ((mappingCurves[channel] = calloc(0x100 , sizeof(double))) == NULL) {
      PrintError("Not enough memory\n");
      return ;
    }
    
  } 
  
  edi = 0;
  
  for (level = 0; level < 0x100; level++) {
    
    for (channel = 0; channel< 6; channel ++ ) {
      double *ptr;
      // takes an array of 0x100 doubles, a double between 0 and 0x100, and  number of doubles
      // Remaps the correction functions

      ptr = mappingCurves[channel];

      ptr[level] = (*magnolia->function)(magnolia->fieldx04[channel], level, magnolia->components);
      
    }

  } //  for (edi = 0; edi < 0x100; edi++) {

  pixel = *image->data;

  switch (parm3) {

  case 1:
    
    assert(0);

    currentRow = 0;
    
    edx = currentRow;
    
    while ( currentRow < image->height) {

      currentPixel = 0;
      
      ptrPixel = pixel;
      
      while ( currentPixel < image->width ) {
	
	
	if (*ptrPixel != 0 ) {
	  
	  int ebx,ecx;

	  ebx = Cherry(ptrPixel[1], ptrPixel[2], ptrPixel[3]); // adds three channels and averages
	  
	  edx = (char)Unknown40(ebx, mappingCurves[4]) - ebx;

	  ecx = ptrPixel[1] + edx;

	  if (ecx < 0) {
	    ptrPixel[1] = 0;
	  }  else if (ecx > 0xff) {
	    ptrPixel[1] = 0xff;
	  }  else {
	    ptrPixel[1] = ecx;
	  }
	  
	  ecx = ptrPixel[2] + edx;
	  
	  if (ecx < 0) {
	    ptrPixel[2] = 0;
	  }  else if (ecx > 0xff) {
	    ptrPixel[2] = 0xff;
	  }  else {
	    ptrPixel[2] = ecx;
	  }
	  
	  ecx = ptrPixel[3] + edx;
	  
	  if (ecx < 0) {
	    ptrPixel[3] = 0;
	  }  else if (ecx > 0xff) {
	    ptrPixel[3] = 0xff;
	  }  else {
	    ptrPixel[3] = ecx;
	  }

	}

	currentPixel++;
	ptrPixel+=4; // advance an entire pixel
	
      } // while ( currentPixel < image->width )
      
      currentRow++;
      
      pixel += image->bytesPerLine;
      
    } //while ( currentRowx < image->height) {
    break;

    
  case 0: //case of switch
      
    ptrPixel = *(image->data);

    for (currentRow = 0;  currentRow < image->height; currentRow++)  {
      
      for (currentPixel = 0 ; currentPixel < image->width ; currentPixel++) {
	
	if ( ptrPixel[0] != 0 ) {
	  
	  for (channel = 0; channel < 3 ; channel ++) {
	    
	    ptrPixel[channel+1] = Unknown40(ptrPixel[channel+1], mappingCurves[channel]);

	  }
	  
	} //      if ( (ptrPixel) != 0 ) {

	ptrPixel +=4;
	
      } //      for (currentPixel = 0 ; currentPixel < imageWidth ; currentPixel++) {
    }
    break;
    
  case 2: // case of switch parm3
    
    assert(0);

    currentRow = 0;
    
    for (currentRow = 0; currentRow < image->height; currentRow++) {

      ptrPixel = pixel;
    
      for (currentPixel = 0; currentPixel < image->width; currentPixel++) {

	if (ptrPixel[0] != 0) {

	  int ebx, var49, var56, esi;

	  ebx =  Cherry(ptrPixel[1], ptrPixel[2], ptrPixel[3]);	  
	  
	  var49 = Unknown40( Apple(ptrPixel[1], ptrPixel[2], ptrPixel[3]), 
				    mappingCurves[5]); //var08
	  
	  var56 = Unknown40(Peach(ptrPixel[1], ptrPixel[2], ptrPixel[3]), 
				   mappingCurves[6]); // var04
	  esi = var49;

	  ptrPixel[1] = Unknown47(ebx, var49, var56);
	  
	  ptrPixel[2] = Unknown48(ebx, var49, var56);
	  
	  ptrPixel[3] = Unknown49(ebx, var49, var56);

	} //	if (ptrPixel[0] != 0) {

	ptrPixel += 4;

      } //      for (currentPixel = 0; currentPixel < image->width; currentPixel++) {
      pixel += image->bytesPerLine;

    } //    for (currentRow = 0; currentRow < image->height; currentRow++) {
    break;

  }


  for (edi = 0;edi < 6; edi++) {

    free(mappingCurves[edi]);

  }


}//end of function



double MapFunction(double parm[], double x, int n)
{
  double x_1; 
  int e;
  double result;
  /* 

  Assume we have a curve defined from [0 to n-1] but it is
  discretized. So we actually have an array p[] of n elements that
  defines this curve (called it p). The curve grows monotonically,
  from 0 to n-1. 

  
  We also have the same curve, but "stretched" from [0 to 255]. Call it
  f. We have a point x in this range. Now we need to find f(x)

  So we need to find the corresponding x_1 in the domain of p.

  But x_1 might fall in between 2 different integers e and e+1. We
  assume the cuve is a straight line between these two points:

  e = floor(x_1);

  The result y is:

  y = p(x_1) = ((x_1 - e ) * (p[e+1] - p[e]))     +  p[e]
  
  */

  x_1 = (x * 255.0) / ( n - 1 );

  e = floor(x_1);
  
  if ( e < 0 ) {

    result = parm[0];

  } else if  ( e < n - 1 ) {
    assert(e < n);
    assert(e >= 0);
    
    result = ((x_1 - e) * (parm[e+1] - parm[e])) + parm[e];

    assert( result >= parm[e]);
    
  } else {
    
    result = parm[n-1];
  }


  if (result >= 0x100) {
    // THIS CODE IS JUST FOR DEBUGGING PURPOSES

    int i;
    fprintf(stderr, "Result %g Value %d Array: ", result, n);
    
    for (i=0; i<= 0xff; i++) {
      fprintf(stderr, "%d: %g ", i, parm[i]);
    }
    fprintf(stderr, "\n");
    assert(0);
  }


  return result;


  // should return a double
  
} // end of function Unknown33




int Unknown40(int value, double mapTable[]) 
{
  
  double delta;

  double deltaPrev;
  double deltaNext;

  double var48;
  int var44;

  double tablePrevValue;
  double tableNextValue;
  int tempInt;
  int nextValueInt;
  int prevValueInt;

  int edx;

  // Find previous and next. Extrapolate if necessary

  if ( value == 0 ) {

    tablePrevValue = 2* mapTable[0]  - mapTable[1]; // XXXXXXXXXXXXXXXXX This just looks backwards!

  } else {
    
    tablePrevValue = mapTable[value-1];

  }

  if ( value == 0xff ) {

    tableNextValue = 2 * mapTable[255] - mapTable[254];


  } else {

    tableNextValue = mapTable[value+1];
    
  }

  if (abs(tableNextValue - tablePrevValue) <= 2.0) {
    // if the difference |f(value - 1)  -f(value+1) is too small

    tempInt =  (int)mapTable[value];

    if ((int)mapTable[value] == 0xff ) {
      return 0xff;
    }

    delta = mapTable[value] - (int)mapTable[value];

    //  chance of being one colour or the next. 
    // of n points, n * delta = base and n(1-*delta) will be base + 1
    // this guarantees that the distribution is faithfull. clever.

    if ( delta * INT32_MAX < rand() ) {
      //      if (C0 == 1)
      return (int)mapTable[value];

    } else {
    
      return ((int)mapTable[value]) + 1;
    
    }
    assert(0); // nothing should reach here

  } //  if (value ... 2.0) {

  // THIS CASE IS WHEN THE TANGENT is > 1

  nextValueInt = (int)tableNextValue;

  if ( (int)tableNextValue > 0xff ) {

    nextValueInt = 0xff;

  }

  prevValueInt = (int)tablePrevValue;

  if ( (int)tablePrevValue < tablePrevValue ) {
    prevValueInt ++;
  }

  // prevValueInt == ceiling(tablePrevValue)

  if ( prevValueInt < 0 ) {
    prevValueInt = 0;
  }


  deltaPrev =  mapTable[value] - tablePrevValue;
  deltaNext = tableNextValue - mapTable[value];

  edx = prevValueInt;

  var48 = 0;
  var44 = 0;

  //  if ( %edx > %nextValueInt ) /// [(int)tablePrevValue] > [(int)tableNextValue] 
  while ( edx <= nextValueInt ) { /// [(int)tablePrevValue] > [(int)tableNextValue] 

    if (edx < mapTable[value]) {
      var48 += (edx - tablePrevValue)/deltaPrev;
    } else {
      var48 += (tableNextValue - edx)/deltaNext;
    }
    edx ++;
  } // while...

  // where did edx = ebx go?

  var48 = rand() * var48 / INT32_MAX;

  edx = prevValueInt;

  while ( edx <= nextValueInt ) {

    if (edx < mapTable[value]) {
      var48 -= (edx - tablePrevValue) / deltaPrev;
    } else { //    if ( %ah == 0x1 ) {
      var48 -= (tableNextValue - edx) / deltaNext;
    }  //   if ( %ah == 0x1 ) 

    if ( var48 < 0 ) {
      return edx;
    }
    edx ++;

  } // while ( %edx <= %nextValueInt ) 

  return nextValueInt;

}


unsigned char Unknown47(unsigned char parm0, unsigned char parm1, unsigned char parm2)
{

  int eax = parm1;
  int edx = parm2;
  int ecx = parm0;


  ecx = ecx * 3;
  ecx = ecx + 2 * eax - 256;
  ecx = (ecx + 2 * edx - 256) * 2 /3;

  if (ecx >= 0) {
    if (ecx > 0xff) 
      return 0xff;
    else
      return ecx;
  } else {
    return 0;
  }
}



unsigned char Unknown48(unsigned char parm0, unsigned char parm1, unsigned char parm2) 
{
  //     (3a - 4b + 512 + 2c - 256) * (2/3)
  //  => (3a - 4b + 2c + 256) * (2/3)
  //  if a = b = c
  // =>  (3a -4a + 2a + 256) * (2/3 => (a +256) * 2/3


  int ecx;

  ecx = parm0 * 3 - (parm1 * 4 - 512);

  ecx = (ecx + 2 * parm2  - 256);
    
  ecx = (ecx * 2)/3;

  if (ecx < 0) {
    return 0;
  } else {
    if (ecx > 0xff)
      return 0xff;
    else
      return ecx;
  }
}



unsigned char Unknown49(unsigned char parm0, unsigned char parm1, unsigned char parm2)
{
  return Unknown48(parm0, parm1, parm2) ;
}



void RemapHistogram(int *histogram, double *remappedHistogram, magnolia_struct *magnolia, int channel)
{

  int index;
  double mappingFunction[256];

  double prevValue;
  double nextValue;

  int value;

  double delta;



  //  fprintf(stderr, "Doubles remappedHistogram: \n");

  // Compute the mapping function.

  //  fprintf(stderr, "Doubles array: \n");
  for (index = 0; index < 0x100 ; index++) {
    
      mappingFunction[index] = (*magnolia->function)(magnolia->fieldx04[channel], (double)index, magnolia->components);

      // DEBUGGING code
      if ((int)mappingFunction[index] < 0 || (int)mappingFunction[index] > 0xff) {
	fprintf(stderr, "error %d %g\n", index, mappingFunction[index]);
	assert(0);
      }

  } //  

  // Initialize the remapped histogram
  for (index = 0; index < 0x100; index ++) {

    remappedHistogram[index] = 0;

  } //if ( %index <= $0xff )

  index = 0;


  prevValue = 0.0;
  nextValue = 0.0;

  for (index = 0; index < 0x100; index++) {

    if (index == 0 ) {

      prevValue = mappingFunction[1] - 2 * mappingFunction[0];
      //      assert((mappingFunction[1] - 2 * mappingFunction[0]) >= 0);
      
    } else { //    if ( %index == 0 ) {

      prevValue = mappingFunction[index - 1]; // makes sense, it would be undefined for index == 0

    }//    if ( %index == 0 ) {

    if ( index == 0xff ) {
      // Extrapolate another value, a[xff] + delta (a[xff] - a[xff-1])
      nextValue = 2 * mappingFunction[0xff] - mappingFunction[0xff-1];
        
    } else { //    if ( %index == $0xff ) {

      nextValue = mappingFunction[index + 1];

    } //    if ( %index == $0xff ) {

    //    ***************************************************

    if (abs(nextValue - prevValue) <=  2.0) { // remember to negate as part of the if jump less

      // RESET stack
      // remove 2 values from the stack
      
      if ( (int)mappingFunction[index] == 0xff ) {
        // REPEATED FROM AAAAAAA
        remappedHistogram[255] = histogram[index] + remappedHistogram[255];
        continue;

      }
      
      //      fprintf(stderr, "PTdouble[%d] = %g\n", index, mappingFunction[index]);
      assert(mappingFunction[index] >= 0 && mappingFunction[index] <= 0xff);
      
      
    } else { // if (top stack (and pop) ??  2.0) { // remember to negate as part of the if jump less

      int ecx;
      int var2072;
      int edx;

      //////////////////////////////////////////////////////////
      double st_0;

      ecx = (int)nextValue;
      
      if ((int)nextValue > 0xff ) {
        ecx = 0xff;
      } 

      edx = (int)prevValue;//TOP


      if (edx < prevValue) { 
        edx ++;
      } //if 
      assert(edx == ceil(prevValue));

      if ( edx < 0 ) {
        edx = 0;
      } //      if ( %edx < 0 ) {


      st_0 = 0;

      
      for (var2072 = edx ; var2072 <= ecx; var2072++ ) {
	
	if (var2072 < mappingFunction[index]) {  // I AM IN DOUBT of this one
	  
	  if ( (mappingFunction[index] - prevValue) != 0.0 ) {
	    assert(mappingFunction[index] - prevValue > 0);
	    //
	    st_0 += (var2072 - prevValue)/(mappingFunction[index] - prevValue);
	    
	  }
	  continue;
	  
	} else { //     if (var2072 < mappingFunction[index])
	  
	  if ( 0.0 != (nextValue - mappingFunction[index]) ) {
	    assert(nextValue - mappingFunction[index] > 0);
	    st_0 += (nextValue- var2072) / (nextValue - mappingFunction[index]);
	  }
	  continue;
	  
	} //    if (var2072 < mappingFunction[index])
	
	assert(0); // it should not reach here
	
	
      } // for loop
      
      if (0.0 != st_0 ) {

	for (var2072 = edx; var2072 <= ecx; var2072++) {

	  if (var2072 < mappingFunction[index]) {
	
	    if (mappingFunction[index] - prevValue != 0.0) {

	      remappedHistogram[var2072] += (((var2072 - prevValue) / (mappingFunction[index] - prevValue)) * histogram[index])/st_0;
	  
	    } //    
	    continue;
	
	  } else {
	    
	    if ((nextValue - mappingFunction[index]) != 0.0) {
	      
	      // ????????????? it was remappedHistogram[edi]
	      remappedHistogram[var2072] += (((nextValue - var2072) / (nextValue - mappingFunction[index]) * histogram[index]))/st_0;
	      
	      continue;
	      
	    } //if
	  } //if 
	} //for
	continue; //???
      }


      if ( (int) mappingFunction[index] == 0xff ) {
        
        remappedHistogram[255] = histogram[index] + remappedHistogram[255];
        
        continue;
        
        
      } //       if ( (int)      mappingFunction[index] == $0xff ) {

      assert(st_0 == 0);

      //////////////////////////////////////////////////////////

    } //// if (top stack (and pop) ??  2.0) { // remember to negate as part of the if jump less


    value = (int) mappingFunction[index];

    delta = mappingFunction[index] - (int) mappingFunction[index];

    // delta determines the rounding error. 1-delta*histogram gets
    // added to the current value and
    // delta *histogram to the next value.

    remappedHistogram[value]   = (1 - delta) *histogram[index] +  remappedHistogram[value];


    remappedHistogram[value+1] = delta * histogram[index] + remappedHistogram[value+1];
    

  } //  for (index = 0; index < 0x100; index++) {


}


void ComputeAdjustmentCurve(double *sourceHistogram, double *referenceHistogram, double *curve) 
{
  // Unknown41

  double  copyReferenceHistogram[0x100];
  double copySourceHist[0x100];
  double otherArray[0x100];
  int i,j;

  double sum2 = 0.0;
  double sum = 0.0;
  //  double total = 0;

  // FIRST, make sure we parameters are sound

  // Should the two histograms have the same accumated total?

  double total1 = 0;
  double total2 = 0;
  for (i=0;i<0x100;i++) {
    assert(sourceHistogram[i]>= 0);
    assert(referenceHistogram[i]>= 0);
    total1 += sourceHistogram[i];
    total2 += referenceHistogram[i];
    
  }
  //  if (total1 != total2) {
  //       fprintf(stderr, "%g %g difference %g\n", total1, total2, total1 - total2);
  //  }

  //  assert(total1 == total2);
  

  // FIRST COPY THE histograms to a temporal place.


  memcpy(copySourceHist, sourceHistogram, 0x200 * 4); // Check this one ???
  memcpy(copyReferenceHistogram, referenceHistogram, 0x200 * 4); // Check this one ???


  //// Debugging messages

//  fprintf(stderr, "41->Histogram source: ");
//  for (i=0; i<0x100;i++) {
//    fprintf(stderr, " %d:%g ", i, copySourceHist[i]);
//    assert(copySourceHist[i] == sourceHistogram[i]);
//    total += copySourceHist[i];
//  }
//  fprintf(stderr, "\nTotal: %g\n", total);
//
//  total = 0;
//  fprintf(stderr, "41->Histogram Reference : ");
//  for (i=0; i<0x100;i++) {
//    fprintf(stderr, " %d:%g ", i, copyReferenceHistogram[i]);
//    assert(copyReferenceHistogram[i] == referenceHistogram[i]);
//    total += copyReferenceHistogram[i];
//  }
//  fprintf(stderr, "\nTotal: %g\n", total);
//

  for (i = 0;  i <= 0xff; i ++ ) {
    
    double st_0 = copySourceHist[i];
    

    for (j = 0; j <= 0xff; j ++) {
      
      if ( st_0 == 0.0  ) {

	otherArray[j] = 0;

      } else if (st_0 < copyReferenceHistogram[j] ) {

	otherArray[j] = st_0;
	copyReferenceHistogram[j] -= st_0;
	
	st_0 = 0.0;
	
      } else {
	//	
	otherArray[j] = copyReferenceHistogram[j];
	
	st_0 -= copyReferenceHistogram[j];
	
	copyReferenceHistogram[j] = 0.0;
	
      }

    }//   for (j = 0; j <= 0xff; j ++) {

    sum = 0.0;
    for (j = 0; j <= 0xff;j++) {

      sum += otherArray[j];

    }//  if ( %j <= $0xff )

    if ( sum == 0.0 ) {
      // Any value with sum 0 should be interpolated
      // between 2 other values
      // This is done at the end. In the meantime, mark it 
      // with a -1
      if ( i == 0 ) {

        curve[0] = 0;

	continue;

      } else { 

        if ( i == 0xff ) {

          curve[0xff] = 255.0;

	  continue;

        }
	curve[i] = -1.0;

      }
      assert(	curve[i] == -1.0);
    }  else { // sum != 0.0

      assert(sum != 0.0);
      //      fprintf(stderr, "Value of sum %g %g \n", sum, otherArray[0]);
      sum2 = 0.0;
      for (j = 0;  j <= 0xff; j ++ ) {

	sum2 += otherArray[j] * j;

      }
      //      fprintf(stderr, "Value of sum2 %g\n", sum2);

      curve[i] = sum2/sum;

    } //else { //if ( ?? ) {


  } // for (i = 0;  i <= 0xff; i ++ ) {


  // LET Us verify the results so far
  for (i=0;i<0x100;i++) {
    assert(curve[i] == -1 || curve[i] >= 0);
    assert(curve[i] < 0x100);
  }

//  fprintf(stderr, "41->Magnolia: ");
//  for (i=0; i<0x100;i++) {
//    fprintf(stderr, " %d:%g ", i, curve[i]);
//  }
//  fprintf(stderr, "\n");
//
//  // Check if the curve got any negative values.

  for (i = 1; i <= 0xfe; i ++) {

    if ( curve[i] == -1.0 ) {
      
      // if the computed value was negative, then 
      // interpolate between its 2 non-negative neighbors

      int j;

      j = i +1;
      
      if ( j <= 0xff ) {

	while ( curve[j] == -1.0  ) {
	  ++j;
	  if (j > 0xff) {
	    break; // 
	  }
	}
      }   //    if ( %j <= $0xff )

      assert(curve[j] >= 0);
      assert(curve[i-1] >= 0);

      curve[i] = curve[i - 1] + (curve[j] - curve[i-1])/(j + 1 - i) ;

    }

  } //  for (i = 1; i <= $0xfe; i ++) {

  // LET Us verify the result
  for (i=0;i<0x100;i++) {
    assert(curve[i] >= 0);
    assert(curve[i] < 0x100);
  }


//  fprintf(stderr, "41->Magnolia: ");
//  for (i=0; i<0x100;i++) {
//    fprintf(stderr, " %d:%g ", i, curve[i]);
//  }
//  fprintf(stderr, "\n");
//
  //  exit(0);


}

#ifdef OLDVERSION

void Unknown41(double *sourceHistogram, double *targetHistogram, double *magnoliaArray) 
{


  double  copyTargetHist[0x100];
  double copySourceHist[0x100];
  double otherArray[0x100];
  double contribution;

  double sum;

  int i,j;

  int edx;


  memcpy(copySourceHist, sourceHistogram, 0x100 * sizeof(double)); // Check this one ???

  memcpy(copyTargetHist, targetHistogram, 0x100 * sizeof(double)); // Check this one ???

  for (i = 0;  i <= 0xff; i ++ ) {

    
    contribution = copySourceHist[i];
    
    for (j = 0; j <= 0xff; j ++) {
      
      if (  copySourceHist[i] == 0.0  ) {

	otherArray[j] = 0;

      } else { //    if (  ??  ) {

	if (copySourceHist[i] < copyTargetHist[j] ) {

	  copyTargetHist[j] -= copySourceHist[i];

	  otherArray[j] = copySourceHist[i];

	  contribution = 0.0;

	} else {

	  otherArray[j] = copyTargetHist[j];


	  contribution = copySourceHist[i] - copyTargetHist[j];

	  copyTargetHist[j] = 0.0;

	}
      } // end of else    if (  ??  ) {


    }//   for (j = 0; j <= 0xff; j ++) {


    sum = 0.0; 


    for (edx = 0; edx <= 0xff;edx++) {

      sum += otherArray[edx];

    }//  if ( %edx <= $0xff )

    if ( sum == 0.0 ) {

      contribution = 0.0;
      
      if ( i == 0 ) {
	
	magnoliaArray[0] = 0;
	
	continue; 
	
      } else { 
	
	if (i == 0xff ) {
	  magnoliaArray[0x7f8/8] = 255.0;
	  continue;
	  
	}
	
	contribution = -1;
	
      }

      assert(contribution == -1);

    }  else { //if ( ?? ) {
      
      
      sum = 0.0;
      for (edx = 0; edx <= 0xff; edx ++ ) {
	sum+= edx  * otherArray[edx];
      }
      contribution /= sum;
      
    } 

    magnoliaArray[i] = contribution;

  } // for (i = 0;  i <= 0xff; i ++ ) {


  for (edx = 1; edx <= 0xfe; edx ++) {
    int ecx;

    if ( magnoliaArray[edx] == -1.0 ) {
      
      ecx = edx +1;
      
      if ( ecx <= 0xff ) {

	while ( magnoliaArray[ecx] == -1.0  ) {
	  if (++ecx > 0xff) {
	    break;
	  }
	}
      }   //    

      magnoliaArray[edx] = magnoliaArray[edx-1] + (ecx - edx -1)/(magnoliaArray[ecx] - magnoliaArray[edx-1]);


    }//  

  } //  for (edx = 1; edx <= $0xfe; edx ++) {

}
#endif


/*
 * replace the extension in filename with
   extension
*/
void ReplaceExt(char* filename, char *extension)
{
  char *temp;
  temp = strrchr(filename, '.');
  if (temp != NULL) {
    strcpy(temp, extension);
  } else {
    strcat(filename, extension);
  }
  return;
}

