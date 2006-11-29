/*
 *  PTmasker $Id$
 *
 *  Based on the program PTStitcher by Helmut Dersch.
 *  
 *  Takes a set of tiffs and computes their stitching masks
 *
 *  Oct 2006
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
#include "ptstitch.h"
#include "pttiff.h"

#define PT_MASKER_USAGE "PTmasker [options] <tiffFiles>+\n\n"\
                         "Options:\n"\
                         "\t-p <prefix>\tPrefix for output files (defaults to masked%%4d)\n"\
                         "\t-e <feather>\tSize of the feather (defaults to zero)\n"\
                         "\t-f\t\tForce processing (do not stop at warnings)\n"\
                         "\t-q\t\tQuiet run\n"\
                         "\t-h\t\tShow this message\n"\
                         "\n"

#define PT_MASKER_VERSION "PTmasker Version " VERSION ", originally written by Helmut Dersch, rewritten by Daniel M German\n"

#define DEFAULT_PREFIX    "masked"

int main(int argc,char *argv[])
{
    char opt;
    fullPath *ptrInputFiles;
    fullPath *ptrOutputFiles;
    
    int counter;
    char outputPrefix[MAX_PATH_LENGTH];
    int filesCount;
    int base = 0;
    int ptForceProcessing = 0;
    int feather = 0;

    ptrInputFiles = NULL;

    counter = 0;

    printf(PT_MASKER_VERSION);

    while ((opt = getopt(argc, argv, "p:fqhe:")) != -1) {

        // o and f -> set output file
        // h       -> help
        // q       -> quiet?
        // k       -> base image, defaults to first
	// s       -> compute seams
    
        switch(opt) {  // fhoqs    f: 102 h:104  111 113 115  o:f:hsq
	case 'e':
	    feather = strtol(optarg, NULL, 10);
	    if (errno != 0) {
		PrintError("Illegal value for feather");
		return -1;
	    }
	    break;
	case 'p':
	    if (strlen(optarg) < MAX_PATH_LENGTH) {
		strcpy(outputPrefix, optarg);
	    } else {
		PrintError("Illegal length for output prefix");
		return -1;
	    }
	    break;
        case 'f':
	    ptForceProcessing = 1;
            break;
        case 'q':
            ptQuietFlag = 1;
            break;
        case 'h':
            printf(PT_MASKER_USAGE);
            exit(0);
        default:
            break;
        }
    }
  
    filesCount = argc - optind;
  
    // Allocate memory for filenames
    if ((ptrInputFiles = calloc(filesCount, sizeof(fullPath))) == NULL || 
        (ptrOutputFiles = calloc(filesCount, sizeof(fullPath))) == NULL)        {
        PrintError("Not enough memory");
        return -1;
    }

    // GET input file names
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
        fprintf(stderr, PT_MASKER_USAGE);
        return -1;
    }

    // Generate output file names
    if (panoFileOutputNamesCreate(ptrOutputFiles, filesCount, outputPrefix) == 0) {
	return -1;
    }

#ifdef testingfeather
    panoFeatherFile(ptrInputFiles, ptrOutputFiles, feather);
    exit(1);
#endif

    if (! ptForceProcessing) {
	char *temp;
	if ((temp = panoFileExists(ptrOutputFiles, filesCount)) != NULL) {
	    PrintError("Output filename exists %s. Use -f to overwrite", temp);
	    return -1;
	}
	if (!panoTiffVerifyAreCompatible(ptrInputFiles, filesCount, TRUE)) {
	    PrintError("Input files are not compatible. Use -f to overwrite");
	    return -1;
	}
    }
    if (! ptQuietFlag) printf("Computing seams for %d files\n", filesCount);
	
    if (panoStitchReplaceMasks(ptrInputFiles, ptrOutputFiles, filesCount,
			       feather) != 0) {
	PrintError("Could not create stitching masks");
	return -1;
    }

    if (ptrInputFiles) free(ptrInputFiles);
    if (ptrOutputFiles) free(ptrOutputFiles);

    return 0;
  
}

