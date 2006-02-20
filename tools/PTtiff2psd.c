/*
 *  PTtiff2psd
 *
 *  Based on the program PTStitcher by Helmut Dersch.
 *
 *  Converts a set of TIFF files into a Photoshop PSD file
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
#include <assert.h>


#include "filter.h"
#include "panorama.h"
#include "PTcommon.h"
#include "ColourBrightness.h"

int quietFlag = 0;


#define PT_TIFF2PSD_USAGE "PTtiff2psd [options] <tiffFiles>+\n\n"\
                         "Options:\n"\
			 "\t-o <filename>\tOutput filename (default merged.psd)\n"\
                         "\t-m\t\tAdd stitching mask"\
                         "\t-q\t\tQuiet run\n"\
                         "\t-r\t\tReverse layers\n"\
                         "\t-h\t\tShow this message\n"\
                         "\n"

#define PT_TIFF2PSD_VERSION "PTtiff2psd Version 0.1, based on PTstitcher by Helmut Dersch, rewritten by Daniel M German\n"

#define DEFAULT_OUTPUT_FILENAME "merged.psd"

int main(int argc,char *argv[])
{
  char opt;
  fullPath *ptrInputFiles;
  int counter;
  fullPath outputFilename;
  fullPath *tempFiles;
  int filesCount;
  int base = 0;
  int forceOverwrite = 0;
  int reverseLayers = 0;
  int addMask = 0;
  int featherSize = 0;
  int eraseTempFiles = 0;
  int i;

  ptrInputFiles = NULL;
  counter = 0;

  printf(PT_TIFF2PSD_VERSION);

  if (StringtoFullPath(&outputFilename, DEFAULT_OUTPUT_FILENAME)) {
    PrintError("Not a valid pathnamefor output filename  [%s]", DEFAULT_OUTPUT_FILENAME);
    return(-1);
  }

  while ((opt = getopt(argc, argv, "o:qhfmr")) != -1) {

// o and f -> set output file
// h       -> help
// q       -> quiet?
// k       -> base image, defaults to first
// m       -> add stitching mask
    
    switch(opt) {  // fhoqs        f: 102 h:104  111 113 115  o:f:hsq
    case 'o':
      if (StringtoFullPath(&outputFilename, optarg) !=0) { // success
	PrintError("Not a valid pathname for output filename");
	return(-1);
      }
      break;
    case 'm':
      addMask = 1;
    case 'f':
      forceOverwrite = 1;
      break;
    case 'r':
      reverseLayers = 1;
      break;
    case 'q':
      quietFlag = 1;
      break;
    case 'h':
      printf(PT_TIFF2PSD_USAGE);
      exit(0);
    default:
      break;
    }
  }
  
  filesCount = argc - optind;
  
  if ((ptrInputFiles = calloc(filesCount, sizeof(fullPath))) == NULL) {
    PrintError("Not enough memory");
    return -1;
  }

  base = optind;
  for (; optind < argc; optind++) {
    char *currentParm;
    int index;

    currentParm = argv[optind];

    // By default files are layered with the first at the bottom, and last at the top
    // This option reverses that
    index = optind - base;
    if (reverseLayers) {
      index = filesCount - 1 - index;
      //just in case
      assert(index >= 0);
      assert(index < filesCount);
    } 
    
    if (StringtoFullPath(&ptrInputFiles[index], currentParm) !=0) { // success
      PrintError("Syntax error: Not a valid pathname");
      return(-1);
    }
  }

  if (filesCount <= 0) {
    PrintError("No files specified in the command line");
    fprintf(stderr, PT_TIFF2PSD_USAGE);
    return -1;
  }

  // Check if the output file already exists...

  // Check that all files are compatible

  if (!VerifyTiffsAreCompatible(ptrInputFiles, filesCount)) {
    PrintError("TIFFs are not compatible");
    return -1;
  }

  // Masks processing...

  if (addMask) {

    if (!quietFlag) {
      Progress(_initProgress, "To add stitching mask");
    }

    // Find names for the temporary files

    tempFiles = calloc(filesCount, sizeof(fullPath));
    if (tempFiles == NULL) {
      PrintError("Not enough memory");
      return -1;
    }
    for (i=0;i<filesCount;i++) {
      strcpy(tempFiles[i].name, ptrInputFiles[0].name);
      if (makeTempPath(&tempFiles[i]) != 0) {
	PrintError("Could not make Tempfile");
	return -1;
      }
    }
    if (AddStitchingMasks(ptrInputFiles, tempFiles, filesCount, featherSize) != 0) {
      PrintError("Unable to add stitching masks");
      return -1;
    }
    free(ptrInputFiles);
    ptrInputFiles = tempFiles;
    eraseTempFiles = 1;
  }

  // Finally create the PSD

  if (!quietFlag) {
    char tempString[MAX_PATH_LENGTH + 40];
    sprintf(tempString, "Creating output file %s", outputFilename.name);
    Progress(_initProgress, tempString);
  }

  if (CreatePSD(ptrInputFiles, filesCount, &outputFilename ) != 0) {
    PrintError("Error while creating PSD file");
    return -1;
  }

  if (eraseTempFiles) {
    for (i=0;i<filesCount; i++)
      remove(ptrInputFiles[i].name);
  }
    
  free(ptrInputFiles);

  return 0;

}

