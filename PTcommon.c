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

#include "filter.h"
#include "panorama.h"

#include "tiffio.h"
#include "pttiff.h"

#include "PTcommon.h"


//declare functions
void getCropInformationFromTiff(TIFF *tif, CropInfo *c);
void getROI( TrformStr *TrPtr, aPrefs *aP, PTRect *ROIRect );
void setFullSizeImageParameters(pt_tiff_parms *imageParameters, CropInfo *crop_info);

int ptQuietFlag = 0;

void tiffErrorHandler(const char* module, const char* fmt, va_list ap)
{
  PrintError("Error in TIFF file (%s) ", module);
  PrintError(fmt, ap);
}


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

  if (!TIFFGetField(tiffFile, TIFFTAG_COMPRESSION, &tiffData->compression)) {
    PrintError("File did not include compression information.");
    return 0;
  }
  if (tiffData->compression == COMPRESSION_LZW) {
    //predictor only exists in LZW compressed files
    if (!TIFFGetField(tiffFile, TIFFTAG_PREDICTOR, &tiffData->predictor)) {
      PrintError("File did not include predictor information.");
      return 0;
    }
  }


  tiffData->bytesPerLine = TIFFScanlineSize(tiffFile);
  if (tiffData->bytesPerLine <= 0) {
    PrintError("File did not include proper bytes per line information.");
    return 0;
  }
  tiffData->bitsPerPixel = tiffData->samplesPerPixel * tiffData->bitsPerSample;

  //tiffData->rowsPerStrip = tiffData->imageLength;
  if (!TIFFGetField(tiffFile, TIFFTAG_ROWSPERSTRIP, &tiffData->rowsPerStrip)) {
    PrintError("File did not include rows per strip information.");
    return 0;
  }  
    
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



int TiffSetImageParameters(TIFF *tiffFile, pt_tiff_parms *tiffData)
{
  int returnValue = 1;

  assert(tiffFile != NULL);
  assert(tiffData != NULL);

  // Each of the invocations below returns 1 if ok, 0 if error. 

  returnValue = TIFFSetField(tiffFile, TIFFTAG_IMAGEWIDTH, tiffData->imageWidth)
    && TIFFSetField(tiffFile, TIFFTAG_IMAGELENGTH, tiffData->imageLength)
    && TIFFSetField(tiffFile, TIFFTAG_BITSPERSAMPLE, tiffData->bitsPerSample)
    && TIFFSetField(tiffFile, TIFFTAG_PHOTOMETRIC, PHOTOMETRIC_RGB)
    && TIFFSetField(tiffFile,  TIFFTAG_PLANARCONFIG, PLANARCONFIG_CONTIG)
    && TIFFSetField(tiffFile, TIFFTAG_SAMPLESPERPIXEL, tiffData->samplesPerPixel)
    && TIFFSetField(tiffFile, TIFFTAG_COMPRESSION, tiffData->compression)
    && TIFFSetField(tiffFile, TIFFTAG_ORIENTATION, ORIENTATION_TOPLEFT)
    && TIFFSetField(tiffFile,  TIFFTAG_ROWSPERSTRIP, tiffData->rowsPerStrip);

  if (returnValue && tiffData->compression == COMPRESSION_LZW) {
    returnValue = TIFFSetField(tiffFile, TIFFTAG_PREDICTOR, tiffData->predictor);
  }

  return returnValue;
  
}



