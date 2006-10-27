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
  int overwrite = 0;
  int filesCount;
  int retVal;
  char *inputFile, *outputFile;
  FILE *testFile;
  
  //Need enough space for a message to be returned if something goes wrong
  
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
    if ((testFile = fopen(outputFile, "r"))!= NULL) {
	fprintf(stderr, "Output file already exists. Use -o to overwrite\n");
	fclose(testFile);
	exit(1);
    }
  }
  
  retVal = panoTiffUnCrop(inputFile, outputFile);
	
  
  exit(retVal);	
  
}

