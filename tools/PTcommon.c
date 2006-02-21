/*
 *  PTcommon.c
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

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <dirent.h>
#include <unistd.h>
#include <stdint.h>
#include <math.h>

#include <tiffio.h>
#include <filter.h>
#include "panorama.h"


#include "PTcommon.h"


typedef struct {
  uint16 samplesPerPixel;
  uint16 bitsPerSample;
  uint32 imageLength;
  uint32 imageWidth;
  int bytesPerLine;
  int bitsPerPixel;
  uint32 rowsPerStrip;
} pt_tiff_parms;

// This function verifies that all the TIFF files have the same width, length and other
// parameters. Returns TRUE on success.
//

TIFF *OpenTiffFromFullPath(fullPath *file, char *mode)
{
  char tempString[MAX_PATH_LENGTH];
  if (GetFullPath(file, tempString) != 0) {
    PrintError("Could not get filename");
    return NULL;
  }
  return TIFFOpen(tempString, mode);
  
}

int TiffGetImageParameters(TIFF *tiffFile, pt_tiff_parms *tiffData)
{
  assert(tiffFile != NULL);

  if (!TIFFGetField(tiffFile, TIFFTAG_IMAGEWIDTH, &tiffData->imageWidth)) {
    PrintError("File did not include image width information.");
    return 0;
  }
  if (!TIFFGetField(tiffFile, TIFFTAG_IMAGELENGTH, &tiffData->imageLength)) {
    PrintError("File did not include image length information.");
    return 0;
  }

  if (!TIFFGetField(tiffFile, TIFFTAG_BITSPERSAMPLE, &tiffData->bitsPerSample)) {
    PrintError("File did not include bits per sample information.");
    return 0;
  }

  if (!TIFFGetField(tiffFile, TIFFTAG_SAMPLESPERPIXEL, &tiffData->samplesPerPixel)) {
    PrintError("File did not include samples per pixel information.");
    return 0;
  }

  tiffData->bytesPerLine = TIFFScanlineSize(tiffFile);
  if (tiffData->bytesPerLine <= 0) {
    PrintError("File did not include proper bytes per line information.");
    return 0;
  }
  tiffData->bitsPerPixel = tiffData->samplesPerPixel * tiffData->bitsPerSample;
  tiffData->rowsPerStrip = tiffData->imageLength;
  return 1;
}


int TiffGetImageParametersFromPathName(fullPath *filename, pt_tiff_parms *tiffData)
{
  TIFF *file;
  int returnValue;

  assert(filename != NULL);
  assert(tiffData != NULL);
  file = OpenTiffFromFullPath(filename, "r");
  if (file == NULL) {
    PrintError("Could not open TIFF file %s", filename->name);
    return 0;
  }
  returnValue = TiffGetImageParameters(file, tiffData);
  TIFFClose(file);

  return returnValue;
}



void TiffSetImageParameters(TIFF *tiffFile, pt_tiff_parms *tiffData)
{
  assert(tiffFile != NULL);

  assert(PHOTOMETRIC_RGB == 2);
  assert(TIFFTAG_PHOTOMETRIC == 0x106);
  assert(TIFFTAG_PLANARCONFIG == 0x11c);
  assert(PLANARCONFIG_CONTIG == 1);
  assert(0x8005 == COMPRESSION_PACKBITS);
  assert(TIFFTAG_ORIENTATION == 0x112);
  assert(TIFFTAG_ROWSPERSTRIP == 0x116);
  
  
  TIFFSetField(tiffFile, TIFFTAG_IMAGEWIDTH, tiffData->imageWidth);
  TIFFSetField(tiffFile, TIFFTAG_IMAGELENGTH, tiffData->imageLength);
  TIFFSetField(tiffFile, TIFFTAG_BITSPERSAMPLE, tiffData->bitsPerSample);
  TIFFSetField(tiffFile, TIFFTAG_PHOTOMETRIC, PHOTOMETRIC_RGB);
  TIFFSetField(tiffFile,  TIFFTAG_PLANARCONFIG, PLANARCONFIG_CONTIG);
  TIFFSetField(tiffFile, TIFFTAG_SAMPLESPERPIXEL, tiffData->samplesPerPixel);
  TIFFSetField(tiffFile, TIFFTAG_COMPRESSION, COMPRESSION_PACKBITS);
  TIFFSetField(tiffFile, TIFFTAG_ORIENTATION, ORIENTATION_TOPLEFT);
  TIFFSetField(tiffFile,  TIFFTAG_ROWSPERSTRIP, tiffData->rowsPerStrip);

}



int VerifyTiffsAreCompatible(fullPath *tiffFiles, int numberImages)
{
  int currentImage;
  pt_tiff_parms firstFileParms;
  pt_tiff_parms otherFileParms;
  char *errorMsg;

  // Open TIFFs

  if (!TiffGetImageParametersFromPathName(&tiffFiles[0], &firstFileParms)) {
    PrintError("Unable to read tiff file");
    return 0;
  }

  for (currentImage = 1;  currentImage < numberImages ; currentImage ++ ) {
    
    if (!TiffGetImageParametersFromPathName(&tiffFiles[currentImage], &otherFileParms)) {
      PrintError("Unable to read tiff file");
      return 0;
    }
    
    errorMsg = NULL;
    if (firstFileParms.imageWidth != otherFileParms.imageWidth) {
      errorMsg = "Image 0 and %d do have the same width\n";
    } else if (firstFileParms.imageLength != otherFileParms.imageLength) {
      errorMsg = "Image 0 and %d do have the same length\n";
    } else if (firstFileParms.bitsPerPixel != otherFileParms.bitsPerPixel) {
      errorMsg = "Image 0 and %d do have the same colour depth\n";
    } else if (firstFileParms.bytesPerLine  != otherFileParms.bytesPerLine) {
      errorMsg = "Image 0 and %d do have the same scan line size\n";
    } else {
      ;
    }
    if (errorMsg != NULL) {
      PrintError(errorMsg, currentImage);
      return 0;
    }
  } // for loop

  return 1;

}


int  CreatePSD(  fullPath *fullPathImages, int numberImages, fullPath* outputFileName)
{

  Image *ptrImage;
  char *var24;
  int i;
  stBuf stitchInfo; // length 524
  fullPath tempFile;
  char tempString[128];
  Image image;
  

  if ( numberImages == 0 ) {
    return 0;
  }
  
  if ( quietFlag == 0 ) {
    
    Progress(_initProgress, "Converting TIFF to PSD");
  }

  sprintf(tempString, "%d", 0x64/numberImages);

  if ( quietFlag == 0 ) {

    Progress(_setProgress, tempString);

  }
  
  SetImageDefaults(&image);

  if (readTIFF(&image, &fullPathImages[0]) != 0 ) {

    PrintError("Could not read TIFF image No 0");

    if ( quietFlag == 0 ) {
      Progress(_disposeProgress, tempString);
    }
    return -1;

  }

  TwoToOneByte(&image);

  if (writePSDwithLayer(&image, outputFileName) != 0) {

    PrintError("Could not write PSD-file");

    if ( quietFlag != 0 ) 
      Progress(_disposeProgress, tempString);
    return -1;
  }

  myfree((void**)image.data);

 
  i = 1;

  var24 = tempString;
  
  ptrImage = &image;

  for (i = 1; i < numberImages; i++) {
    
    sprintf(var24, "%d", i * 100/numberImages);
    
    
    if ( quietFlag == 0 ) {
      
      if ( Progress(_setProgress,var24) == 0 ) {
	remove(outputFileName->name);
	return -1;
      }
      
    }

    if (readTIFF(ptrImage, &fullPathImages[i]) != 0) {
      
      PrintError("Could not read TIFF image No &d", i);
      
      
      if ( quietFlag == 0 ) {
	
	Progress(_disposeProgress, var24);
	
      }
      return -1;
      
    }
    
    TwoToOneByte(ptrImage);
    
    stitchInfo.seam = 1;
    stitchInfo.feather = 0;
    
    strcpy(tempFile.name, outputFileName->name);
    
    
    if (makeTempPath(&tempFile) != 0) {
      PrintError("Could not make Tempfile");
      return -1;
      
    }
    
    if (addLayerToFile(ptrImage, outputFileName, &tempFile, &stitchInfo) != 0) {
      //       remove(outputFileName->name);
      PrintError("Could not write Panorama File");
      return -1;
    }
    
    remove(outputFileName->name);
    
    rename(tempFile.name, outputFileName->name);
    
    myfree((void**)image.data);
  }   
  if (! quietFlag) {
    Progress(_setProgress, "100");
    Progress(_disposeProgress, tempString);
  }

  return 0;
}


static void ComputeStitchingMask8bits(Image *image)
{

  int column;
  int row;
  unsigned char *ptr;
  unsigned char *pixel;
  uint16  *ptrCounter;
  uint16 count;


  //  fprintf(stderr, "St1\n");

  for (column = 0; column < image->width; column ++) {

    count = 0;

    // Point to the given column in row 0

    ptr = *image->data + column * 4;

    //    fprintf(stderr, "St1.1 Column[%d]\n", column);



    for (row = 0; row < image->height; row ++) {

      //      fprintf(stderr, "St1.2 Column[%d] Row[%d]\n", column, row);

      pixel = row * image->bytesPerLine + ptr;

      if (*pixel == 0) {
	
	count = 0;
	
      } else {

	count ++;
	
      }
      // Use the GB pixel area to keep a count of how many pixels we have seen in 
      // the mask area.

      ptrCounter = (uint16*)(pixel +2);
      *ptrCounter = count;

    } //     for (row = 0; row < image->heght; row ++) {


    //    fprintf(stderr, "St1.3 Column[%d]\n", column);

    count = 0;
    row = image->height;

    while (--row  >= 0) {

      //      fprintf(stderr, "St1.4 Column[%d] Row[%d]\n", column, row);

      pixel = ptr + row + image->bytesPerLine;

      if ( *pixel == 0 ) {

	count = 0;

      } else {

	count ++;

      }

      ptrCounter = (uint16*)(pixel +2);

      if ( *ptrCounter < count) {
      
	count = *ptrCounter; 
      
      } else {
      
	*ptrCounter  = count;
      
      }
    
    
    } //while
    
    //    fprintf(stderr, "St1.5 Column[%d]\n", column);

    
  } //

  ///////////// row by row

  //  fprintf(stderr, "St2\n");
  return; ///AAAAAAAAAAAAAAAA


  for (row = 0; row < image->height; row ++) {

    count = image->width;
    
    ptr = row * image->bytesPerLine + *(image->data);
    
    
    for (column = 0; column < image->width; column ++ ) {
      
      
      pixel = ptr + 4 * column;
      
      if ( *pixel == 0 ) {
	
	count = 0;
	
      } else {
	
	count ++;
	
      }
      
      ptrCounter = (uint16*)(pixel +2);
      
      if (*ptrCounter < count) {
	
	//	count = *ptrCounter; AAAAAAAAAAAAA
	
      } else {
	
	*ptrCounter = count;
	
      } //

    } // for column

    //-----------------------------;;


    for (column = 0; column < image->width; column ++ ) {


      pixel = ptr + column * 4;

      if ( *pixel == 0 ) {
	count = 0;
      } else {
	count ++;
      }

      ptrCounter =  (uint16*)(pixel +2);

      if (*ptrCounter < count) {
	//	count = *ptrCounter; AAAAAAAAAAAAAAA
      } else {
	*ptrCounter = count;
      }
    } // for

    //---------------------------------------;

    //  fprintf(stderr, "St3\n");

    count = image->width;
    column = image->width;

    while (--column >= 0) {

      pixel = ptr + column * 4;

      if (0 == *pixel ) {
	count = 0;
      } else {
	count ++;
      }

      ptrCounter =  (uint16*)(pixel +2);

      if (*ptrCounter < count) {
	count = *ptrCounter;
      } else {
	*ptrCounter = count;
      }
    } //    while (--column >= 0) {

    //--------------------------------;
    column = image->width;

    //  fprintf(stderr, "St4\n");

    while (--column >= 0) {

      pixel = ptr + 4 * column;

      if ( *pixel == 0 ) {
	count = 0;
      } else {
	count ++;
      }

      ptrCounter =  (uint16*)(pixel +2);

      if (*ptrCounter < count) {
	
	count = *ptrCounter ;
      
      } else {
	*ptrCounter = count;
      }
    } // end of while
    
  } // end of for row


}

static void ComputeStitchingMask16bits(Image *image)
{
  fprintf(stderr, "Masking not supported for this image type (%d bitsPerPixel)\n", (int)image->bitsPerPixel);
  exit(1);
}


static void ComputeStitchingMaskMap(Image *image)
{
  if ( image->bitsPerPixel == 32 ) {
    ComputeStitchingMask8bits(image);
    return;
  } else  if ( image->bitsPerPixel == 64 ) {
    ComputeStitchingMask16bits(image);
    return;
  }
  fprintf(stderr, "Masking not supported for this image type (%d bitsPerPixel)\n", (int)image->bitsPerPixel);
  exit(1);
}




//
// Compute the map of the stitching mask and create a file with it.
// The stitching mask will be contained in the GB channels (this is,
// the 16 bits corresponding to the G and B channel will contain a uint16 that
// contains, for that particular point, the stitching mask.
//
int CreateMaskMapFiles(fullPath *inputFiles, fullPath *maskFiles, int numberImages)
{
  int index;
  char tempString[512];
  Image image; 

  if ( quietFlag == 0 ) 
    Progress(_initProgress, "Preparing Stitching Masks");

  // for each image, create merging mask and save to temporal file
  for (index = 0; index < numberImages; index ++ ) {
    
    sprintf(tempString, "%d", index * 100/numberImages);
    
    // Do progress
    if ( quietFlag == 0 ) {
      if (Progress(_setProgress, tempString) ==0 ) {
	return 0;
      }
    }
    
    if (readTIFF(&image, &inputFiles[index]) != 0) {
      PrintError("Could not read TIFF-file");
      return 0;
    }

    // Compute the stitching mask in-situ
    ComputeStitchingMaskMap(&image);

    strcpy(maskFiles[index].name, inputFiles[0].name); 

    if (makeTempPath(&maskFiles[index]) != 0) {
      PrintError("Could not make Tempfile");
      return -1;
    }

    writeTIFF(&image, &maskFiles[index]);
    
    //    fprintf(stderr, "Written to file %s\n", maskFiles[index].name);
    
    myfree((void**)image.data);

  } // for (index...

  Progress(_disposeProgress, tempString);

  return 1;
}






int  ReplaceAlphaChannels(fullPath *inputImagesNames, fullPath *masksNames, fullPath *outputNames, int numberImages,
			 pt_tiff_parms *imageDefaultParms)
{
  int index;
  unsigned char *imageRowBuffer = NULL;
  unsigned char *maskRowBuffer = NULL;
  char tempString[MAX_PATH_LENGTH];
  int row;
  int j;

  TIFF *imageFile;
  TIFF *outputFile;
  TIFF *maskFile;

  if ((imageRowBuffer = calloc(imageDefaultParms->bytesPerLine,1 )) == NULL ||
      (maskRowBuffer = calloc(imageDefaultParms->bytesPerLine,1 )) == NULL ) {
    PrintError("Not enough memory");
    return -1;
  }

  // The algorith is fairly simple. 

  // For each image
  //   For each row in that image
  //     Read row of image
  //     Read row of mask
  //     Replace alpha channel in image with masks alpha channel
  //     Write row

  if (!quietFlag) {
    Progress(_initProgress, "Inserting Alpha Channel");
  }

  for (index = 0; index < numberImages; index ++) { //

    // Do one file at a time

    if ( quietFlag == 0 ) {
      sprintf(tempString, "%d", index * 100/ numberImages);
      if (Progress(_setProgress, tempString) == 0) {
	return 0;
      }
    }
    
    // Open input image	
    if ( (imageFile = OpenTiffFromFullPath(&inputImagesNames[index], "r")) == NULL) {
      PrintError("Could not open TIFF-file");
      return 0;
    }

    // Open mask file
    if ( (maskFile = OpenTiffFromFullPath(&masksNames[index], "r")) == NULL) {
      PrintError("Could not open mask file");
      return 0;
    }

    // Create output file
    if ((outputFile = OpenTiffFromFullPath(&outputNames[index], "w")) == NULL) {
      PrintError("Could not create TIFF-file");
      return 0;
    }
    TiffSetImageParameters(outputFile,imageDefaultParms);

    // start processing each row

    for (row = 0; row < imageDefaultParms->imageLength; row ++) {

      unsigned char *source;
      unsigned char *destination;

      TIFFReadScanline(imageFile, imageRowBuffer, row, 0);

      TIFFReadScanline(maskFile, maskRowBuffer, row, 0);

      // Depending on the type of image, process...

      if ( imageDefaultParms->bitsPerPixel == 32 ) {

	destination = imageRowBuffer + 3;
	source = maskRowBuffer + 3;

	for (j = 0; j < imageDefaultParms->imageWidth; j ++ ) {
	  *destination = *source;
	  destination +=4;
	  source +=4;
	} 

      } else {

	destination = imageRowBuffer + 6;
	source = maskRowBuffer + 6;

	for (j = 0; j < imageDefaultParms->imageWidth ; j ++ ) {
	  *destination = *source;

	  source += 8;
	  destination += 8;

	} // for j
       
      } // end of if

      TIFFWriteScanline(outputFile, imageRowBuffer, row, 0);
     
    } // for 

    TIFFClose(imageFile);
    TIFFClose(maskFile);
    TIFFClose(outputFile);

  } // for index
  if (!quietFlag) 
    Progress(_disposeProgress, tempString);


  free(imageRowBuffer);
  free(maskRowBuffer);

  return 1;

}

int  ReplaceAlphaChannel(fullPath *inputImage, fullPath *mask, fullPath *output,
			 pt_tiff_parms *imageDefaultParms)
{
  unsigned char *imageRowBuffer = NULL;
  unsigned char *maskRowBuffer = NULL;
  int row;
  int j;

  TIFF *imageFile;
  TIFF *outputFile;
  TIFF *maskFile;

  if ((imageRowBuffer = calloc(imageDefaultParms->bytesPerLine,1 )) == NULL ||
      (maskRowBuffer = calloc(imageDefaultParms->bytesPerLine,1 )) == NULL ) {
    PrintError("Not enough memory");
    return 0;
  }

  //   For each row in that image
  //     Read row of image
  //     Read row of mask
  //     Replace alpha channel in image with masks alpha channel
  //     Write row

  // Open input image	
  if ( (imageFile = OpenTiffFromFullPath(inputImage, "r")) == NULL) {
    PrintError("Could not open TIFF-file");
    return 0;
  }
  
  // Open mask file
  if ( (maskFile = OpenTiffFromFullPath(mask, "r")) == NULL) {
    PrintError("Could not open mask file");
    return 0;
  }
  
  // Create output file
  if ((outputFile = OpenTiffFromFullPath(output, "w")) == NULL) {
    PrintError("Could not create TIFF-file");
    return 0;
  }
  TiffSetImageParameters(outputFile,imageDefaultParms);
  
  // start processing each row
  
  for (row = 0; row < imageDefaultParms->imageLength; row ++) {
    
    unsigned char *source;
    unsigned char *destination;
    
    TIFFReadScanline(imageFile, imageRowBuffer, row, 0);
    
    TIFFReadScanline(maskFile, maskRowBuffer, row, 0);
    
    // Depending on the type of image, process...
    
    if ( imageDefaultParms->bitsPerPixel == 32 ) {
      
      destination = imageRowBuffer + 3;
      source = maskRowBuffer + 3;
      
      for (j = 0; j < imageDefaultParms->imageWidth; j ++ ) {
	*destination = *source;
	destination +=4;
	source +=4;
      } 
      
    } else {
      
      destination = imageRowBuffer + 6;
      source = maskRowBuffer + 6;
      
      for (j = 0; j < imageDefaultParms->imageWidth ; j ++ ) {
	*destination = *source;
	
	source += 8;
	destination += 8;
	
      } // for j
      
    } // end of if
    
    TIFFWriteScanline(outputFile, imageRowBuffer, row, 0);
    
  } // for 
  
  TIFFClose(imageFile);
  TIFFClose(maskFile);
  TIFFClose(outputFile);
  
  free(imageRowBuffer);
  free(maskRowBuffer);

  return 1;

}


static void SetBestAlphaChannel16bits(unsigned char *imagesBuffer, int numberImages, pt_tiff_parms *imageParms)
{
  assert(0); // it should not be here... yet
}
static void SetBestAlphaChannel8bits(unsigned char *imagesBuffer, int numberImages, pt_tiff_parms *imageParms)
{
  unsigned char *pixel;
  uint16 *ptrCount;
  uint16 best;
  uint16 maskValue;
  int column;
  int j;


  for  (column=0, pixel = imagesBuffer;  column < imageParms->imageWidth; column++, pixel +=4) {

    best = 0;
    ptrCount = (uint16*)(pixel + 2);
    maskValue = *ptrCount;

    // find the image with the highest value

    for (j = 1; j < numberImages; j ++) {

      ptrCount = (uint16*)(pixel + imageParms->bytesPerLine * j  + 2);

      if (*ptrCount > maskValue) {

	best = j;
	maskValue = *ptrCount;
      
      }
    } // for j

    if ( maskValue != 0 ) {

      // set the mask of the ones above, but not below... interesting...

      for (j = best+1;  j < numberImages; j ++ ) {
	unsigned char *pixel2;
  
	pixel2 = pixel +  imageParms->bytesPerLine * j;

	if (0 != *pixel2) {
	  *pixel2 = 1;
	} 
      }
    }
  } // for i

}


static void CalculateAlphaChannel(unsigned char *imagesBuffer, int numberImages, pt_tiff_parms *imageParms)
{

  if (imageParms->bitsPerPixel == 32) {
    SetBestAlphaChannel8bits(imagesBuffer, numberImages, imageParms);
  } else if (imageParms->bitsPerPixel == 64) {
    SetBestAlphaChannel16bits(imagesBuffer, numberImages, imageParms);
  } else {
    fprintf(stderr, "CalculateAlphaChannel not supported for this image type (%d bitsPerPixel)\n", imageParms->bitsPerPixel);
    exit(1);
  }
}



static int CreateAlphaChannels(fullPath *masksNames, fullPath *alphaChannelNames,
			       int numberImages, pt_tiff_parms *imageParameters)
{
  TIFF **tiffMasks;
  TIFF **tiffAlphaChannels;
  unsigned char *imagesBuffer;
  unsigned char *ptrBuffer;
  int index;
  int row;
  char tempString[24];

  
  // Allocate arrays of TIFF* for the input and output
  // images. process is one row at a time, with all images
  // processed at the same time
  tiffMasks = (TIFF**)calloc(numberImages, sizeof(TIFF*));
  tiffAlphaChannels = (TIFF**)calloc(numberImages, sizeof(TIFF*));
  imagesBuffer = calloc(numberImages, imageParameters->bytesPerLine);
  
  if (imagesBuffer == NULL ||
      tiffAlphaChannels == NULL ||
      imagesBuffer == NULL) {
    PrintError("Not enough memory");
    return 0;
  }

  /// Alpha Channel calculation

  //  fprintf(stderr, "Start alpha channel calculation\n");
 
  if ( quietFlag == 0 ) {
    Progress(_initProgress, "Calculating Alpha Channel");
  }
    
  // Open for read
  //       mask files
  //  and  input files
  // Open for write  alpha channel files

  for (index = 0; index < numberImages; index++) {

    if ((tiffMasks[index] = OpenTiffFromFullPath(&masksNames[index], "r")) == 0) {
      PrintError("Could not open TIFF-file");
      return 0;
    }

    strcpy(alphaChannelNames[index].name, masksNames[0].name);

    if (makeTempPath(&alphaChannelNames[index]) != 0) {
      PrintError("Could not make Tempfile");
      return 0;
    }

    tiffAlphaChannels[index] = OpenTiffFromFullPath(&alphaChannelNames[index], "w");

    if ( tiffAlphaChannels[index] == NULL ) {
      PrintError("Could not create TIFF-file");
      return 0;
    }

    TiffSetImageParameters(tiffAlphaChannels[index], imageParameters);

  }// for index...

  //  fprintf(stderr, "Files have been created, process each row\n");
 
  for (row= 0; row< imageParameters->imageLength; row++) {


    if ( quietFlag == 0 ) {
      
      if ( row== (row/ 20) * 20 ) {
	
	sprintf(tempString, "%lu", row* 100/imageParameters->imageLength);
	
	
	if (Progress(_setProgress, tempString) == 0) {
	  
	  for (index = 0 ; index < numberImages; index ++) {
	    TIFFClose(tiffMasks[index]);
	    TIFFClose(tiffAlphaChannels[index]);
	    remove(alphaChannelNames[index].name);
	  }
	  return 0;
	}
      } 
    }
    
    //    fprintf(stderr, "To process row [%d] bytesperline %d\n", i, bytesPerLine);

    for (ptrBuffer = imagesBuffer, index = 0; index < numberImages; index ++, ptrBuffer += imageParameters->bytesPerLine) {

      TIFFReadScanline(tiffMasks[index], ptrBuffer, row, 0);

      RGBAtoARGB(ptrBuffer, imageParameters->imageWidth, imageParameters->bitsPerPixel);
     
    }

    CalculateAlphaChannel(imagesBuffer, numberImages, imageParameters);

    for (index = 0 , ptrBuffer = imagesBuffer; index < numberImages; index ++, ptrBuffer+= imageParameters->bytesPerLine) {

      ARGBtoRGBA(ptrBuffer, imageParameters->imageWidth, imageParameters->bitsPerPixel);
      TIFFWriteScanline(tiffAlphaChannels[index], ptrBuffer, row, 0);
     
    } //for
   
  } //for i


  for (index = 0; index < numberImages; index ++) {
    TIFFClose(tiffMasks[index]);
    TIFFClose(tiffAlphaChannels[index]);
  } // for index.

  free(imagesBuffer);
  free(tiffAlphaChannels);
  free(tiffMasks);

  return 1;
}

static void ApplyFeather8bits(Image *image, int featherSize)
{

  fprintf(stderr, "\nFeathering 8 bits not implemented yet\n");
  
}

static void ApplyFeather16bits(Image *image, int featherSize)
{

  fprintf(stderr, "\nFeathering 16 bits not implemented yet\n");
  
}


static int ApplyFeather(fullPath *inputFile, fullPath *outputFile, int featherSize)
{
  Image image;
  if (readTIFF(&image, inputFile) != 0) {
    PrintError("Could not open TIFF-file [%s]", inputFile->name);
    return 0;
  }

  //  fprintf(stderr, "To apply feather %d\n", featherSize);
  if (image.bitsPerPixel == 32) {
    ApplyFeather8bits(&image, featherSize);
  } else if (image.bitsPerPixel == 64) {
    ApplyFeather16bits(&image, featherSize);
  } else {
    fprintf(stderr, "Apply feather not supported for this image type (%d bitsPerPixel)\n", (int)image.bitsPerPixel);
    exit(1);
  }

  if (writeTIFF(&image, outputFile) != 0) {
    PrintError("Could not write TIFF-file [%s]", outputFile->name);
    return 0;
  }

  myfree((void**)image.data);

  return 1;

}


int AddStitchingMasks(fullPath *inputFiles, fullPath *outputFiles, int numberImages, int featherSize)
{
  fullPath *alphaChannelFiles;
  fullPath *maskFiles;
  int i;
  Image image ;
  char tempString[512];
  pt_tiff_parms imageParameters;

  if ( numberImages == 0 ) {
    return 0;
  }

  SetImageDefaults(&image);

  maskFiles = calloc(numberImages, sizeof(fullPath));
  alphaChannelFiles = calloc(numberImages, sizeof(fullPath));

  if ( maskFiles == NULL ||
       alphaChannelFiles == NULL ) {
    PrintError("Not enough memory");
    return -1;
  } 

  // CREATE stitching maps

  if (!CreateMaskMapFiles(inputFiles, maskFiles, numberImages)) {
    PrintError("Could not create the stitching masks");
    return -1;
  }
  //  exit(1);

  // Get TIFF information
  
  if (!TiffGetImageParametersFromPathName(&inputFiles[0], &imageParameters)) {
    PrintError("Could not read TIFF-file");
    return -1;
  }
  
  
  if (!CreateAlphaChannels(maskFiles, alphaChannelFiles, numberImages, &imageParameters)) {
    PrintError("Could not create alpha channels");
    return -1;
  }

  // From this point on we do not need to process all files at once. This will save temporary disk space
  
  if ( quietFlag == 0 ) {
    Progress(_setProgress, "Creating alpha channel and feather tool");
  }

  for (i=0;i<numberImages;i++) {
    fullPath withAlphaChannel;


    sprintf(tempString, "%d", 100 * i/ numberImages);

    if ( quietFlag == 0 ) {
      if (Progress(_setProgress, tempString) == 0) {
	// We have to delete any temp file
	return -1;
      }
    }

    // We no longer need the mask files
    remove(maskFiles[i].name);

    // Reuse the temporary name
    memcpy(&withAlphaChannel, &maskFiles[i], sizeof(fullPath));

    // Replace the alpha channel of the input image
    if (!ReplaceAlphaChannel(&inputFiles[i], &alphaChannelFiles[i], &withAlphaChannel, &imageParameters)) {
      PrintError("Unable to replace alpha channel in image %d", i);
      return -1;
    }
    // we no longer need the alpha channel
    remove(alphaChannelFiles[i].name);

    if (featherSize > 0) {
      fullPath feathered;
      memcpy(&feathered, &maskFiles[i], sizeof(fullPath));
      if (!ApplyFeather(&withAlphaChannel, &feathered, featherSize)) {
	PrintError("Unable to apply feather to image %d", i);
	return -1;
      }

      remove(withAlphaChannel.name);
      rename(feathered.name, outputFiles[i].name);

    } else {
      rename(withAlphaChannel.name, outputFiles[i].name);
    }
  }

  free(maskFiles);
  free(alphaChannelFiles);

  return 0;

}

#ifdef __Win__
void InsertFileName( fullPath *fp, char *fname ){
	char *c = strrchr((char*)(fp->name), PATH_SEP);
	if(c != NULL) c++;
	else c = fp->name;
	strcpy( c, fname );
}	
#endif