int VerifyTiffsAreCompatible(fullPath *tiffFiles, int numberImages)
{
  int currentImage;
  pt_tiff_parms firstFileParms;
  pt_tiff_parms otherFileParms;
  char *errorMsg;

  //MRDL: Reluctantly commented these out...the calls to TIFFSetWarningHandler and 
  //TIFFSetErrorHandler cause to GCC to abort, with a series of errors like this:
  //../../../LibTiff/tiff-v3.6.1/libtiff/libtiff.a(tif_unix.o)(.text+0x11a): In function `TIFFOpen':
  //../../../libtiff/tiff-v3.6.1/libtiff/../libtiff/tif_unix.c:144: multiple definition of `TIFFOpen'
  //../libpano12.a(dyces00121.o)(.text+0x0): first defined here
  // Make sure we have a tiff error handler
  //TIFFSetWarningHandler(tiffErrorHandler);
  //TIFFSetErrorHandler(tiffErrorHandler);
  
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
  int i;
  stBuf stitchInfo;
  fullPath tempFile;
  char tempString[128];
  Image image;

  assert(numberImages > 0);
  assert(fullPathImages != NULL);
  assert(outputFileName != NULL);
  
  if ( ptQuietFlag == 0 ) {
    Progress(_initProgress, "Converting TIFF to PSD");
    sprintf(tempString, "%d", 100/numberImages);
    Progress(_setProgress, tempString);
  }

  // Process background of PSD
  SetImageDefaults(&image);

  if (readTIFF(&image, &fullPathImages[0]) != 0 ) {
  
    PrintError("Could not read TIFF image No 0");
	 if ( ptQuietFlag == 0 )
      Progress(_disposeProgress, tempString);

    return -1;
  }


  if (!( image.bitsPerPixel == 64 ||  image.bitsPerPixel == 32)  ) {
    PrintError("Image type not supported (%d bits per pixel)\n", image.bitsPerPixel);
    return 0;
  }

  if (numberImages > 1 && image.bitsPerPixel != 32) {
    if ( image.bitsPerPixel == 64 ) {
      PrintError("Panotools is not able to save 16bit PSD images. Downsampling to 8 bit");
      TwoToOneByte(&image); //we need to downsample to 8 bit if we are provided 16 bit images
    }
  }

  //Write out the first image as the base layer in the PSD file
  if (writePSDwithLayer(&image, outputFileName) != 0) {
    PrintError("Could not write PSD-file");
    if ( ptQuietFlag != 0 ) 
      Progress(_disposeProgress, tempString);
    return -1;
  }

  myfree((void**)image.data);
  ptrImage = &image;
  
  //Now iterate over all other images and add them as layers to the PSD file
  for (i = 1; i < numberImages; i++) {
    
    if ( ptQuietFlag == 0 ) {
		sprintf(tempString, "%d", i * 100/numberImages);    
      if ( Progress(_setProgress,tempString) == 0 ) {
		  remove(outputFileName->name);
		  return -1;
      }
    }

    if (readTIFF(ptrImage, &fullPathImages[i]) != 0) {

		PrintError("Could not read TIFF image No &d", i);
      if ( ptQuietFlag == 0 )
		  Progress(_disposeProgress, tempString);
      return -1;
    }
    
    // We can't process 16 bit TIFFs. We have to downsample to 8 bit if necessary
    if ( image.bitsPerPixel == 64 ) 
      TwoToOneByte(ptrImage);
    
    // Create a new file with the result PSD, then delete the current one
    
    strcpy(tempFile.name, outputFileName->name);
    
    if (makeTempPath(&tempFile) != 0) {
      PrintError("Could not make Tempfile");
      return -1;
      
    }

    stitchInfo.seam = 1;
    stitchInfo.feather = 0;
        
    if (addLayerToFile(ptrImage, outputFileName, &tempFile, &stitchInfo) != 0) {
      PrintError("Could not write Panorama File");
      return -1;
    }
    
    remove(outputFileName->name);
    rename(tempFile.name, outputFileName->name);
    
    myfree((void**)image.data);
  }
  
  if (! ptQuietFlag) {
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
  uint16_t  *ptrCounter;
  uint16_t count;

  // Use the GreenBlue pixel area is used to keep a counter of the
  // minimum distance (in pixels) away we are from the edges of the
  // mask (horizontal or vertical)

  // The algorithm is fairly simple:

  // For each column
  //   Process each row from top to down
  //     Set each pixel counter to the number of pixels from edge of the mask (from the left)
  //   Process each row from bottom to top
  // Set each pixel counter to the minimum between current counter and number of pixels
  // from edge (from the right)

  // for each row
  //   repeat the same algorithm (done per column)
  
  for (column = 0; column < image->width; column ++) {
    count = 0;
    // Point to the given column in row 0
    ptr = *image->data + column * 4;
    //    fprintf(stderr, "St1.1 Column[%d]\n", column);

    for (row = 0; row < image->height; row ++) {
      pixel = row * image->bytesPerLine + ptr;
      count = (*pixel == 0) ? 0 : count+1;

      ptrCounter = (uint16_t*)(pixel +2);
      *ptrCounter = count;
    }


    count = 0;
    row = image->height;

    while (--row  >= 0) {
      pixel = ptr + row + image->bytesPerLine;

      count = (*pixel == 0) ? 0 : count+1;

      ptrCounter = (uint16_t*)(pixel +2);
      if ( *ptrCounter < count)
        count = *ptrCounter; 
      else
        *ptrCounter  = count;
    
    } //while
    
    //    fprintf(stderr, "St1.5 Column[%d]\n", column);

    
  } //

  ///////////// row by row

  //  fprintf(stderr, "St2\n");

  for (row = 0; row < image->height; row ++) {
    count = image->width;
    ptr = row * image->bytesPerLine + *(image->data);

    // process from left to right
    for (column = 0; column < image->width; column ++ ) {
      pixel = ptr + 4 * column;
      count = (*pixel == 0) ? 0 : count+1;

      ptrCounter = (uint16_t*)(pixel +2);
      
      if (*ptrCounter > count)
		  *ptrCounter = count;
    } // for column

    //-----------------------------;;


    //  fprintf(stderr, "St3\n");

	 count = 0;
	 column = image->width;

    while (--column >= 0) {
      pixel = ptr + column * 4;

      count = (*pixel == 0) ? 0 : count+1;

      ptrCounter =  (uint16_t*)(pixel +2);
      if (*ptrCounter < count)
		  count = *ptrCounter;
      else
        *ptrCounter = count;

    } //    while (--column >= 0)


  } // end of for row


}

static void ComputeStitchingMask16bits(Image *image)
{

  int column;
  int row;
  unsigned char *ptr;
  uint16_t *pixel;
  uint16_t  *ptrCounter;
  uint16_t count;

  //  fprintf(stderr, "St1\n");

  for (column = 0; column < image->width; column ++) {

    count = 0;

    // Point to the given column in row 0

    ptr = *image->data + column * 8;

    //    fprintf(stderr, "St1.1 Column[%d]\n", column);

    for (row = 0; row < image->height; row ++) {

      //      fprintf(stderr, "St1.2 Column[%d] Row[%d]\n", column, row);

      pixel = (uint16_t*) (row * image->bytesPerLine + ptr);

      if (*pixel == 0) {
   
		  count = 0;
   
      } else {

		  count ++;
   
      }
      // Use the G pixel area to keep a count of how many pixels we have seen in 
      // the mask area.

      ptrCounter = pixel +2;
      *ptrCounter = count;

    } //     for (row = 0; row < image->heght; row ++) {


    //    fprintf(stderr, "St1.3 Column[%d]\n", column);

    count = 0;
    row = image->height;

    while (--row  >= 0) {

      //      fprintf(stderr, "St1.4 Column[%d] Row[%d]\n", column, row);

      pixel = (uint16_t*) (row * image->bytesPerLine + ptr);

      if ( *pixel == 0 ) {

		  count = 0;

      } else {

		  count ++;

      }

      ptrCounter = pixel +2;

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
  //  return; ///AAAAAAAAAAAAAAAA


  for (row = 0; row < image->height; row ++) {

    count = image->width;
    
    ptr = row * image->bytesPerLine + *(image->data);

    for (column = 0; column < image->width; column ++ ) {
      
      
      pixel = (uint16_t*)(ptr + 4 * 2 * column);
      
      if ( *pixel == 0 ) {
   
		  count = 0;
   
      } else {
   
		  count ++;
   
      }
      
      ptrCounter = pixel +2;
      
      if (*ptrCounter < count) {
   
		  // count = *ptrCounter; AAAAAAAAAAAAA
   
      } else {
   
		  *ptrCounter = count;
   
      } //

    } // for column

    //-----------------------------;;


    for (column = 0; column < image->width; column ++ ) {


      pixel = (uint16_t*)(ptr + 4 * 2 * column);

      if ( *pixel == 0 ) {
		  count = 0;
      } else {
		  count ++;
      }

      ptrCounter = pixel +2;

      if (*ptrCounter < count) {
		  // count = *ptrCounter; AAAAAAAAAAAAAAA
      } else {
		  *ptrCounter = count;
      }
    } // for

    //---------------------------------------;

    //  fprintf(stderr, "St3\n");

    count = image->width;
    column = image->width;

    while (--column >= 0) {

      pixel = (uint16_t*)(ptr + 4 * 2 * column);

      if (0 == *pixel ) {
		  count = 0;
      } else {
		  count ++;
      }

      ptrCounter =  pixel +2;

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

      pixel = (uint16_t*)(ptr + 4 * 2 * column);


      if ( *pixel == 0 ) {
		  count = 0;
      } else {
		  count ++;
      }

      ptrCounter =  pixel +2;

      if (*ptrCounter < count) {
   
		  count = *ptrCounter ;
      
      } else {
		  *ptrCounter = count;
      }
    } // end of while
    
  } // end of for row
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
// the 16 bits corresponding to the G and B channel will contain a uint16_t that
// contains, for that particular point, the stitching mask.
//
int CreateMaskMapFiles(fullPath *inputFiles, fullPath *maskFiles, int numberImages)
{
  int index;
  char tempString[512];
  Image image; 

  if ( ptQuietFlag == 0 ) 
    Progress(_initProgress, "Preparing Stitching Masks");

  // for each image, create merging mask and save to temporal file
  for (index = 0; index < numberImages; index ++ ) {
    
    sprintf(tempString, "%d", index * 100/numberImages);
    
    // Do progress
    if ( ptQuietFlag == 0 ) {
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

    
    // Do progress

  if (!ptQuietFlag)

    Progress(_setProgress, "100");
  Progress(_disposeProgress, tempString);

  return 1;
}






static int  ReplaceAlphaChannels(fullPath *inputImagesNames, fullPath *masksNames, fullPath *outputNames, int numberImages,
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

  if (!ptQuietFlag) {
    Progress(_initProgress, "Inserting Alpha Channel");
  }

  for (index = 0; index < numberImages; index ++) { //

    // Do one file at a time

    if ( ptQuietFlag == 0 ) {
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
    if (!TiffSetImageParameters(outputFile,imageDefaultParms)) {
      PrintError("Unable to initialize TIFF file\n");
      return 0;
    }

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

		if (TIFFWriteScanline(outputFile, imageRowBuffer, row, 0) != 1) {
		  PrintError("Unable to write scan line\n");
		  TIFFClose(imageFile);
		  TIFFClose(maskFile);
		  TIFFClose(outputFile);
		  return 0;
		}
    } // for 

    TIFFClose(imageFile);
    TIFFClose(maskFile);
    TIFFClose(outputFile);

  } // for index

  if (!ptQuietFlag) 
    Progress(_disposeProgress, tempString);


  free(imageRowBuffer);
  free(maskRowBuffer);

  return 1;

}

static int  ReplaceAlphaChannel(fullPath *inputImage, fullPath *mask, fullPath *output)
{
  unsigned char *imageRowBuffer = NULL;
  unsigned char *maskRowBuffer = NULL;
  int row;
  int j;

  int returnValue = 0;
  
  unsigned char *source;
  unsigned char *destination;

  TIFF *imageFile;
  TIFF *outputFile;
  TIFF *maskFile;
  
  CropInfo crop_info;
  pt_tiff_parms croppedImageParms;
  
  //   For each row
  //     Read row of image
  //     Read row of mask
  //     Replace alpha channel in image with masks alpha channel
  //     Write row
  //
  //Note that all three images involved here (input image, mask image and 
  //resulting image) have the same dimensions, and we don't care what the 
  //dimensions are the for the final output image  

  // Open input image   
  if ( (imageFile = OpenTiffFromFullPath(inputImage, "r")) == NULL) {
    PrintError("Could not open TIFF-file");
    return 0;
  } else {
    TiffGetImageParameters(imageFile, &croppedImageParms);
    getCropInformationFromTiff(imageFile, &crop_info);
  }
  
  //Allocate line buffers for image and mask
  if ((imageRowBuffer = calloc(croppedImageParms.bytesPerLine,1 )) == NULL ||
      (maskRowBuffer = calloc(croppedImageParms.bytesPerLine,1 )) == NULL ) {
    PrintError("Not enough memory");
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
  
  if (!TiffSetImageParameters(outputFile, &croppedImageParms)) {
    PrintError("Unable to initialize TIFF file\n");
    return 0;
  }
  
  // Processing one row at a time
  int jumpBytes          = (croppedImageParms.bitsPerPixel == 32 ) ? 4 : 8;
  int alphaChannelOffset = (croppedImageParms.bitsPerPixel == 32 ) ? 3 : 6;
  
  for (row = 0; row < croppedImageParms.imageLength; row ++) {
    
    TIFFReadScanline(imageFile, imageRowBuffer, row, 0);
    TIFFReadScanline(maskFile, maskRowBuffer, row, 0);
    
    destination = imageRowBuffer + alphaChannelOffset;
    source      = maskRowBuffer  + alphaChannelOffset;
      
    for (j = 0; j < croppedImageParms.imageWidth; j++ ) {
      *destination = *source;
      destination += jumpBytes;
      source      += jumpBytes;
    } 
    
    if (TIFFWriteScanline(outputFile, imageRowBuffer, row, 0)!= 1) {
      PrintError("Unable to write data to output file\n");
      returnValue = 0;
      goto end;
    }
    
  }
  
  returnValue = 1;
 end:

  TIFFClose(imageFile);
  TIFFClose(maskFile);
  TIFFClose(outputFile);
  
  free(imageRowBuffer);
  free(maskRowBuffer);

  return returnValue;

}


static void SetBestAlphaChannel16bits(unsigned char *imagesBuffer, int numberImages, pt_tiff_parms *imageParms)
{
  //  fprintf(stderr, "SetBestAlphaChannel16bits not supported yet\n");
  //assert(0); // it should not be here... yet

  unsigned char *pixel;
  uint16_t *ptrCount;
  uint16_t best;
  uint16_t maskValue;
  int column;
  int j;


  for  (column=0, pixel = imagesBuffer;  column < imageParms->imageWidth; column++, pixel +=8) {

    best = 0;
    ptrCount = (uint16_t*)(pixel + 2);
    maskValue = *ptrCount;

    // find the image with the highest value

    for (j = 1; j < numberImages; j ++) {

      ptrCount = (uint16_t*)(pixel + imageParms->bytesPerLine * j  + 2);

      if (*ptrCount > maskValue) {

		  best = j;
		  maskValue = *ptrCount;
      
      }
    } // for j

    if ( maskValue != 0 ) {

      // set the mask of the ones above, but not below... interesting...

      for (j = best+1;  j < numberImages; j ++ ) {
		  uint16_t *pixel2;
  
		  pixel2 = (uint16_t*)(pixel +  imageParms->bytesPerLine * j);

		  if (0 != *pixel2) {
			 *pixel2 = 1;
		  } 
      }
    }
  } // for i




}

static void SetBestAlphaChannel8bits(unsigned char *imagesBuffer, int numberImages, pt_tiff_parms *imageParms)
{
  unsigned char *pixel;
  uint16_t *ptrCount;
  uint16_t best;
  uint16_t maskValue;
  int column;
  int j;


  for  (column=0, pixel = imagesBuffer;  column < imageParms->imageWidth; column++, pixel +=4) {

    best = 0;
    ptrCount = (uint16_t*)(pixel + 2);
    maskValue = *ptrCount;

    // find the image with the highest value

    for (j = 1; j < numberImages; j ++) {

      ptrCount = (uint16_t*)(pixel + imageParms->bytesPerLine * j  + 2);

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


/**
 * imageParameters contains the dimensions of the output image, which may not be 
 * the same as the dimensions fo the input images if they are "cropped"
 */
static int CreateAlphaChannels(fullPath *masksNames, fullPath *alphaChannelNames,
										 int numberImages, pt_tiff_parms *fullSizeImageParameters)
{
  TIFF **tiffMasks;
  TIFF **tiffAlphaChannels;
  unsigned char *imagesBuffer;
  unsigned char *ptrBuffer;
  unsigned char *ptrPixel;
  int index;
  char tempString[24];
  
  int returnValue = 0;  
  pt_tiff_parms inputImageParameters;
  int croppedImageRowIndex;
  int fullSizeRowIndex;
  CropInfo crop_info;

  
  // Allocate arrays of TIFF* for the input and output
  // images. process is one row at a time, with all images
  // processed at the same time
  tiffMasks         = (TIFF**)calloc(numberImages, sizeof(TIFF*));
  tiffAlphaChannels = (TIFF**)calloc(numberImages, sizeof(TIFF*));
  
  // The imagesBuffer contains as many rows as we have input images, and 
  // each row is as wide as the final output image
  imagesBuffer = calloc(numberImages, fullSizeImageParameters->bytesPerLine);
  
  if (imagesBuffer == NULL || tiffAlphaChannels == NULL || imagesBuffer == NULL) {
    PrintError("Not enough memory");
    return 0;
  }

  if ( ptQuietFlag == 0 )
    Progress(_initProgress, "Calculating Alpha Channel");

  // Alpha Channel calculation    
  // Open for read
  //       mask files
  //  and  input files
  // Open for write  alpha channel files

  // Open up an input image, then create a corresponding output image...repeat for all images
  for (index = 0; index < numberImages; index++) {
  
    if ((tiffMasks[index] = OpenTiffFromFullPath(&masksNames[index], "r")) == 0) {
      PrintError("Could not open TIFF-file");
      return 0;
    }
    
    TiffGetImageParameters(tiffMasks[index], &inputImageParameters);

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

    if (!TiffSetImageParameters(tiffAlphaChannels[index], &inputImageParameters)) {
      PrintError("Unable to initialize TIFF file\n");
      return 0;
    }
        

  }// finished opening up output files
  
   

  //  fprintf(stderr, "Files have been created, process each row\n");
 
  //iterate one row at a time, and for each row process all images
  for (fullSizeRowIndex = 0; fullSizeRowIndex < fullSizeImageParameters->imageLength; fullSizeRowIndex++) {

    if ( ptQuietFlag == 0 ) {
      if ( fullSizeRowIndex== (fullSizeRowIndex/ 20) * 20 ) {
        sprintf(tempString, "%lu", fullSizeRowIndex * 100/fullSizeImageParameters->imageLength);
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
    //read data from all input images
    for (ptrBuffer = imagesBuffer, index = 0; index < numberImages; index++, ptrBuffer += fullSizeImageParameters->bytesPerLine) {

      memset(ptrBuffer, 0, fullSizeImageParameters->bytesPerLine);
      
      getCropInformationFromTiff(tiffMasks[index], &crop_info);
      croppedImageRowIndex = fullSizeRowIndex - crop_info.y_offset;
      
      if (croppedImageRowIndex >= 0 && croppedImageRowIndex < crop_info.cropped_height ) {
      
        if (TIFFReadScanline(tiffMasks[index], ptrBuffer + (crop_info.x_offset * fullSizeImageParameters->bitsPerPixel/8), croppedImageRowIndex, 0) != 1) {
          PrintError("Error reading temporary TIFF data");
          returnValue = 0;
          goto end;
        }
      }
      RGBAtoARGB(ptrBuffer, fullSizeImageParameters->imageWidth, fullSizeImageParameters->bitsPerPixel);      
      
    }

	 //calculate the alpha channel for this row in all images
    CalculateAlphaChannel(imagesBuffer, numberImages, fullSizeImageParameters);

	 //write out the alpha channel data for this row to all output images
    for (index = 0 , ptrBuffer = imagesBuffer; index < numberImages; index++, ptrBuffer += fullSizeImageParameters->bytesPerLine) {

      ARGBtoRGBA(ptrBuffer, fullSizeImageParameters->imageWidth, fullSizeImageParameters->bitsPerPixel);
      
      getCropInformationFromTiff(tiffMasks[index], &crop_info);
      croppedImageRowIndex = fullSizeRowIndex - crop_info.y_offset;
            
      if (croppedImageRowIndex >= 0 && croppedImageRowIndex < crop_info.cropped_height ) {

        if ( TIFFWriteScanline(tiffAlphaChannels[index], ptrBuffer + (crop_info.x_offset * fullSizeImageParameters->bitsPerPixel/8), croppedImageRowIndex, 0) != 1) {
          PrintError("Unable to write data to output file\n");
          returnValue = 0;
          goto end;
        }
      }
    }

  } //for fullSizeRowIndex

  returnValue = 1;
  
 end:

  if (!ptQuietFlag) {
    Progress(_setProgress, "100");
    Progress(_disposeProgress, "");
  }

  for (index = 0; index < numberImages; index ++) {
    TIFFClose(tiffMasks[index]);
    TIFFClose(tiffAlphaChannels[index]);
  } // for index.

  free(imagesBuffer);
  free(tiffAlphaChannels);
  free(tiffMasks);

  return returnValue;
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

/**
 * Replaces the alpha channel in each image in inputFiles with a generated
 * mask.  The mask is calculated so as to route the seam between overlapping
 * images through the center of the overlap region...
 */
int AddStitchingMasks(fullPath *inputFiles, fullPath *outputFiles, int numberImages, int featherSize)
{
  fullPath *alphaChannelFiles;
  fullPath *maskFiles;
  int i;
  Image image ;
  char tempString[512];
  pt_tiff_parms imageParameters;
  CropInfo crop_info;

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

  // Get TIFF information from 0th image in input list
  if (!TiffGetImageParametersFromPathName(&inputFiles[0], &imageParameters)) {
    PrintError("Could not read TIFF-file");
    return -1;
  }
  
  getCropInformation(inputFiles[0].name, &crop_info);
  
  // Because the 0th intermediate TIFF file might be a "cropped" file, we 
  // need to update the imageParameters so that the dimensions reflect the
  // the size of the full-sized output image, rather than one of the 
  // (potentially) cropped intermediate files
  setFullSizeImageParameters(&imageParameters, &crop_info);  
  
  if (!CreateAlphaChannels(maskFiles, alphaChannelFiles, numberImages, &imageParameters)) {
    PrintError("Could not create alpha channels");
    return -1;
  }

  // From this point on we do not need to process all files at once. This will save temporary disk space
  
  for (i=0;i<numberImages;i++) {
    fullPath withAlphaChannel;

    sprintf(tempString, "%d", 100 * i/ numberImages);

    if ( ptQuietFlag == 0 ) {
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
    if (!ReplaceAlphaChannel(&inputFiles[i], &alphaChannelFiles[i], &withAlphaChannel)) {
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

void BlendLayers8Bit(unsigned char **imageDataBuffers, int counterImageFiles, char *resultBuffer, 
							int lines,
							int imageWidth, int scanLineSize)

{

  // 0x8(%ebp)    imageDataBuffers
  // 0xc(%ebp)    counterImageFiles
  // 0x10(%ebp)   resultBuffer
  // 0x14(%ebp)   lines
  // 0x18(%ebp)   imageWidth
  // 0x1c(%ebp)   scanLineSize

  // 0xffffffdc(%ebp)  imageIndex
  // 0xffffffe0(%ebp)  alphaChannel
  // 0xffffffe4(%ebp)  blue
  // 0xffffffe8(%ebp)  green
  // 0xffffffec(%ebp)  red
  // 0xfffffff0(%ebp)  rowOffset
  // 0xfffffff4(%ebp)  pixelOffset
  // 0xfffffff8(%ebp)  currentLine
  // 0xfffffffc(%ebp)  currentColumn

  int imageIndex = 0;
  unsigned int colours[3];
  unsigned int alphaChannel;
  unsigned int currentLine;
  unsigned int currentColumn;
  unsigned int rowOffset;

  currentLine = 0;

  for (currentLine = 0; currentLine < lines; currentLine++) {

    //printf("Currnet line %d\n", currentLine);

    rowOffset = scanLineSize * currentLine;

    for (currentColumn = 0; currentColumn < imageWidth; currentColumn++) {

      unsigned int pixelOffset;
      unsigned int i;

      //      printf("Currnet column %d\n", currentColumn);

      pixelOffset = rowOffset + currentColumn * 4;


      // Initialize colours for this pixel
      alphaChannel = 0;
      for (i=0;i<3;i++)
		  colours[i] =0;


      // Do alpha blending, from top to bottom. Bail out when alpha channel is equal to maximum

      for (imageIndex =counterImageFiles-1; imageIndex >= 0; imageIndex--) {

		  unsigned int alphaContribution;
		  unsigned char *ptrPixel;
		  unsigned int bottomAlpha;
		  unsigned int index;


		  // printf("Currnet image %d\n", imageIndex);


		  // The alpha blending algorithm is (for premultiplied values)

		  // C_result = C_above + C_below * (1 - alpha_above)
		  // A_result = Alpha_above + alpha_below * (1 - alpha_above)

		  // Find pixel in this layer
		  ptrPixel = imageDataBuffers[imageIndex] + pixelOffset;


		  // printf("TO read pixel\n");

		  bottomAlpha = *(ptrPixel + 3); // this should be the value of the mask for this particular pixel
   
		  // printf("After read pixel\n");

		  alphaContribution = ((0xff - alphaChannel) *  bottomAlpha)/0xff;

		  // I don't really think this step is necessary, but due to innestability of the calculation
		  // alphaContribution it might overflow the byte valuex

		  if ( alphaChannel + alphaContribution > 0xff ) {
			 alphaContribution = 0xff - alphaChannel;
		  }

		  alphaChannel += alphaContribution;

		  // Makek sure the alpha channel is within range
		  assert(alphaChannel >= 0 && alphaChannel <= 0xff);

		  // now do the colours

		  // printf("TO set pixel\n");

		  for (index = 0; index < 3; index ++) {
			 colours[index] += (*(ptrPixel+index) * alphaContribution)/0xff ; // 
			 if (!(colours[index] >= 0 && colours[index] <= 0xff)) {
				printf("PPPPPPPPPPPPPPPPPanic %d index [%d]\n", colours[index], index);
			 }
			 assert(colours[index] >= 0 && colours[index] <= 0xff);
		  }

		  // We don't need to continue if the alpha channel is at the max
		  if ( alphaChannel >= 0xff )
			 break;

      } // for (imageIndex =counterImageFiles-1; imageIndex >= 0; imageIndex--) {

      // Is it really necessary to check the values of the colours and alphachannel to make
      // sure they are not overflowing a byte?
      
      // Set the value of the pixel
      for (i=0;i<3;i++) {
		  assert(colours[i] <= 0xff && colours[i] >= 0);
		  *(resultBuffer + pixelOffset + i) = colours[i];
      }

      *(resultBuffer + pixelOffset + 3) = alphaChannel;


    } //(currentColumn < imageWidth)

  } //for currentLine < lines

}

void BlendLayers16Bit(unsigned char **imageDataBuffers, int counterImageFiles, char *resultBuffer, 
							 int lines,
							 int imageWidth, int scanLineSize)

{

  int imageIndex = 0;
  unsigned long long colours[3];
  unsigned long long alphaChannel;
  unsigned int currentLine;
  unsigned int currentColumn;
  unsigned int rowOffset;

  uint16_t *u16ResultBuffer=(uint16_t*)resultBuffer;
  uint16_t **u16ImageDataBuffers=(uint16**)imageDataBuffers;
  
  currentLine = 0;


  for (currentLine = 0; currentLine < lines; currentLine++) {

    //  printf("Lines %d\n", lines);
    //  printf("Width %d\n", imageWidth);
    //  printf("length %d\n", scanLineSize);


	 //printf("Currnet line %d\n", currentLine);

    // scanLineSize is in bytes, but we need the length in 16bit units
    rowOffset = (scanLineSize/2) * currentLine;

    for (currentColumn = 0; currentColumn < imageWidth; currentColumn++) {

      unsigned int pixelOffset;
      unsigned int i;

      //      printf("Currnet column %d\n", currentColumn);

      pixelOffset = rowOffset + currentColumn * 4;

      //      printf("Currnet offset %d\n", pixelOffset);

      // Initialize colours for this pixel
      alphaChannel = 0;
      for (i=0;i<3;i++)
		  colours[i] =0;


      // Do alpha blending, from top to bottom. Bail out when alpha channel is equal to maximum

      for (imageIndex =counterImageFiles-1; imageIndex >= 0; imageIndex--) {

		  unsigned long long alphaContribution;
		  uint16_t *ptrPixel;
		  unsigned long long bottomAlpha;
		  unsigned int index;


		  // printf("Currnet image %d\n", imageIndex);


		  // The alpha blending algorithm is (for premultiplied values)

		  // C_result = C_above + C_below * (1 - alpha_above)
		  // A_result = Alpha_above + alpha_below * (1 - alpha_above)

		  // Find pixel in this layer
		  ptrPixel = u16ImageDataBuffers[imageIndex] + pixelOffset;


		  // printf("TO read pixel\n");

		  bottomAlpha = *(ptrPixel + 3); // this should be the value of the mask for this particular pixel
   
		  //printf("After read pixel\n");

		  alphaContribution = ((0xffff - alphaChannel) *  bottomAlpha)/0xffff;

		  // I don't really think this step is necessary, but due to innestability of the calculation
		  // alphaContribution it might overflow the byte valuex

		  if ( alphaChannel + alphaContribution > 0xffff ) {
			 alphaContribution = 0xffff - alphaChannel;
		  }

		  alphaChannel += alphaContribution;

		  // Makek sure the alpha channel is within range
		  assert(alphaChannel >= 0 && alphaChannel <= 0xffff);

		  // now do the colours

		  //printf("TO set pixel\n");

		  for (index = 0; index < 3; index ++) {
			 colours[index] += (*(ptrPixel+index) * alphaContribution)/0xffff ; // 
			 if (!(colours[index] >= 0 && colours[index] <= 0xffff)) {
				printf("PPPPPPPPPPPPPPPPPanic %d index [%d]\n", colours[index], index);
			 }
			 assert(colours[index] >= 0 && colours[index] <= 0xffff);
		  }

		  // We don't need to continue if the alpha channel is at the max
		  if ( alphaChannel >= 0xffff )
			 break;

      } // for (imageIndex =counterImageFiles-1; imageIndex >= 0; imageIndex--) {

      // Is it really necessary to check the values of the colours and alphachannel to make
      // sure they are not overflowing a byte?
      //      printf("Done loop\n");      
      // Set the value of the pixel
      for (i=0;i<3;i++) {
		  assert(colours[i] <= 0xffff && colours[i] >= 0);
		  *(u16ResultBuffer + pixelOffset + i) = colours[i];
      }
      //      printf("Done loop 2\n");      
      *(u16ResultBuffer + pixelOffset + 3) = alphaChannel;


    } //(currentColumn < imageWidth)

  } //for currentLine < lines

}




static void BlendLayers(unsigned char **imageDataBuffers, unsigned int counterImageFiles, char *resultBuffer, 
								unsigned int linesToRead,
								unsigned int imageWidth, unsigned int bitsPerPixel, unsigned int scanLineSize)
{

  if (bitsPerPixel == 32) {
    BlendLayers8Bit(imageDataBuffers, counterImageFiles, resultBuffer, linesToRead, imageWidth, scanLineSize);
  } else if (bitsPerPixel == 64) {
    BlendLayers16Bit(imageDataBuffers, counterImageFiles, resultBuffer, linesToRead, imageWidth, scanLineSize);
  }
}

int FlattenTIFF(fullPath *fullPathImages, int counterImageFiles, fullPath *outputFileName, int removeOriginals)
{
           
  //+12   counterImageFil  es
  //+8    fullPathImages

  //  scanLineSize

  /*
	 unsigned int linesToRead;
	 unsigned int offsetThisPass;
	 unsigned int linesPerPass;

	 unsigned int samplesPerPixel;
  */
  TIFF **tiffFileHandles;
  unsigned char **imageDataBuffers;
  unsigned char *resultBuffer;
  
  
  fullPath tmpFullPath;
  char tmpFilename[512];
  pt_tiff_parms otherFileParms;
  pt_tiff_parms imageParameters;
  CropInfo crop_info;
  TIFF *tiffFile;
  unsigned int linesPerPass;

  unsigned int i;
  unsigned int offsetBeforeThisPass = 0;
  int linesLeft = 0;
  unsigned int linesToRead;
  int rowInPass;
  int inputImageRowIndex;
  int outputImageRowIndex;
  unsigned char *pixelPtr;  

  // Get TIFF parameters from the 0th intermediate file...we can reuse most of
  // these for the final flattened output file
  if (!TiffGetImageParametersFromPathName(&fullPathImages[0], &imageParameters)) {
    PrintError("Could not read TIFF-file");
    return -1;
  }
  
  strcpy(tmpFullPath.name, fullPathImages[0].name);
  
  // Read the crop information from the 0th file...this will tell us what 
  // the dimensions of the "full-size" output image should be
  getCropInformation(tmpFullPath.name, &crop_info);
  
  // Because the 0th intermediate TIFF file might be a "cropped" file, we 
  // need to update the imageParameters so that the dimensions reflect the
  // the size of the full-sized output image, rather than one of the 
  // (potentially) cropped intermediate files
  setFullSizeImageParameters(&imageParameters, &crop_info);
  
  //modify "tmpFullPath" to contain the name of a new, empty temp file
  if (makeTempPath(&tmpFullPath) != 0) {
    PrintError("Could not make Tempfile");
    return -1;
  }

  //copy the name of this new tmpFullPath into a string (tmpFilename)
  if (GetFullPath(&tmpFullPath, tmpFilename) != 0) {
    PrintError("Could not get filename");
    return -1;
  }

  //open up temporary output file for writing as a TIFF
  if ((tiffFile = TIFFOpen(tmpFilename, "w")) == 0) {
    PrintError("Could not create TIFF file");
    return -1;
  }

  TiffSetImageParameters(tiffFile, &imageParameters);

  // Calculate number of lines to read at a time so that we are reading 
  // approximately 500 KB at a time from each input file.  This could be 
  // memory intensive if we have an awful lot of images and not much memory, 
  // but probably not a big problem for 99.9% of cases on 99.9% of machines.
  linesPerPass = 500000 / imageParameters.bytesPerLine;

  if (linesPerPass == 0)
    linesPerPass = 1;
  
  // We dont need to read more lines that the size of the file
  if (imageParameters.imageLength < linesPerPass ) {
    linesPerPass = imageParameters.imageLength;
    if (linesPerPass == 0) {
      PrintError("Invalid image length in TIFF file. It might be corrupted");
      return -1;
    }
  }
  
  //Open up all intermediate TIFF files at once
  tiffFileHandles = calloc(counterImageFiles, sizeof(TIFF*));

  for (i = 0; i < counterImageFiles; i ++ ) {
    
    if (GetFullPath(&fullPathImages[i], tmpFilename) != 0) {
      PrintError("Could not get filename");
      return -1;
    }
    
    if ((tiffFileHandles[i] = TIFFOpen(tmpFilename, "r")) == 0) {
      PrintError("Could not open TIFF-Layer %d", i);
      return -1;
    }
    
  }

  // Create as many image data buffers as we have input files.  Note that the 
  // input buffers are as wide as the final output image, which may be more
  // than we technically need if the input images are cropped...it makes the 
  // code simpler, however.
  imageDataBuffers = calloc(counterImageFiles,sizeof(unsigned char*));

  for (i = 0; i < counterImageFiles; i ++) {
    imageDataBuffers[i] = calloc(linesPerPass * imageParameters.bytesPerLine, 1);
	 if (imageDataBuffers[i] == NULL) {
      PrintError("Not enough memory to allocate input buffers");
      return -1;
	 }    
  }

  //we need one buffer to store output result
  resultBuffer = calloc(linesPerPass * imageParameters.bytesPerLine, 1);
  
  if (resultBuffer == NULL) {
    PrintError("Not enough memory to allocate output buffer");
    return -1;
  }

  offsetBeforeThisPass = 0;

  if (ptQuietFlag == 0) {
    Progress(_initProgress, "Flattening Image");
  }

  //  printf("To do %d lines\n", imageParameters.imageLength);

  linesLeft = imageParameters.imageLength;

  // Main flattening loop...iterate over input files, read some data from each, 
  // combine into output buffer, write to file
  while (linesLeft > 0) {

    linesToRead = (linesLeft > linesPerPass) ? linesPerPass : linesLeft;

	 // iterate over each input file
    for (i = 0; i < counterImageFiles; i ++) {
      getCropInformationFromTiff(tiffFileHandles[i], &crop_info);

      // Get a few lines of data from this input file one row at a time
      for (rowInPass = 0; rowInPass < linesToRead; rowInPass++) {
      
        //figure out which row to read/write from input/output images
        outputImageRowIndex = offsetBeforeThisPass + rowInPass;
        inputImageRowIndex  = outputImageRowIndex - crop_info.y_offset;
        
        //point to first byte on this row of the input buffer
        pixelPtr = imageDataBuffers[i]  + (imageParameters.bytesPerLine * rowInPass);
      
		  //clear out any old data, and fill with empty space (zeros)
		  memset(pixelPtr, 0, imageParameters.bytesPerLine);        

		  // Only try to read data if we are reading from a row that exists in the 
		  // input image        
        if (inputImageRowIndex >= 0 && inputImageRowIndex < crop_info.cropped_height) {
          if (TIFFReadScanline(tiffFileHandles[i], pixelPtr + (crop_info.x_offset * imageParameters.bitsPerPixel/8), inputImageRowIndex, 0) != 1) {
            PrintError("Error reading tiff file\n");
            return -1;
          }
		  }
      } 
    }

    //    printf("Passing offsetAfterThisPass [%d] of [%d] linesPerPass  %d \n",offsetAfterThisPass, imageParameters.imageLength, linesPerPass);

    if (ptQuietFlag == 0) {
      sprintf(tmpFilename, "%d", (offsetBeforeThisPass+linesToRead)  * 100 / imageParameters.imageLength);
      if (Progress(_setProgress, tmpFilename) == 0)
        return -1;
    }
    
    // FlattenImageSection
    BlendLayers(imageDataBuffers, counterImageFiles, resultBuffer, linesToRead,
					 imageParameters.imageWidth, imageParameters.bitsPerPixel, imageParameters.bytesPerLine);

    for (i = 0; i < linesToRead; i++) {
      if (TIFFWriteScanline(tiffFile, resultBuffer + imageParameters.bytesPerLine * i, offsetBeforeThisPass + i, 0) != 1) {
        PrintError("Unable to write TIFF to file\n");
        return -1;
      }
    }

    offsetBeforeThisPass += linesToRead;
    linesLeft -= linesToRead;

  } 

  if (!ptQuietFlag) 
    Progress(_disposeProgress, "Done flattening.");

  //  printf("Lines read %d from %d\n", offsetBeforeThisPass,imageParameters.imageLength);

  for (i = 0; i < counterImageFiles; i ++) {
    free(imageDataBuffers[i]);
    TIFFClose(tiffFileHandles[i]);
  }

  TIFFClose(tiffFile);

  if (removeOriginals) {
    for (i = 0; i < counterImageFiles; i ++) {
      remove(fullPathImages[i].name);
    }
  }

  rename(tmpFullPath.name, outputFileName->name);

  free(tiffFileHandles);

  free(imageDataBuffers);
  free(resultBuffer);

  return 0;

}


#ifdef __Win__
//void InsertFileName( fullPath *fp, char *fname ){
// char *c = strrchr((char*)(fp->name), PATH_SEP);
// if(c != NULL) c++;
// else c = fp->name;
// strcpy( c, fname );
//}   
#endif


static int Create_LP_ivr(Image *image, fullPath* fullPathImage)
{
  fprintf(stderr,"Create_LP_ivr this function is not implemented yet\n");
  exit(1);
}

static int Unknown01(Image *image, fullPath* fullPathImage)
{
  fprintf(stderr,"Unknown01 this function is not implemented yet\n");
  exit(1);
}

static int Unknown02(Image *image, fullPath* fullPathImage)
{
  fprintf(stderr,"Unknown02 this function is not implemented yet\n");
  exit(1);
}

static int Unknown03(Image *image, fullPath* fullPathImage)
{
  fprintf(stderr,"Unknown03 this function is not implemented yet\n");
  exit(1);
}

static int Unknown04(Image *image, fullPath* fullPathImage)
{
  fprintf(stderr,"Unknown04 this function is not implemented yet\n");
  exit(1);
}

static int Unknown05(Image *image, fullPath* fullPathImage)
{
  fprintf(stderr,"Unknown05 this function is not implemented yet\n");
  exit(1);
}

static int Create_QTVR(Image *image, fullPath* fullPathImage)
{
  fprintf(stderr,"Create QTVR is not implemented yet\n");
  exit(1);
}

static void ARGtoRGBAImage(Image *im)
{
  int right;    
  int left;
  int bottom;
  int top;
  int width;
  int i;


  if ( im->selection.bottom == 0  &&
       im->selection.right == 0 ) {

    top = 0;
    left = 0;
    bottom = im->height;
    right = im->width;


  }  else {

    top = im->selection.top;
    bottom = im->selection.bottom;
    left = im->selection.left;
    right = im->selection.right;
  }

  width = right - left;

  //fprintf(stderr, "\nWidth %10d Top: %10d bottom %10d Right %10d Left %10d-------", width, top, bottom, right, left);

  assert(width >= 0);
  assert(bottom >= top);

  for (i = 0;  i < bottom - top ; i ++) {

    ARGBtoRGBA(*(im->data) + i * im->bytesPerLine, width, im->bitsPerPixel);

  } // for 

}



void Clear_Area_Outside_Selected_Region(Image *image)
{
  // This function clears (i.e. sets to zero) the area outside the 
  // selection region 

  int right;    
  int left;
  int bottom;
  int top;
  //  int width;
  //  int var24;
  int bytesPerPixel;  // 32
  unsigned char *dataPtr;
  unsigned char *pixelPtr;

  int currentRow;
  int currentColumn;

  // Only works for 8/16 bit per channel (32/64 bits per pixel) images
  assert(image->bitsPerPixel == 32 || image->bitsPerPixel == 64);
  
  top = image->selection.top;
  bottom = image->selection.bottom;
  left = image->selection.left;
  right = image->selection.right;
  
  if ( bottom == 0 )
    bottom = image->height;
 
  if ( right == 0 )
    right = image->width;

  if ( image -> format == _fisheye_circ) {
    PrintError("Not implemented yet");
    exit(1);
  }

  if ( image->bitsPerPixel == 32 ) {
    bytesPerPixel  = 4;
  } else if (image->bitsPerPixel == 64) {
    bytesPerPixel  = 8;    
  } else {
    assert(0); // it should not reach here 
    exit(0);
  }
    
  // Clear the area at above the image
  dataPtr = *(image->data);

  for (currentRow =0; currentRow < top; currentRow++ ) {
    pixelPtr = dataPtr;
        
    for (currentColumn = 0;  currentColumn < image->width ;   currentColumn++) {
      assert(sizeof(int) == bytesPerPixel);
      memset(pixelPtr, 0, bytesPerPixel);
      pixelPtr+=bytesPerPixel;
    }
    
    dataPtr += image->bytesPerLine;
  }
      
  // Clear area below the picture
  dataPtr = bottom * image->bytesPerLine + *(image->data);
  
  for (currentRow=bottom    ;  currentRow < image->height ; currentRow++) {
    pixelPtr = dataPtr;
    for (currentColumn = 0; currentColumn < image->width ; currentColumn++) {
      memset(pixelPtr, 0, bytesPerPixel);
      pixelPtr += bytesPerPixel;
    }
    
    dataPtr += image->bytesPerLine;
    
  } //  for (    ;  %currentColumn < image->width ; currentColumn++,pixelPtr += bytesPerPixel) {
  
  
  /* Clear the area to the left of the picture */
  
  dataPtr = *(image->data);
  for (   currentRow = 0 ; currentRow < image->height;   currentRow++) {
    
    pixelPtr = dataPtr;      
    for (currentColumn = 0 ; currentColumn < left ; currentColumn++) {
      memset(pixelPtr, 0, bytesPerPixel);
      pixelPtr += bytesPerPixel;
    }
    
    dataPtr += image->bytesPerLine;
  }
  
  /* Clear the area to the right of the picture */
  
  dataPtr = *(image->data);
  
  for (currentRow = 0; currentRow < image->height; currentRow++ ) {
    
    pixelPtr = dataPtr + bytesPerPixel * right;
    
    for (currentColumn=right ;currentColumn < image->width; currentColumn++) {
      
      memset(pixelPtr, 0, bytesPerPixel);
      
      pixelPtr += bytesPerPixel;
      
    }
    
    dataPtr += image->bytesPerLine;
    
  }
  
  return;
      

#ifdef not_implemented_yet
  

  //  THIS IS the code for fisheye_circular 24 bits, but I don't understand it yet.

  pixelPtr = right;
  currentColumn = left;

  eax = left + right;


  var20 = (left + right)/2;
  var24 = (top + bottom) /2;
  assert(left >= right);
  temp = (left - right ) /2;
  tmpStr28 = temp * temp;
    

  dataPtr =  *(image->data);
    
  for (currentRow = 0 ; currentRow < image->height ; currentRow++) {
      
	 currentColumn = 0;
	 pixelPtr = dataPtr;
      
	 if ( currentColumn < image->width ) {
        
        
		temp = currentRow - var24;

		var36 = temp * temp;
        
		do { 
          
		  temp = currentColumn - var20;
		  temp = temp * temp + var36;
          
		  if ( %eax > tmpStr28 ) {
            
			 *pixelPtr = 0; //moves only 1 byte
		  }
          
		  currentColumn++;
		  pixelPtr += bytesPerPixel;
          
		} while ( currentColumn < image->width );
        
	 } //    if ( currentColumn < image->width ) {
      
      
	 dataPtr += image-> bytesPerLine;
      
      
  } //for ( ; currentRow < image->height ; ) {
    
  return;
} //if ( image->bitsPerPixel == $0x20 ) {
#endif
    


#ifdef not_implemented_yet

THIS IS the code for fisheye_circular 64 bits

    
var20 = (left + right) /2;
    
var24 = (top + bottom)/2;
    
currentColumn = (left - right )/2;
    
tmpStr28 = currentColumn * currentColumn;
    
dataPtr = *(image->data);
    
for (  currentRow = 0; ; currentRow < image->height ; ++currentRow) {
  //if ( currentRow >= image->height )
  //              return;
      
  currentColumn = 0;
  pixelPtr = dataPtr;
      
  if ( currentColumn < image->width ) {
        
	 var40 = (currentRow * var24) * (currentRow * var24);
        
	 do {
          
		eax = currentColumn * currentColumn + var40;
          
		if ( %eax > tmpStr28 ) {
		  *pixelPtr = 0; // Again CAREFUL, moves 8 bytes
		}
          
		currentColumn ++;
		pixelPtr = bytesPerPixel;
          
	 } while (currentColumn < image->width );
	 //if ( currentColumn < image->width )
	 //       
        
  } //if ( currentColumn < image->width ) {
      
  dataPtr +=image->bytesPerLine;
      
 } //for ( ; %currentRow < image->height ; ) 
#endif

return;

  
}


void Unknown09(Image *currentImagePtr)
{
  // NEEDED
  fprintf(stderr,"Unknown09 this function is not implemented yet\n");
  exit(1);
}


/**
 * This function computes the minimal rectangle needed to encompass
 * the region of the output image (TrPtr->dest) that will be populated with 
 * data from the input image (TrPtr->src) using the options specified
 * in aP.  The ROIRect is populated with the left/right/top/bottom values
 * that define this ROI within the output image
 */
void getROI( TrformStr *TrPtr, aPrefs *aP, PTRect *ROIRect )
{
  struct   MakeParams  mpinv;
  fDesc    invstack[15], finvD;
  int   color               = 0;

  int             x, y, x_jump;
  double             x_d, y_d;  // Cartesian Coordinates of point in source (i.e. input) image
  double           Dx, Dy;      // Coordinates of corresponding point in destination (i.e. output) image

  double             w2   = (double) TrPtr->dest->width  / 2.0 - 0.5;   //half destination image width
  double             h2   = (double) TrPtr->dest->height / 2.0 - 0.5;   //half destination image height
  double             sw2 = (double) TrPtr->src->width   / 2.0 - 0.5;   //half source image width
  double             sh2 = (double) TrPtr->src->height  / 2.0 - 0.5;   //half source image height

  //Set initial values for ROI to be adjusted during this function
  ROIRect->left         = TrPtr->dest->width;
  ROIRect->right        = 0;
  ROIRect->top          = TrPtr->dest->height; 
  ROIRect->bottom       = 0;

  //The "forward" transform (although not used here) allows us to map pixel
  //coordinates in the output image to their location in the source image.
  //SetMakeParams( stack, &mp, &(aP->im) , &(aP->pano), color );
  //fD.func = execute_stack; fD.param = stack;

  //The "inverse" transform allows us to map pixel coordinates in each source image
  //to their location in the output image.
  SetInvMakeParams( invstack, &mpinv, &(aP->im) , &(aP->pano), color );   
  finvD.func = execute_stack_new; 
  finvD.param = invstack;
  
  //iterate over edges of input image and compute left/right/top/bottom-most coordinate
  //in output image
  for (y = 0; y <= TrPtr->src->height; y += 1) {
      
	 x_jump = (y==0 || y==TrPtr->src->height) ? 1 : TrPtr->src->width;
        
	 for (x = 0; x <= TrPtr->src->width; x += x_jump) {
		//convert source coordinates to cartesian coordinates (i.e. origin at center of image)
		x_d = (double) x - sw2 ;
		y_d = (double) y - sh2 ;
        
		//Map the source image cartesian coordinate to the destination image cartesian coordinate
		finvD.func( x_d, y_d, &Dx, &Dy, finvD.param);
        
		//Convert destination cartesian coordinate back to destination "screen" coordinates (i.e. origin at top left of image)
		Dx += w2;
		Dy =  h2 + Dy ;
         
		if( (Dx < TrPtr->dest->width) && (Dy < TrPtr->dest->height) && (Dx >= 0) && (Dy >= 0) ) {
		  //Update ROI if pixel is valid (i.e. inside the final panorama region)
		  if ((int)Dx < ROIRect->left) ROIRect->left = (int)Dx;
		  if ((int)Dx > ROIRect->right) ROIRect->right = (int)Dx;
		  if ((int)Dy < ROIRect->top) ROIRect->top = (int)Dy;
		  if ((int)Dy > ROIRect->bottom) ROIRect->bottom = (int)Dy;
		}
	 }
  }

  //PrintError("ROI: %d,%d - %d, %d", ROIRect->left, ROIRect->top, ROIRect->right, ROIRect->bottom);
}

/**
 * Populates the CropInfo struct with data about cropping of 
 * the TIFF file specified by filename
 */
void getCropInformationFromTiff(TIFF *tif, CropInfo *c)
{
  float x_position, x_resolution, y_position, y_resolution;
   
  //these are the actual, physical dimensions of the TIFF file
  TIFFGetField(tif, TIFFTAG_IMAGEWIDTH, &(c->cropped_width));
  TIFFGetField(tif, TIFFTAG_IMAGELENGTH, &(c->cropped_height));

  //If nothing is stored in these tags, then this must be an "uncropped" TIFF 
  //file, in which case, the "full size" width/height is the same as the 
  //"cropped" width and height
  if (TIFFGetField(tif, TIFFTAG_PIXAR_IMAGEFULLWIDTH, &(c->full_width))==0)   (c->full_width = c->cropped_width);
  if (TIFFGetField(tif, TIFFTAG_PIXAR_IMAGEFULLLENGTH, &(c->full_height))==0) (c->full_height = c->cropped_height);
   
  if (TIFFGetField(tif, TIFFTAG_XPOSITION, &x_position)==0) x_position = 0;
  if (TIFFGetField(tif, TIFFTAG_XRESOLUTION, &x_resolution) == 0) x_resolution = 0;
  if (TIFFGetField(tif, TIFFTAG_YPOSITION, &y_position)==0) y_position = 0;
  if (TIFFGetField(tif, TIFFTAG_YRESOLUTION, &y_resolution) == 0) y_resolution = 0;

  //offset in pixels of "cropped" image from top left corner of 
  //full image (rounded to nearest integer)
  c->x_offset = (uint32)((x_position * x_resolution) + 0.49);
  c->y_offset = (uint32)((y_position * y_resolution) + 0.49);
    
  //printf("%s: %dx%d  @ %d,%d", filename, c->cropped_width, c->cropped_height, c->x_offset, c->y_offset);
}

/**
 * Populates the CropInfo struct with data about cropping of 
 * the TIFF file specified by filename
 */
void getCropInformation(char *filename, CropInfo *c)
{
   
  TIFF *tif = TIFFOpen(filename, "r");
  if (tif == NULL) {
	 PrintError("getCropInformation: Could not open TIFF file");
  } else {
	 getCropInformationFromTiff(tif, c);
	 TIFFClose(tif);
  }
          
}
 
void setFullSizeImageParameters(pt_tiff_parms *imageParameters, CropInfo *crop_info)
{
  // Update the imageParameters so that the dimensions reflect the
  // the size of the full-sized output image, (recorded in the crop_info struct)
  imageParameters->imageLength    = crop_info->full_height;
  imageParameters->imageWidth     = crop_info->full_width;
  imageParameters->bytesPerLine   = imageParameters->imageWidth * (imageParameters->bitsPerPixel/8);
}
  

int CreatePanorama(fullPath ptrImageFileNames[], int counterImageFiles, fullPath *panoFileName, fullPath *scriptFileName)
{

  Image *currentImagePtr;
  aPrefs *prefs;
  int var01;
  int var00;
  int colourCorrection;
  int panoProjection;

  int lines;
  fullPath *fullPathImages;
  int  loopCounter;
  char tmpStr48[8];
  char var40[8];
  char *tempString;        // It looks like a char *temp;          
  char tmpStr28[512];
  char var16[512];
  VRPanoOptions defaultVRPanoOptions;

  char           tmpStr[64];  // string
  fullPath       currentFullPath;
  fullPath       panoName;          // according to documention: QTVR, PNG, PICT, TIFF, etc plus options...*/
  fullPath       tempScriptFile ;
  char           output_file_format[256];
  Image          resultPanorama;    //Output Image
  Image          image1;            //Input Image

  FILE            *regFile;
  char            *regScript;
  unsigned int    regLen;
  unsigned int    regWritten;

  unsigned int    bpp;              // Bits Per Pixel

  TIFF            *tiffFile;       //Output file...will be written during this function
  TrformStr       transform;       //structure holds pointers to input and output images and misc other info

  int ebx;
  
  int croppedOutput = 1, croppedWidth = 0, croppedHeight = 0;
  PTRect ROIRect;
  unsigned int outputScanlineNumber = 0;
  
  /* Variables */
  colourCorrection = 0; // can have values of 1 2 or 3
  var00 = 0;
  var01 = 0;

  //Copy script line for line into a new temporary file
  memcpy(&tempScriptFile , scriptFileName, sizeof(fullPath));
  makeTempPath(&tempScriptFile);
    
  //TIFFSetWarningHandler(tiffErrorHandler);
  //TIFFSetErrorHandler(tiffErrorHandler);
  
  
  if ((regFile = fopen(tempScriptFile.name, "w")) == NULL) {
    PrintError("Could not open temporary Scriptfile");
    goto mainError;
  }
  
  if ((regScript = LoadScript(scriptFileName)) == 0) {
    PrintError("Could not load ScriptFile");
    goto mainError;
  }

  regLen = strlen(regScript);
  
  // Write script to temp file
  regWritten = fwrite(regScript, 1, regLen, regFile);
  
  // Make sure script was written completely
  if (regWritten != strlen(regScript)) {
    PrintError("Could not write temporary script");
    goto mainError;
  }

  fclose(regFile);
  free(regScript);

  //Initialize members to zero
  SetImageDefaults(&image1);
  SetImageDefaults(&resultPanorama);

  //transform structure holds input and output images, and some miscellaneous other information
  transform.src = &image1;            // Input image
  transform.dest = &resultPanorama;   // Output image
  transform.mode = 8;                 // How to run transformation
  transform.success = 1;              // 1 success 0 failure

  //Allocate space to hold fully qualified names of input images
  if ((fullPathImages = malloc(counterImageFiles * 512)) ==  NULL) {
    PrintError("Not enough memory");
    goto mainError;
  }
  
  // This is the main processing loop...it iterates over each input image
  // and maps the pixels in these input images into the output image(s)
  for (loopCounter = 0; loopCounter < counterImageFiles; loopCounter++) {
    
    currentImagePtr = &image1;

    // Read the next adjust line (contains yaw, pitch, roll and other information)
    // for one input image from the script file
    if ((prefs = readAdjustLine(&tempScriptFile)) == 0) {
      PrintError("Could not read Scriptfile");
      goto mainError;
    }
    
    colourCorrection = prefs->sBuf.colcorrect; 
    // This is a strange value:
    // colourCorrection == (i & 3) + (i+1)*4;
    // where i is the number of the reference image
    
    assert(colourCorrection >=0 && colourCorrection < (counterImageFiles+1) *4 );
    if (prefs->pano.cP.radial != 0) {
      assert(0); // I really don't want to execute this code yet
      
      // correct_Prefs
      //...
      //  3 colors x (4 coeffic. for 3rd order polys + correction radius)
      //radial_params[3][5] double (OFFSET 6c4 1732
      //
      //  [0][0..4]  40 bytes      6c4  6cc   6d4   6dc 6e4
      //  [1][0..4]  40 (28)       6ec  6f4   6fc   704 70c
      //  [2][0..4]  40 (28)       714  71c   724   72c 734
      //                            3c
      // radial_params 
      //  3 * 5 * 8 = 120
      
      var00 = prefs->pano.cP.radial_params[0][2]; // what is this for, I have NO idea.
      var00++;
#ifdef asdfasdf
      /*
	I AM NOT TOTALLY SURE ABOUT THIS
	804a01d:   dd 82 d4 06 00 00       fldl   0x6d4(%edx)            // loads address into FL
	804a023:   d9 bd b2 eb ff ff       fnstcw 0xffffebb2(%ebp)           ;;;;;;;;;;;>>> -5198
	804a029:   66 8b 8d b2 eb ff ff    mov    0xffffebb2(%ebp),%cx           ;;;;;;;;;;;>>> -5198
	804a030:   66 81 c9 00 0c          or     $0xc00,%cx
	804a035:   66 89 8d b0 eb ff ff    mov    %cx,0xffffebb0(%ebp)           ;;;;;;;;;;;>>> -5200
	804a03c:   d9 ad b0 eb ff ff       fldcw  0xffffebb0(%ebp)           ;;;;;;;;;;;>>> -5200
	
	804a042:   db 9d 7c e7 ff ff       fistpl 0xffffe77c(%ebp)           ;;;;;;;;;;;>>> -6276 var00
	804a048:   d9 ad b2 eb ff ff       fldcw  0xffffebb2(%ebp)           ;;;;;;;;;;;>>> -5198
	804a04e:   ff 85 7c e7 ff ff       incl   0xffffe77c(%ebp)           ;;;;;;;;;;;>>> -6276 var00
      */
#endif
      
    } // begins 804a00e
	 
	 
    if (prefs->pano.cP.horizontal != 0) {
      assert(0); // I really don't want to see this code executed yet
      
      var01 = prefs->pano.cP.horizontal_params[0] ;// 0x75c //[3] 3 colours x horizontal shift value
      var01++;
      
#ifdef adsfasdf 
      /*
	804a063:   dd 80 5c 07 00 00       fldl   0x75c(%eax)               // loads address into FL
	804a069:   d9 bd b2 eb ff ff       fnstcw 0xffffebb2(%ebp)           ;;;;;;;;;;;>>> -5198
	804a06f:   66 8b 95 b2 eb ff ff    mov    0xffffebb2(%ebp),%dx           ;;;;;;;;;;;>>> -5198
	804a076:   66 81 ca 00 0c          or     $0xc00,%dx
	804a07b:   66 89 95 b0 eb ff ff    mov    %dx,0xffffebb0(%ebp)           ;;;;;;;;;;;>>> -5200
	804a082:   d9 ad b0 eb ff ff       fldcw  0xffffebb0(%ebp)           ;;;;;;;;;;;>>> -5200
	804a088:   db 9d 78 e7 ff ff       fistpl 0xffffe778(%ebp)           ;;;;;;;;;;;>>> -6280  var01
	804a08e:   d9 ad b2 eb ff ff       fldcw  0xffffebb2(%ebp)           ;;;;;;;;;;;>>> -5198
	804a094:   ff 85 78 e7 ff ff       incl   0xffffe778(%ebp)           ;;;;;;;;;;;>>> -6280  var01
      */
#endif
    }
    
    // Copy the current output file name to he fullPathImages[loopCounter]
    memcpy( &fullPathImages[loopCounter], &panoFileName, sizeof(fullPath));
    
    // Create temporary file where output data wil be written
    if (makeTempPath(&fullPathImages[loopCounter]) != 0) {
      PrintError("Could not make Tempfile");
      goto mainError;
    }
    
    // Populate currentFullPath.name with output file name
    GetFullPath(&fullPathImages[loopCounter], currentFullPath.name);
    
    // Open up output file for writing...data will be written in TIFF format
    if ((tiffFile = TIFFOpen(currentFullPath.name, "w")) == 0) {
      PrintError("Could not open %s for writing", currentFullPath.name);
      goto mainError;
    }
    
    // Projection format for final panorama
    panoProjection = prefs->pano.format;
    
    // Copy output pano name to panoName
    memcpy(&panoName,  &prefs->pano.name, sizeof(fullPath));
    //memcpy(&global5640, &prefs->sBuf, sizeof(stBuf));
    
    //panoName.name contains the n"XXX" value from the script "p" lines (e.g. n"TIFF_m" or n"QTVR w400 h300 c1")
    tempString = panoName.name;
    --tempString; /* nextWord does ++ before testing anything, this guarantess proper execution */
    nextWord(output_file_format, &tempString);
    
    //New for PTMender...Use "cropped" TIFFs as intermediate file format when creating
    //most flattened output images (e.g. PNG, JPG), as well as for layered
    //PSD formats.  By default, PTMender works similarly to PTStitcher for 
    //TIFF_m and TIFF_mask formats, using "full size" TIFFs as intermediate 
    //(and output) format, unless the user explicitly requests cropped
    //output by inlcluding "r:CROP" as part of the "p" line (e.g. n"TIFF_m r:CROP")
    //This behavior is retained to maintain compatibility with PTStitcher, although
    //it is clearly less efficient than producting cropped TIFFs as output.
    //
    //Using cropped TIFF as the intermediate format significantly speeds up 
    //processing, with larger panos showing more dramatic increases in speed.  
    //It should also mean that the creation of the "flattened" formats will 
    //be significantly more memory-friendly, as the masking steps and PSD 
    //assembly steps won't need to load images the size of the output file 
    //into memory at once.  Unless the PTMender is fed extremely large input 
    //images, all memory constraints should now be a thing of the past (MRDL - May 2006).
    
    if ( (strstr(output_file_format, "TIFF_") && strstr(panoName.name, "r:CROP")) || 
         strcmp(output_file_format, "PSD_nomask") == 0 ||
         strcmp(output_file_format, "PSD_mask")   == 0 ||
         strcmp(output_file_format, "PSD")        == 0 ||
         strcmp(output_file_format, "JPEG")       == 0 ||
         strcmp(output_file_format, "JPG")        == 0 ||
         strcmp(output_file_format, "TIFF")       == 0 ||
         strcmp(output_file_format, "TIF")        == 0 ||
         strcmp(output_file_format, "PNG")        == 0)
      croppedOutput = 1;
    else
      croppedOutput = 0;
      
    transform.interpolator = prefs->interpolator;
    transform.gamma = prefs->gamma;

    if(ptQuietFlag == 0) {
      sprintf(tmpStr, "Converting Image %d / %d", (loopCounter+1), counterImageFiles);
      Progress(_initProgress, tmpStr );
    }
 
    //Read input image into transform.src
    if (readImage(currentImagePtr, &ptrImageFileNames[loopCounter]) != 0) {
      PrintError("could not read image");
      goto mainError;
    }

    //This "masks" the input image so that some pixels are excluded from 
    //transformation routine during pixel remapping/interpolation 
    if (prefs->im.cP.cutFrame != 0) { // remove frame? 0 - no; 1 - yes
      if (CropImage(currentImagePtr, &(prefs->im.selection)) == 0) {
        prefs->im.selection.left   = 0;  
        prefs->im.selection.right  = 0; 
        prefs->im.selection.bottom = 0; 
        prefs->im.selection.top    = 0; 
      }
    }
    
    //setup width/height of input image
    prefs->im.width = image1.width;
    prefs->im.height = image1.height;

    if (ptQuietFlag == 0) {
      if (Progress(_setProgress, "5") == 0) {
        TIFFClose(tiffFile);
        remove(fullPathImages[loopCounter].name);
        return(-1);
      }
    }
    
    //Try to set reasonable values for output pano width and/or height if not 
    //specified as part of input (Do this only when processing first image in script)
    if (loopCounter == 0) {

      if (prefs->pano.width == 0) {
        // if the pano did not set the width, then try to set it
        if (prefs->im.hfov != 0.0) {
          prefs->pano.width = prefs->im.width * prefs->pano.hfov /prefs->im.hfov;
          prefs->pano.width /=10; // Round to multiple of 10
          prefs->pano.width *=10;
        }
      }

      if (prefs->pano.height == 0)
        prefs->pano.height = prefs->pano.width/2;

      resultPanorama.height = prefs->pano.height;
      resultPanorama.width = prefs->pano.width;

      if (resultPanorama.height == 0 || resultPanorama.width == 0) {
        PrintError("Please set Panorama width/height");
        goto mainError;
      }
    } //End attempt at setting reasonable values for pano width/height


    // Set output width/height for output file 
    if (croppedOutput) {
      getROI( &transform, prefs, &ROIRect);
      //Dimensions determine size of TIFF file
      croppedWidth = (ROIRect.right - ROIRect.left) + 1;
      croppedHeight = (ROIRect.bottom - ROIRect.top)+ 1;

      TIFFSetField(tiffFile, TIFFTAG_IMAGEWIDTH, croppedWidth);
      TIFFSetField(tiffFile, TIFFTAG_IMAGELENGTH, croppedHeight);
    } else {
      TIFFSetField(tiffFile, TIFFTAG_IMAGEWIDTH, resultPanorama.width);
      TIFFSetField(tiffFile, TIFFTAG_IMAGELENGTH, resultPanorama.height);
    }
    
    
    if (image1.bitsPerPixel == 64) {
      bpp = 16;
    } else if (image1.bitsPerPixel == 32) {
      bpp = 8;
    } else {
      PrintError("Image type not supported\n");
      goto mainError;
    }


    // Number of bits per pixel...generally 8 bits per channel (but can also do 16 bits)
    TIFFSetField(tiffFile, TIFFTAG_BITSPERSAMPLE, bpp);
    
    // We always use Photometric RGB (Indicates a RGB TIFF file with no ColorMap.)
    TIFFSetField(tiffFile, TIFFTAG_PHOTOMETRIC, PHOTOMETRIC_RGB); // 0x106

    //Indicates how the components of each pixel are stored.  
    //1 (PLANARCONFIG_CONTIG) is the default and 
    //indicates that the data are stored in "Chunky format".
    //The component values for each pixel are stored contiguously.
    //The order of the components within the pixel is specified by
    //PhotometricInterpretation. For RGB data, the data is stored as
    //RGBRGBRGB...
    TIFFSetField(tiffFile, TIFFTAG_PLANARCONFIG, PLANARCONFIG_CONTIG);

    //Always use 4 samples per pixel (RGB + Alpha channel)
    TIFFSetField(tiffFile, TIFFTAG_SAMPLESPERPIXEL, 4);

    //Packbits compression was used by original PTStitcher and is retained
    //as the default...the option to use the more efficient LZW compression
    //is also provided
    if (strstr(prefs->pano.name, "c:LZW") != NULL) {
      TIFFSetField(tiffFile, TIFFTAG_COMPRESSION, (uint16_t)COMPRESSION_LZW);
      TIFFSetField(tiffFile, TIFFTAG_PREDICTOR, 2);   //using predictor usually increases LZW compression ratio for RGB data
    } else if (strstr(prefs->pano.name, "c:NONE") != NULL) {
      TIFFSetField(tiffFile, TIFFTAG_COMPRESSION, (uint16_t)COMPRESSION_NONE);
    } else if (strstr(prefs->pano.name, "c:DEFLATE") != NULL) {
      TIFFSetField(tiffFile, TIFFTAG_COMPRESSION, COMPRESSION_DEFLATE);
    }
    else { // Default is PACKBITS
      TIFFSetField(tiffFile, TIFFTAG_COMPRESSION, COMPRESSION_PACKBITS);
    }


    //"1" indicates that The 0th row represents the visual top of the image, 
    //and the 0th column represents the visual left-hand side.
    TIFFSetField(tiffFile, TIFFTAG_ORIENTATION, 1);

    //TIFFTAG_ROWSPERSTRIP indicates the number of rows per "strip" of TIFF data.  The original PTStitcher
    //set this value to the panorama height whch meant that the entire image
    //was contained in one strip.  This is not only explicitly discouraged by the 
    //TIFF specification ("Use of a single strip is not recommended. Choose RowsPerStrip 
    //such that each strip is about 8K bytes, even if the data is not compressed, 
    //since it makes buffering simpler for readers. The 8K value is fairly 
    //arbitrary, but seems to work well."), but is also makes it impossible
    //for programs to read the output from Pano Tools to perform random 
    //access on the data which leads to unnecessarily inefficient approaches to 
    //manipulating these images).
    //
    //In practice, most panoramas generated these days (Feb 2006) contain more than 
    //2000 pixels per row (equal to 8KB mentioned above), so it is easiest to
    //hard-code this value to one, which also enables complete random access to 
    //the output files by subsequent blending/processing applications
    
    //PTStitcher code:
    //TIFFSetField(tiffFile, TIFFTAG_ROWSPERSTRIP, (croppedOutput ? croppedHeight : resultPanorama.height) );
    
    //New-and-improved PTMender code:
    TIFFSetField(tiffFile, TIFFTAG_ROWSPERSTRIP, 1);

    if (croppedOutput) {
      //If writing cropped output, these tags write the postion offset (from top left)
      //into TIFF metadata. 
      
      
      //The X offset in ResolutionUnits of the left side of the image, with 
      //respect to the left side of the page.
      TIFFSetField(tiffFile, TIFFTAG_XPOSITION, (float)(ROIRect.left) / 150.0 );
      
      //The Y offset in ResolutionUnits of the top of the image, with 
      //respect to the top of the page.
      TIFFSetField(tiffFile, TIFFTAG_YPOSITION, (float)(ROIRect.top) / 150.0 );

      //The number of pixels per ResolutionUnit in the ImageWidth
      TIFFSetField(tiffFile, TIFFTAG_XRESOLUTION, (float)150.0);
      
      //The number of pixels per ResolutionUnit in the ImageLength (height)
      TIFFSetField(tiffFile, TIFFTAG_YRESOLUTION, (float)150.0);
      
      //The size of the picture represented by an image.  Note: 2 = Inches.  This
      //is required so that the computation of pixel offset using XPOSITION/YPOSITION and
      //XRESOLUTION/YRESOLUTION is valid (See tag description for XPOSITION/YPOSITION).
      TIFFSetField(tiffFile, TIFFTAG_RESOLUTIONUNIT, (uint16_t)2);      
      
      // TIFFTAG_PIXAR_IMAGEFULLWIDTH and TIFFTAG_PIXAR_IMAGEFULLLENGTH
      // are set when an image has been cropped out of a larger image.  
      // They reflect the size of the original uncropped image.
      // The TIFFTAG_XPOSITION and TIFFTAG_YPOSITION can be used
      // to determine the position of the smaller image in the larger one.
      TIFFSetField(tiffFile, TIFFTAG_PIXAR_IMAGEFULLWIDTH, resultPanorama.width);
      TIFFSetField(tiffFile, TIFFTAG_PIXAR_IMAGEFULLLENGTH, resultPanorama.height);      
    }

    //The resultPanorama.selection determines which region of the output image
    //is iterated over during the main pixel-remapping processing logic.  Much
    //of the image will be empty (black space) for any given input image.  However,
    //if cropped output is selected, then only the region of interest (ROI) into
    //which this input image will be mapped is processed...this significantly
    //speeds up processing
    if (croppedOutput) {
      resultPanorama.selection.left     = ROIRect.left;
      resultPanorama.selection.right    = ROIRect.right +1;  // the right edge is actually the pixel NOT in the pano
      resultPanorama.selection.top      = ROIRect.top;      
    } else {
      resultPanorama.selection.left     = 0;
      resultPanorama.selection.right    = resultPanorama.width;    
      resultPanorama.selection.top      = 0;      
    }

    resultPanorama.bitsPerPixel = image1.bitsPerPixel ;
    resultPanorama.bytesPerLine = TIFFScanlineSize(tiffFile);
    
    //The output image is generated a few lines at a time to make efficient use
    //of limited memory...compute a reasonable number of lines to process (must
    //be at least 1, but no more than output height)
    lines = 500000 / resultPanorama.bytesPerLine;
    
    if (lines == 0)
      lines = 1;
    
    //Don't process more lines than are available
    if (lines > (croppedOutput ? croppedHeight : resultPanorama.height) )
      lines = (croppedOutput ? croppedHeight : resultPanorama.height);
    
    if ((resultPanorama.data  = (unsigned char**)mymalloc(lines * resultPanorama.bytesPerLine ) ) == NULL) {
      PrintError("Not enough memory for output panorama buffer");
      exit(0);
    }
	 //NB resultPanorama.selection.bottom is actually one pixel beyond last row with data.
    resultPanorama.selection.bottom   = resultPanorama.selection.top + lines;
    
    //    printf("bits per pixel %d\n", resultPanorama.bitsPerPixel);
    //    printf("cropped %d\n", croppedOutput);

    if (resultPanorama.bitsPerPixel != image1.bitsPerPixel) {
      PrintError("All source images must have the same number of bits per pixel.");
      exit(0);
    }

    //Copy all position related data (yaw, pitch, roll, etc) for input image to currentImagePtr
    CopyPosition(currentImagePtr, &(prefs->im));

    //image1.selection determines how much of the input image to be 
    //included during main pixel remapping logic
    image1.selection.top =  prefs->im.selection.top ;
    image1.selection.bottom = prefs->im.selection.bottom;
    image1.selection.left = prefs->im.selection.left;
    image1.selection.right = prefs->im.selection.right;

    CopyPosition(&resultPanorama, &(prefs->pano));

    //Set image data outside selection region to zeros

    Clear_Area_Outside_Selected_Region(currentImagePtr);

    //pano.width and height must be equal to the full canvas size (not the 
    //size of the cropped output image...if selected) in order for the pixel 
    //remapping logic to work correctly.
    prefs->pano.width = resultPanorama.width;
    prefs->pano.height = resultPanorama.height;

    //Iterate over the output image multiple lines at a time, remapping pixels
    //from the input image into the output image, and writing data to an
    //output TIFF file.  Finish iterating when we reach the bottom of the 
    //output image (or, in the case of a cropped file, the bottom of the 
    //output ROI).
    outputScanlineNumber = 0;
    while (resultPanorama.selection.top < (croppedOutput ? ROIRect.bottom +1 : resultPanorama.height) ) {
  
      // Call the main pixel remapping routine...all the interpolation happens here
      MakePano(&transform, prefs);
        
      if (transform.success == 0) { // Error 
        PrintError("Error converting image");
        goto mainError;
      }
      
      //Reverse byte order before writing out to TIFF file
      ARGtoRGBAImage(&resultPanorama);

      //Write calculated data rows to TIFF file one row (aka "scanline") at a time
      for (ebx = 0; ebx< resultPanorama.selection.bottom - resultPanorama.selection.top ; ebx++) {
        if (TIFFWriteScanline(tiffFile, *resultPanorama.data + (resultPanorama.bytesPerLine * ebx), outputScanlineNumber, 1) != 1) {
          PrintError("Unable to write to TIFF file\n");
          return -1;
        }
           
        outputScanlineNumber++;
      }
  
      if (ptQuietFlag == 0) {

	//Update progress bar
	if (croppedOutput)
	  sprintf(tmpStr, "%d", (int)( (resultPanorama.selection.bottom-ROIRect.top)*100  / croppedHeight));
	else
	  sprintf(tmpStr, "%d", (int)(resultPanorama.selection.bottom*100  / resultPanorama.height));
	
        if (Progress(_setProgress, tmpStr) == 0) {
          // Cancelled by the user
          TIFFClose(tiffFile);
          remove(tempScriptFile.name);
          remove(fullPathImages[loopCounter].name);
          return(-1);
        }
      }
      
      //specify the next batch of rows to be processed 
      resultPanorama.selection.top = resultPanorama.selection.bottom;
      resultPanorama.selection.bottom = resultPanorama.selection.top + lines;
   
      //Be careful at boundary...end of image
      if ( resultPanorama.selection.bottom > (croppedOutput ? ROIRect.bottom +1: resultPanorama.height) )
        resultPanorama.selection.bottom = (croppedOutput ? ROIRect.bottom +1: resultPanorama.height);
    }
    
    TIFFClose(tiffFile);
    
    if (image1.data != NULL) {
      myfree((void**)image1.data);
      image1.data = NULL;   
    } 
    
    if (prefs->td != NULL) {
      myfree((void**)prefs->td);
    }
    
    if (prefs->ts != NULL)  {
      myfree((void**)prefs->ts);
    }
    free(prefs);
    
    if (resultPanorama.data != NULL) {
      myfree((void**)resultPanorama.data);
      resultPanorama.data = NULL;
    }
    
  }

  if (!ptQuietFlag) 
    Progress(_disposeProgress, "");

  // This is the end of the pixel remapping for all input images.
  // At this point we should have a collection of TIFF files containing
  // the warped input images.  For TIFF_m format this is all we need.  For
  // other formats, we may need to do extra work (feathering, flattening, etc.)
  
  //----------------------------------------------------------------------
  
  remove(tempScriptFile.name);
  
  if (resultPanorama.data != NULL)
    myfree((void**)resultPanorama.data);

  if (image1.data != NULL)
    myfree((void**)image1.data);
  
  // These functions are to correct and/or brightness.  They are not required for 
  // panoramas that do not need any brightness adjustments.  Moreover, Dersch
  // was not fully satisfied with the quality of results obtained from
  // using these functions, and knew that they could be significantly
  // improved.  In general, I think it best to avoid using these features, 
  // and doing any color/brightness adjustments manually either before
  // or after stitching.  While these functions work OK for some images, some 
  // of the time, they can produce some obviously wrong results in some
  // circumstances...perhaps an area for future improvement, but probably not 
  // as important a feature (now that we have multi-resolution splining 
  // software like Enblend) as when Desrch first added these (MRDL).

  if (var00 != 0) {
    ColourBrightness(fullPathImages,fullPathImages, counterImageFiles, var00 -1, 1);
  }
  
  if (var01 != 0) { //
    ColourBrightness(fullPathImages, fullPathImages, counterImageFiles, var01 - 1, 2);
  } // 

  if (colourCorrection != 0) {
    ColourBrightness(fullPathImages, fullPathImages, counterImageFiles, (colourCorrection / 4) - 1, 0);
  }

  SetVRPanoOptionsDefaults(&defaultVRPanoOptions);

  /* Soo, at this point we have skipped the first word of the panorama:
	  # n"QTVR w400 h300 c1"           additional viewer options in a quoted string together with format
	  #              the following options are recognized:
	  #                  w(width) and h(height) of viewer window (only QTVR on Macs)
	  #                  c(codec: 0-JPEG, 1-Cinepak, 2-Sorenson) (only QTVR on Macs)
	  #                  q(codec quality):
	  #                     0-high,1-normal,2-low    QTVR on Macs
	  #                     0-100(highest)           on other jpeg-formats (PAN, IVR, IVR_java, VRML)
	  #                  g  progressive jpeg (0-no, 1-yes) (PAN, IVR, IVR_java, VRML)
	  #                     Optimized JPEG (0-on(default), 2-disabled), (3-progressive with optimized disabled)
	  #                  p  initial pan angle ( QTVR on Macs, VRML, IVR)
	  #                  v  field of view (QTVR, VRML, IVR)
	  #                  Many more options can be set by editing the viewer scripts
  */
  //int getVRPanoOptions( VRPanoOptions *v, char *line )

  getVRPanoOptions(&defaultVRPanoOptions, tempString);
  
  
  //If we are dealing with an output format that is not TIFF_m or PSD_nomask,
  //then we have to add "masks" to the images before finishing...
  if ( strcmp(output_file_format, "TIFF_m") != 0 && strcmp(output_file_format, "PSD_nomask") != 0 ) {
    // There is no point in adding stitching masks for just one image 
    if (counterImageFiles > 1) {
      if (AddStitchingMasks(fullPathImages, fullPathImages, counterImageFiles, prefs->sBuf.feather)!=0) {
        PrintError("Could not create stitching masks");
        goto mainError;
      }
    }
  }
  
  /************ OUTPUT FORMATS: Multiple TIFF ***************/
  // TIFF_m and TIFF_mask...just rename the intermediate files 
  // that we've already computed with numbers (e.g. img0000.tif, img0001.tif, etc.) 
  // and we are finished processing.
  if ( strcmp(output_file_format, "TIFF_m")==0 || strcmp(output_file_format, "TIFF_mask")==0 ) {

    for (loopCounter = 0; loopCounter < counterImageFiles; loopCounter ++ ) {

      strcpy(var16, panoFileName->name);
      sprintf(var40, "%04d", loopCounter);
      strcat(var16, var40);
      ReplaceExt(var16, ".tif");

      // This renames the currently morphed image to the new filename!
      rename(fullPathImages[loopCounter].name, var16);

    } // end of for loop
    free(fullPathImages);
    return(0);
  }

  /************ OUTPUT FORMATS: Layered PSD ***************/
  // Layered PSD is less simple...we need to assemble the existing
  // intermediate files into a layered photoshop document
  if ( strcmp(output_file_format, "PSD_nomask") == 0 || strcmp(output_file_format, "PSD_mask")==0 ) {
    ReplaceExt(panoFileName->name, ".psd");
    
    if (CreatePSD(fullPathImages,counterImageFiles, panoFileName) != 0) {
      PrintError("Error creating PSD file");
      return(-1);
    }
    
    for (loopCounter = 0; loopCounter < counterImageFiles; loopCounter++) {
      remove(fullPathImages[loopCounter].name);
    }
    
    free(fullPathImages);
    return(0);
  } 
  
  
  /************ OUTPUT FORMATS: Flattened files ***************/
  // All other formats require us to "flatten" the intermediate layers into
  // one final document...general approach is to flatten to a single TIFF file, 
  // and then convert this to the desired output file format (e.g. JPEG, PNG, etc.)
  if (FlattenTIFF(fullPathImages,counterImageFiles,&fullPathImages[0], TRUE) != 0) { 
    PrintError("Error while flattening TIFF-image");
	 goto mainError;
  }

  ReplaceExt(panoFileName->name, ".tif");
  rename(fullPathImages[0].name, panoFileName->name);

  free(fullPathImages);

  //Desired output format is TIFF...no further conversion needed
  if (strcmp(output_file_format, "TIFF") == 0 || strcmp(output_file_format, "TIF") == 0)
    return(0);


  //Read back in again so we can convert to final desired format
  if (readImage(&resultPanorama, panoFileName) != 0) {
    PrintError("Could not read result image %s", panoFileName->name);
    goto mainError;
  }

  remove(panoFileName->name);

  if (strcmp(output_file_format, "QTVR") == 0)
    return Create_QTVR(&resultPanorama, panoFileName);

  if (strcmp(output_file_format, "IVR_java") == 0) { 
    if (panoProjection == 1)
      return Unknown03(&resultPanorama, panoFileName);
    else
      return Unknown02(&resultPanorama, panoFileName);
  }

  if (strcmp(output_file_format, "VRML") == 0)
    return Unknown05(&resultPanorama, panoFileName);

  if (strncmp(output_file_format, "IVR", 3) == 0) {  // compare first 3 characters of it // end at 804ae10
    if (panoProjection == 1)
      return Unknown01(&resultPanorama, panoFileName);
    else
      return Create_LP_ivr(&resultPanorama, panoFileName);
  }

  if (strcmp(output_file_format, "PAN") == 0) {  // 
    return Unknown04(&resultPanorama, panoFileName);
  }  // 804ae10

  if (strcmp(output_file_format, "JPEG") == 0 || strcmp(output_file_format, "JPG") == 0) {
    if (!ptQuietFlag) {
      printf("Creating JPEG (quality %d jpegProgressive %d)\n", defaultVRPanoOptions.cquality, defaultVRPanoOptions.progressive);
    }
    ReplaceExt(panoFileName->name, ".jpg");
    return writeJPEG(&resultPanorama, panoFileName, defaultVRPanoOptions.cquality, defaultVRPanoOptions.progressive);
  }


  if (strcmp(output_file_format, "PSD") == 0) {
    ReplaceExt(panoFileName->name, ".PSD");
    return (writePSD(&resultPanorama, panoFileName));

  } 

  if (strcmp(output_file_format, "PNG") == 0) {
    ReplaceExt(panoFileName->name, ".PNG");
    return (writePNG(&resultPanorama, panoFileName));
  }
  
  PrintError("Panorama output format not supported: %s", output_file_format);


 mainError:
  return(-1);

}

