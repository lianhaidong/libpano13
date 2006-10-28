/*
 *  PTblender $id$
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
#include <errno.h>


#include "filter.h"
#include "panorama.h"
#include "PTcommon.h"
#include "ColourBrightness.h"
#include "pttiff.h"

#define DEFAULT_PREFIX "blended%04d"

#define PT_BLENDER_USAGE "PTblender [options] <tiffFiles>+\n\n"\
                         "Options:\n"\
                         "\t-p <prefix>\tPrefix for output filename. Defaults to blended%%4d\n"\
                         "\t-k <index>\tIndex to image to use as a reference (0-based, defaults to 0)\n"\
                         "\t-t [0,1,2]\tType of colour correction: 0 full (default), 1 brightness only, 2 colour only\n"\
                         "\t-c\t\tOutput curves smooth\t\t(Output one per each corrected file)\n"\
                         "\t-m\t\tOutput curves arbitrary map\t(Output one per each corrected file)\n"\
                         "\t-f\t\fForce processing (ignore warnings)\n"\
                         "\t-q\t\tQuiet run\n"\
                         "\t-h\t\tShow this message\n"\
                         "\n"

#define PT_BLENDER_VERSION "PTblender Version " VERSION ", originally written by Helmut Dersch, rewritten by Daniel M German\n"


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
    int base = 0;
    int outputCurvesType = 0; // if 1 => create Photoshop curve files (.acv)
    int typeCorrection = 0;
    int ptForceProcessing;

    ptrInputFiles = NULL;

    counter = 0;

    printf(PT_BLENDER_VERSION);


    strcpy(outputPrefix, "corrected%4d");

    while ((opt = getopt(argc, argv, "p:k:t:fqcmh")) != -1) {

	// o and f -> set output file
	// h       -> help
	// q       -> quiet?
	// k       -> base image, defaults to first
    
	switch(opt) {  // fhoqs        f: 102 h:104  111 113 115  o:f:hsq
	case 'p':
	    if (strlen(optarg) < MAX_PATH_LENGTH) {
		strcpy(outputPrefix, optarg);
	    } else {
		PrintError("Illegal length for output prefix");
		return -1;
	    }
	    break;
	case 'k':
	    referenceImage = strtol(optarg, &endPtr, 10);
	    if (errno != 0) {
		PrintError("Invalid integer in -k option");
		return -1;
	    }
	    break;
	case 't':
	    typeCorrection = strtol(optarg, &endPtr, 10);
	    if (errno != 0 || (typeCorrection < 0 || typeCorrection > 2)) {
		PrintError("Invalid integer in -t option");
		return -1;
	    }
	    break;
	case 'f':
	    ptForceProcessing = 1;
	case 'q':
	    ptQuietFlag = 1;
	    break;
	case 'c':
	    if (outputCurvesType == CB_OUTPUT_CURVE_ARBITRARY) {
		PrintError("Can't use both -c and -m options");
		return -1;
	    }
	    outputCurvesType = CB_OUTPUT_CURVE_SMOOTH;
	    break;
	case 'm':
	    if (outputCurvesType == CB_OUTPUT_CURVE_SMOOTH) {
		PrintError("Can't use both -c and -m options");
		return -1;
	    }
	    outputCurvesType = CB_OUTPUT_CURVE_ARBITRARY;
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

    if (referenceImage < 0 || referenceImage >= filesCount) {
	PrintError(tempString, "Illegal reference image number %d. It should be between 0 and %d\n", 
		referenceImage, filesCount-1);
	return -1;
    }

    //We can't output curves for type 1 or 2 corrections
    if (outputCurvesType != 0) {
	if (typeCorrection!= 0) {
	    PrintError("Output of curves is not supported for correction type %d", typeCorrection);
	    return -1;
	}
    }
    if (panoFileOutputNamesCreate(ptrOutputFiles, filesCount, outputPrefix) == 0) {
	return -1;
    }



    if (!ptForceProcessing) {
	char *temp;
	if ((temp = panoFileExists(ptrOutputFiles, filesCount)) != NULL) {
	    PrintError("Output filename exists %d", temp);
	    return -1;
	}

	if (!panoTiffVerifyAreCompatible(ptrInputFiles, filesCount, TRUE)) {
	    PrintError("TIFFs are not compatible");
	    return -1;
	}
    }

    if (! ptQuietFlag) printf("Colour correcting photo using %d as a base type %d\n", referenceImage, typeCorrection);

    ColourBrightness(ptrInputFiles, ptrOutputFiles, filesCount, referenceImage, typeCorrection, outputCurvesType);

    free(ptrInputFiles);
    ptrInputFiles = ptrOutputFiles;
    ptrOutputFiles = NULL;

    return 0;
  
}

