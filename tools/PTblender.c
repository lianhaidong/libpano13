/*
 *  PTblender
 *
 *  Based on the program PTStitcher by Helmut Dersch.
 *  
 *  Implements the colour and brightness correction originally found
 *  in PTStitcher.
 *
 *  Jan 2006
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


#include <stdlib.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <dirent.h>
#include <unistd.h>



#include "filter.h"
#include "panorama.h"
#include "ColourBrightness.h"

int quietFlag = 0;


#define PT_BLENDER_USAGE "PTblender [options] <tiffFiles>+\n\n"\
                         "Options:\n"\
			 "\t-o <prefix>\tPrefix for output filename\n"\
			 "\t-k <index>\tIndex to image to use as a reference (0-based)\n"\
			 "\t-q\t\tQuiet run\n\t-h\t\tShow this message\n"\
                         "\n"

#define PT_BLENDER_VERSION "PTblender Version 0.1, originally written by Helmut Dersch, rewritten by Daniel M German\n"

#define DEFAULT_PREFIX "corrected%04d"
#define DEFAULT_PREFIX_NUMBER_FORMAT "%04d"

int main(int argc,char *argv[])
{
  char opt;
  int referenceImage = 0;
  fullPath *ptrInputFiles;
  fullPath *ptrOutputFiles;

  int counter;
  char outputPrefix[MAX_PATH_LENGTH];
  char *endPtr;
  int filesCount;
  char tempString[MAX_PATH_LENGTH];
  int i;
  int base = 0;

  ptrInputFiles = NULL;
  counter = 0;

  printf(PT_BLENDER_VERSION);


  strcpy(outputPrefix, "corrected%4d");

  while ((opt = getopt(argc, argv, "o::k:hqc")) != -1) {

// o and f -> set output file
// h       -> help
// q       -> quiet?
// k       -> base image, defaults to first
    
    switch(opt) {  // fhoqs        f: 102 h:104  111 113 115  o:f:hsq
    case 'o':
      if (strlen(optarg) < MAX_PATH_LENGTH) {
	strcpy(outputPrefix, optarg);
      } else {
	PrintError("Illegal length for output prefix");
      }
      break;
    case 'k':
      referenceImage = strtol(optarg, &endPtr, 10);
      if (*endPtr != NULL) {
	PrintError("Invalid integer in -k option");
	return -1;
      }
    case 'q':
      quietFlag = 1;
      break;
    case 'h':
      printf(PT_BLENDER_USAGE);
      exit(0);
    default:
      break;
    }
  }
  
  filesCount = argc - optind;
  
  if ((ptrInputFiles = calloc(filesCount, sizeof(fullPath))) == NULL || 
      (ptrOutputFiles = calloc(filesCount, sizeof(fullPath))) == NULL)  {
    PrintError("Not enough memory");
    return -1;
  }

  base = optind;
  for (; optind < argc; optind++) {
    char *currentParm;

    currentParm = argv[optind];

    if (StringtoFullPath(&ptrInputFiles[optind-base], currentParm) !=0) { // success
      PrintError("Syntax error: Not a valid pathname");
      return(-1);
    }
  }

  if (filesCount <= 0) {
    PrintError("No files specified in the command line");
    fprintf(stderr, PT_BLENDER_USAGE);
    return -1;
  }

  if (referenceImage <0 || referenceImage >= filesCount) {
    sprintf(tempString, "Illegal reference image number %d. It should be between 0 and %d\n", 
	    referenceImage, filesCount-1);
    PrintError(tempString);
    return -1;
  }


  // Create output filename

  if (strchr(outputPrefix, '%') == NULL) {
    strcat(outputPrefix, DEFAULT_PREFIX_NUMBER_FORMAT);
  }

  for (i =0; i< filesCount ; i++) {
    char outputFilename[MAX_PATH_LENGTH];

    sprintf(outputFilename, outputPrefix, i);
   
    // Verify the filename is different from the prefix 
    if (strcmp(outputFilename, outputPrefix) == 0) {
      PrintError("Invalid output prefix. It does not generate unique filenames.");
      return -1;
    }

    if (StringtoFullPath(&ptrOutputFiles[i], outputFilename) != 0) { 
      PrintError("Syntax error: Not a valid pathname");
      return(-1);
    }
    ReplaceExt(ptrOutputFiles[i].name, ".tif");
    
    //    fprintf(stderr, "Output filename [%s]\n", ptrOutputFiles[i].name);

  }

  ColourBrightness(ptrInputFiles, ptrOutputFiles, filesCount, referenceImage, 0);

  return 0;
  
}

