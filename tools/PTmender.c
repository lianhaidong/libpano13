/*
 *  PTmender
 *
 *  Based on the program PTStitcher by Helmut Dersch.
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


// TODO
//    Create_Panorama requires some floating point assembly to be interpreted

#define __DEBUG__

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <dirent.h>
#include <unistd.h>

#include "tiffio.h"
#include "filter.h"
#include "panorama.h"

#include "PTmender.h"
#include "PTcommon.h"
#include "file.h"
#include "ColourBrightness.h"


// Global variables for the program


int ptDebug = 0;

#define DEFAULT_OUTPUT_NAME "pano"

#define PT_MENDER_VERSION  "PTmender Version " VERSION ", originally written by Helmut Dersch, rewritten by Daniel German\n"

#define PT_MENDER_USAGE "PTmender [options] <script filename>\n\n"\
                         "Options:\n"\
                         "\t-o <prefix>\tPrefix for output filename, defaults to " DEFAULT_OUTPUT_NAME "\n"\
                         "\t-q\t\tQuiet run\n"\
                         "\t-h\t\tShow this message\n"\
                         "\n"


int sorting_function(const void *, const void *);
int hasPathInfo(char *aName);

static void panoMenderDuplicateScriptFile(char *scriptFileName, char *script, fullPath  *scriptPathName)
{
    FILE* scriptFD;
    int temp;
    
    strcpy(scriptPathName->name, scriptFileName);
      
    // Get a temp filename for the copy of the script
    if (panoFileMakeTemp(scriptPathName) == 0) {
        PrintError("Could not make Tempfile");
        exit(1);
    }
    
    // Copy the script
    if ((scriptFD = fopen(scriptPathName->name, "w")) == NULL) {
        PrintError("Could not open temporary Scriptfile");
        exit(1);
    }
    
    temp = fwrite(script, 1, strlen(script), scriptFD); 
    
    if (strlen(script) != temp) {
        PrintError("Could not write temporary Scriptfile");
        exit(1);
    }
    
    fclose(scriptFD);
    
}



int main(int argc,char *argv[])
{
    char *script;
    int counter;
    fullPath *ptrImageFileNames;
    AlignInfo alignInfo;
    fullPath scriptFileName;
    fullPath panoFileName;
    int i;

    char opt;

    ptrImageFileNames = NULL;
    counter = 0;
    strcpy(panoFileName.name, DEFAULT_OUTPUT_NAME);
    strcpy(scriptFileName.name, "");

    printf(PT_MENDER_VERSION);
    
    while ((opt = getopt(argc, argv, "o:f:hsqd")) != -1) {
	
	// o       -> set output file
	// h       -> help?
	// q       -> quiet?
	
	switch(opt) { 
	    
	case 'o':   // specifies output file name
	    if (StringtoFullPath(&panoFileName, optarg) != 0) { 
		PrintError("Syntax error: Not a valid pathname");
		return(-1);
	    }
	    break;
	    
	case 'd':
	    ptDebug = 1;
	    break;
	case 'q':
	    ptQuietFlag = 1;
	    break;
      
	case 'h':
	    PrintError(PT_MENDER_USAGE);
	    return -1;
      
	default:
	    break;
	}
    }

    if (optind != argc - 1) {
	PrintError(PT_MENDER_USAGE);
	return -1;
    }

    if (StringtoFullPath(&scriptFileName, argv[optind]) !=0) { // success
	PrintError("Syntax error: Not a valid pathname");
	PrintError(PT_MENDER_USAGE);
	return(-1);
    }

    // Check if we got a filename
    if (strlen(scriptFileName.name) == 0) {
	PrintError("No script name provided\n");
	PrintError(PT_MENDER_USAGE);
	return -1;

    }  // end of if (scriptFileName[0] != 0) {

    // Prompt user to specify output filename if not set via command line
    if (strlen(panoFileName.name) == 0) {
	PrintError("No output filename specified\n");
	PrintError(PT_MENDER_USAGE);
	return -1;
    }
  
    // We don't have any images yet. We read the Script and load them from it.
    if (ptDebug) {
	fprintf(stderr, "Loading script [%s]\n", scriptFileName.name);
    }
    
    script = LoadScript(&scriptFileName);
    
    if (script == NULL) {
	PrintError("Could not load script [%s]", scriptFileName.name);
	return -1;
    }
    
    // parse input script and set up an array of input file names
    if (ParseScript(script, &alignInfo) != 0) {
	PrintError("Unable to parse input script");
	return -1;
    }


    // TODO redo

    // The parser of panotools is really broken. To retrieve each
    // input filename it reads the file,
    // finds the first filename, then removes it, and writes the rest of the file again
    // This is done recursively 

    //an "o" line represents an input image
    counter = numLines(script, 'o');
    if (ptDebug) {
	fprintf(stderr, "Found %d images in script\n", counter);
    }
    
    DisposeAlignInfo(&alignInfo);


    // create a temporary copy we can overwrite
    fullPath scriptPathName;
    panoMenderDuplicateScriptFile(scriptFileName.name, script, &scriptPathName);

    free(script); 

    
    if ((ptrImageFileNames = malloc(counter * 512)) == NULL) {
        PrintError("Not enough memory\n");
        exit(1);
    }
    
    for (i = 0; i < counter; i++) {
        aPrefs* preferences;
        
	if (ptDebug) {
	    fprintf(stderr, "Reading image filename  %d: ", i);
	}
        if ( (preferences = readAdjustLine(&scriptPathName)) == NULL) {
	    PrintError("Could not read ScriptFile");
	    exit(1);
        }
	
        //Only prepend the path to the script to the filenames if the filenames
        //don't already have path information
        if ( (hasPathInfo(preferences->im.name)) == 0 )
	    strcpy(ptrImageFileNames[i].name, scriptFileName.name);
        else
	    strcpy(ptrImageFileNames[i].name, "");
        
        InsertFileName(&ptrImageFileNames[i], preferences->im.name);
	
	if (ptDebug) {
	    fprintf(stderr, "%s\n", ptrImageFileNames[i].name);
	}
        if (preferences->td != NULL)
	    free(preferences->td);
        
        if (preferences->ts != NULL)
	    free(preferences->ts);
	
        free(preferences);
	
    } // end of for (i = 0; i < counter; i++) {
    
    remove(scriptPathName.name);

    // By now we should have loaded up the input filename array, the output 
    // panorama name, and the name of the script file (copied to a temporary
    // directory).  Now we can create the output image.
    return panoCreatePanorama(ptrImageFileNames, counter, &panoFileName, &scriptFileName);

}


//////////////////////////////////////////////////////////////////////

char* Filename(fullPath* path)
{
    char *temp;
    if ((temp = strrchr(path->name, '/')) != NULL) {
	temp++;
    } else {
	temp = path->name;
    }
    return temp;
}


int hasPathInfo(char *aName)
{
    return ((strchr(aName, PATH_SEP) == NULL) ? 0 : 1);
}
