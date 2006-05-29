/*
 *  PTuncrop
 *
 *  This program takes as input a cropped TIFF and generates an uncropped TIFF
 *
 *  May 2005
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

#define PT_UNCROP_USAGE "PTuncrop [options] <inputFile> <outputFile>\n\n"\
                         "Options:\n"\
                         "-o\t\tOverwrite output file if it exists\n"\
			 "\t-q\t\tQuiet run\n\t-h\t\tShow this message\n"\
                         "\n"

#define PT_UNCROP_VERSION "PTuncrop Version " VERSION ", by Daniel M German\n"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "tiffio.h"
#include "panorama.h"
#include "filter.h"
#include "PTcommon.h"
#include "pttiff.h"

int main(int argc,char *argv[])
{
  char opt;
  int overwrite =0;
  int filesCount;
  TIFF *tiffInput;
  TIFF *tiffOutput;
  pt_tiff_parms tiffInfo;
  CropInfo cropInfo;
  char * inputFile;
  char *outputFile;
  FILE *output;
  char *buffer;
  int row;
  int outputRow ;
  printf(PT_UNCROP_VERSION);

  while ((opt = getopt(argc, argv, "ohq")) != -1) {

// o overwrite
// h       -> help
// q       -> quiet?
    
    switch(opt) {  // fhoqs        f: 102 h:104  111 113 115  o:f:hsq
    case 'o':
      overwrite = 1;
      break;
    case 'q':
      ptQuietFlag = 1;
      break;
    case 'h':
      printf(PT_UNCROP_USAGE);
      exit(0);
    default:
      break;
    }
  }
  filesCount = argc - optind;

  if (filesCount != 2) {
    printf(PT_UNCROP_USAGE);
    exit(0);
  }


  inputFile = argv[optind];
  outputFile = argv[optind+1];

  if (!overwrite) {
    if ((output = fopen(outputFile, "r"))!= NULL) {
	fprintf(stderr, "Output file already exists. Use -o to overwrite\n");
	fclose(output);
	exit(1);
    }
  }
  
  if ((tiffInput = TIFFOpen(inputFile, "r")) == NULL){
    fprintf(stderr, "Unable to open input file [%s]\n", inputFile);
    exit(1);
  }

  getCropInformationFromTiff(tiffInput, &cropInfo);

  if (cropInfo.x_offset == 0 && 
      cropInfo.y_offset == 0 ) {
    fprintf(stderr, "Input file is not a cropped TIFF (but it is a tiff)\n");;
    exit(1);
  }
    
  printf("After open\n");


  if (!TiffGetImageParameters(tiffInput, &tiffInfo)) {
    fprintf(stderr, "Unable to get input file information\n");;
    exit(1);
  }

  printf("After get\n");

  if ((tiffOutput = TIFFOpen(outputFile, "w") )== NULL) {
    fprintf(stderr, "Unable to open output file [%s]\n", outputFile);
    exit(1);
  }
     
  
  tiffInfo.imageWidth = cropInfo.full_width;
  tiffInfo.imageLength = cropInfo.full_height;

  // Set output parameters 
  if (!TiffSetImageParameters(tiffOutput, &tiffInfo)) {
    fprintf(stderr, "Unable to set output parameters\n");;
    exit(1);
  }
  
  // Allocate buffer for line
  buffer = malloc(cropInfo.full_width * tiffInfo.bitsPerPixel/8);
  
  if (buffer == NULL) {
    fprintf(stderr, "Unable to allocate memory for IO buffer\n");
    exit(1);
  }

  // Read one line at a time


  memset(buffer, 0, cropInfo.full_width * tiffInfo.bitsPerPixel/8);
  for (outputRow = 0; outputRow < cropInfo.y_offset; outputRow ++) {
    TIFFWriteScanline(tiffOutput,  buffer, outputRow, 0);
  }

  for (row = 0; row < cropInfo.cropped_height; row ++) {
    memset(buffer, 0, cropInfo.full_width * tiffInfo.bitsPerPixel/8);
    TIFFReadScanline(tiffInput, buffer + cropInfo.x_offset * tiffInfo.bitsPerPixel/8, row, 0);
    TIFFWriteScanline(tiffOutput,  buffer, outputRow, 0);
    outputRow++;
  }

  memset(buffer, 0, cropInfo.full_width * tiffInfo.bitsPerPixel/8);

  for (outputRow = cropInfo.cropped_height + cropInfo.y_offset; outputRow < cropInfo.full_height; outputRow ++) {
    TIFFWriteScanline(tiffOutput,  buffer, outputRow, 0);
  }

  free(buffer);

  TIFFClose(tiffInput);
  TIFFClose(tiffOutput);
  

  return 0;
  
}

