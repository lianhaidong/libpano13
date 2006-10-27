/*
 *  PTroller $id$
 *
 *  Based on the program PTStitcher by Helmut Dersch.
 *  
 *   Flattens a set of TIFFs into one TIFF
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


#define PT_ROLLER_USAGE "PTroller [options] <tiffFiles>+\n\n"\
                         "Options:\n"\
                         "\t-o <filename>\tOutput filename (defaults to merged.tif)\n"\
                         "\t-s\t\tCompute stitching seams (default not)\n"\
                         "\t-q\t\tQuiet run\n\t-h\t\tShow this message\n"\
                         "\n"

#define PT_ROLLER_VERSION "PTroller Version " VERSION ", originally written by Helmut Dersch, rewritten by Daniel M German\n"

#define DEFAULT_FILENAME "merged.tif"

int main(int argc,char *argv[])
{
    char opt;
    int referenceImage = -1;
    fullPath *ptrInputFiles;
    fullPath *ptrOutputFiles;
    
    int counter;
    char flatOutputFileName[MAX_PATH_LENGTH];
    char *endPtr;
    int filesCount;
    char tempString[MAX_PATH_LENGTH];
    int i;
    int base = 0;
    int flattenFlag =0;
    int outputCurvesType = 0; // if 1 => create Photoshop curve files (.acv)
    int typeCorrection = 0;
    int ptComputeSeams = 0;
    fullPath pathName;

    ptrInputFiles = NULL;

    counter = 0;

    printf(PT_ROLLER_VERSION);

    strcpy(flatOutputFileName, DEFAULT_FILENAME);

    while ((opt = getopt(argc, argv, "o:hqs")) != -1) {

        // o and f -> set output file
        // h       -> help
        // q       -> quiet?
        // k       -> base image, defaults to first
	// s       -> compute seams
    
        switch(opt) {  // fhoqs    f: 102 h:104  111 113 115  o:f:hsq
	case 's':
	    ptComputeSeams = 1;
	    break;
        case 'o':
            if (strlen(optarg) < MAX_PATH_LENGTH) {
                strcpy(flatOutputFileName, optarg);
            } else {
                PrintError("Illegal length for output filename");
            }
            break;
        case 't':
            typeCorrection = strtol(optarg, &endPtr, 10);
            if (errno != 0 || (typeCorrection < 0 || typeCorrection > 2)) {
                PrintError("Invalid integer in -t option");
                return -1;
            }
            break;
        case 'q':
            ptQuietFlag = 1;
            break;
        case 'h':
            printf(PT_ROLLER_USAGE);
            exit(0);
        default:
            break;
        }
    }
  
    filesCount = argc - optind;
  
    if ((ptrInputFiles = calloc(filesCount, sizeof(fullPath))) == NULL || 
        (ptrOutputFiles = calloc(filesCount, sizeof(fullPath))) == NULL)        {
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
        fprintf(stderr, PT_ROLLER_USAGE);
        return -1;
    }

    if (! ptQuietFlag) printf("Flattening image\n");

    if (ptComputeSeams) {
	
	if (! ptQuietFlag) printf("Computing seams for %d files\n", filesCount);
	
	if (panoStitchReplaceMasks(ptrInputFiles, ptrInputFiles, filesCount,
				   0) != 0) {
	    PrintError("Could not create stitching masks");
	    return -1;
	}
    }
    if (StringtoFullPath(&pathName, flatOutputFileName) != 0) { 
        PrintError("Syntax error: Not a valid pathname");
        return(-1);
    }
    panoReplaceExt(pathName.name, ".tif");

    if (!panoFlattenTIFF(ptrInputFiles, filesCount, &pathName, FALSE)) { 
        PrintError("Error while flattening TIFF-image");
        return -1;
    }

    if (ptrInputFiles) free(ptrInputFiles);
    if (ptrOutputFiles) free(ptrOutputFiles);

    return 0;
  
}

