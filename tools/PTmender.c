/*
 *  PTmender
 *
 *  Based on the program PTStitcher by Helmut Dersch.
 *  
 *  It is intended to duplicate the functionality of original program
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

#define PT_MENDER_VERSION  "PTmender Version " VERSION ", originally written by Helmut Dersch, rewritten by Daniel German\n"

int sorting_function(const void *, const void *);
int hasPathInfo(char *aName);


int main(int argc,char *argv[])
{
  int ebx;
  int inputFileCounter2;
  char *script;
  char *currentParm;
  DIR *directory;
  int sort=0;
  int counter;
  fullPath *ptrImageFileNames;
  AlignInfo alignInfo;
  char var36[512];
  fullPath scriptFileName;
  fullPath panoFileName;

  char opt;

  ptrImageFileNames = NULL;
  counter = 0;
  panoFileName.name[0] = 0;
  scriptFileName.name[0] = 0;

  printf(PT_MENDER_VERSION);

  while ((opt = getopt(argc, argv, "o:f:hsqd")) != -1) {

// o and f -> set output file
// h       -> help?
// q       -> quiet?
// s       -> sort
    
    switch(opt) {  // fhoqs        f: 102 h:104  111 113 115  o:f:hsq
      
    case 'o':   // specifies output file name
      if (StringtoFullPath(&panoFileName, optarg) != 0) { 
        PrintError("Syntax error: Not a valid pathname");
        return(-1);
      }
      break;

    case 'f':   // specifies script name
      if (StringtoFullPath(&scriptFileName,optarg) != 0) { 
        PrintError("Syntax error: Not a valid pathname");
        return(-1);
      }
      break;

    case 's':
      sort = 1;
      break;

    case 'd':
      ptDebug = 1;
      break;
    case 'q':
      ptQuietFlag = 1;
      break;
      
    case 'h':
      PrintError("Usage: PTStitcher [options] file1 file2 ...");
      exit(1);
      
    default:
      break;
    }
  }

  while (optind < argc  ) {
    currentParm = argv[optind];
    optind++;
    // Test if it is a directory?
    if ((directory =  opendir(currentParm)) != NULL) {
      char *edx;
      struct dirent *dirEntry;
    
    ///* This is what readdir returns a pointer to
    //     struct dirent
    //              {
    //                  long d_ino;                 /* inode number */                  0
    //                  off_t d_off;                /* offset to this dirent */         08(
    //                  unsigned short d_reclen;    /* length of this d_name */         0c(
    //                  char d_name [NAME_MAX+1];   /* file name (null-terminated) */   0e(
    //              }
    //*/

      while ((dirEntry =  readdir(directory)) != NULL) {
        edx = dirEntry->d_name;
        if (strcmp(dirEntry->d_name, ".") != 0 || strcmp(dirEntry->d_name, "..") != 0) {
          strcpy(var36, currentParm);
          
          if (var36[strlen(var36)] == '/') {
            // if the name has a trailing /, remove it
            var36[strlen(var36)] = 0;
          }
          
          sprintf(var36+ strlen(var36), "%c%s", '/', dirEntry->d_name);
          
          if (IsTextFile(var36)) {
            if (StringtoFullPath(&scriptFileName,var36) != 0) {
              PrintError("Syntax error: Not a valid pathname");
              return(-1);
            } 
          } else {
            // if it not a text file then assume it is an image
            // Allocate one more slot
            if ((ptrImageFileNames = realloc(ptrImageFileNames, ++counter * 512)) == NULL) {
          PrintError("Not enough memory");
              exit(1);
            }
            
            sort = 1;
            /* move the new filename */
            if (StringtoFullPath(&ptrImageFileNames[counter], var36) != 0) {
              PrintError("Syntax error: Not a valid pathname");
              return(-1);
            }
          }
        }
          
      }
     //    if ((directory =  opendir(currentParm)) != NULL) 
    } else if (IsTextFile(currentParm) == 0) { // checks if the file does not have .txt extension
      // it is a assumed to be an image
      counter++;
      if((ptrImageFileNames = realloc(ptrImageFileNames, counter * 512)) == NULL) {
    PrintError("Not enough memory");
        exit(1);
      }

      if (StringtoFullPath(&ptrImageFileNames[counter-1], currentParm) != 0) {
        PrintError("Syntax error: Not a valid pathname");
        return(-1);
      }
    } else { //It has to be textfile
      if (StringtoFullPath(&scriptFileName, currentParm) !=0) { // success
        PrintError("Syntax error: Not a valid pathname");
        return(-1);
      }
    }
  } // end of while loop  while (optind < argc  ) {
  
  //;;;;;;;; While loop ends here
  
  // This code sets scriptFileName to "./Script.txt" if no other name was given
  if (scriptFileName.name[0] == 0) {
    char *temp;
    
    // set scriptFilename to default path './'
    makePathToHost(&scriptFileName);

    // Then append/replace the filename
    if ((temp = strrchr(scriptFileName.name, PATH_SEP)) != NULL)
      temp++;

    strcpy(temp, "Script.txt");

    if (ptDebug)
      fprintf(stderr, "Using Script.txt by default\n");

  }  // end of if (scriptFileName[0] != 0) {

  // Prompt user to specify output filename if not set via command line
  if (strlen(panoFileName.name) == 0) {

    // This is what todo if at this point we dont have a panorama filename
    if (SaveFileAs(&panoFileName, "Save Panorama as... ", "pano") != 0) {
      PrintError("No filename provided");
      exit(1);
    }

    // So at this point we have script and panorama filenames
    if (counter != 0) {
      sort = 1;
    }
  }
  
  //Sort input file names if requested
  if (counter != 0) {
    if (sort != 0)
      qsort(ptrImageFileNames, counter, 512, sorting_function);
  } else {
    // We don't have any images yet. We read the Script and load them from it.
    if (ptDebug) {
      fprintf(stderr, "Loading script %s\n", scriptFileName.name);
    }

    script = LoadScript(&scriptFileName);
    
    if (script == NULL) {
      PrintError("Could not load script.");
      exit(1);
    }
    

    // parse input script and set up an array of input file names
    if (ParseScript(script, &alignInfo) == 0) {
      counter = alignInfo.numIm;
      if (counter != 0) {
        if ((ptrImageFileNames = malloc(512 * counter)) == NULL) {
          PrintError("Not enough memory");
          exit(1);
        }
        
        //Iterate over input images and populate input filename array
        for (ebx = 0; ebx < counter; ebx ++) {
          //If the image filenames don't appear to have any path information, then 
          //prepend the path to the script (if any) that was specified on the 
          //command line (Note: this was the only behavior in the original 
          //PTStitcher.  It has been moved into this conditional block because
          //the script path could get prepended to an already fully qualified
          //filename...not very useful.
          if ( (hasPathInfo(alignInfo.im[ebx].name)) == 0 )
            strcpy(ptrImageFileNames[ebx].name, scriptFileName.name);
          else
            strcpy(ptrImageFileNames[ebx].name, "");
            
          InsertFileName(&ptrImageFileNames[ebx], alignInfo.im[ebx].name);
	  if (ptDebug) {
	    fprintf(stderr, "Input image %d [%s]\n", ebx, ptrImageFileNames[ebx].name);
	  }

        } // for (ebx = 0; ebx < counter; ebx ++)
      }//  if (counter != 0) 
      
      DisposeAlignInfo(&alignInfo);
    }  // if (ParseScript(script, &alignInfo) == 0)
    
    
    //Copy script to temporary directory, reparse and populate input filename arrray
    if (counter == 0) {
      //  Still no files ... TODO: determine conditions under which this logic is exercised

      // In the original program alignInfo is used as a fullPath
      // I don't understand why, and instead I created a local variable
      // to do it.

      fullPath scriptPathName;
      FILE* scriptFD;
      int temp;

      //an "o" line represents an input image
      counter = numLines(script, 'o');
      
      if (ptDebug) {
	fprintf(stderr, "Processing %d counter images\n", counter);
      }

      if (counter == 0) {
        PrintError("No Input Images");
        exit(1);
      }
      
      strcpy(scriptPathName.name, scriptFileName.name);
      
      if (panoFileMakeTemp(&scriptPathName) == 0) {
        PrintError("Could not make Tempfile");
        exit(1);
      }
      
      if ((scriptFD = fopen(scriptPathName.name, "w")) == NULL) {
        PrintError("Could not open temporary Scriptfile");
        exit(1);
      }
      
      temp = fwrite(script, 1, strlen(script), scriptFD); 
      
      if (strlen(script) != temp) {
        PrintError("Could not write temporary Scriptfile");
        exit(1);
      }
      
      fclose(scriptFD);
      
      if ((ptrImageFileNames = malloc(counter * 512)) == NULL) {
        PrintError("Not enough memory\n");
        exit(1);
      }
      
      for (inputFileCounter2 = 0; inputFileCounter2 < counter; inputFileCounter2++) {
        aPrefs* preferences;
        
        if ( (preferences = readAdjustLine(&scriptPathName)) == NULL) {
          PrintError("Could not read ScriptFile");
          exit(1);
        }
    
    
        //Only prepend the path to the script to the filenames if the filenames
        //don't already have path information
        if ( (hasPathInfo(preferences->im.name)) == 0 )
          strcpy(ptrImageFileNames[inputFileCounter2].name, scriptFileName.name);
        else
          strcpy(ptrImageFileNames[inputFileCounter2].name, "");
        
        InsertFileName(&ptrImageFileNames[inputFileCounter2], preferences->im.name);
    
        if (preferences->td != NULL)
          free(preferences->td);
        
        if (preferences->ts != NULL)
          free(preferences->ts);

        free(preferences);
    
      } // end of for (inputFileCounter2 = 0; inputFileCounter2 < counter; inputFileCounter2++) {
      
      remove(scriptPathName.name);
      
      if (counter == 0) {
        PrintError("No Input Images");
        exit(1);
      }
      
    } //    if (counter == 0)
    free(script); 
    
  }
  
  if (ptDebug) {
    fprintf(stderr, "Processing %d counter images\n", counter);
  }
    
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


int sorting_function(const void *p1, const void *p2)
{
  return strcmp(p1, p2);
}


int hasPathInfo(char *aName)
{
    return ((strchr(aName, PATH_SEP) == NULL) ? 0 : 1);
}
