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
#include <sys/types.h>
#include <dirent.h>
#include <unistd.h>

#include <tiffio.h>
#include <filter.h>
#include "panorama.h"

#include "PTmender.h"
#include "PTcommon.h"
#include "ColourBrightness.h"


// Global variables for the program


//stBuf global5640;
int   quietFlag;

VRPanoOptions defaultVRPanoOptions;

int ptDebug = 0;

#define PT_MENDER_VERSION  "PTmender Version 0.4.0, originally written by Helmut Dersch, rewritten by Daniel German\n"

int sorting_function(const void *, const void *);

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
      quietFlag = 1;
      break;
      
    case 'h':
      PrintError("Usage: PTStitcher [options] file1 file2 ...");
      exit(0);
      
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
              exit(0);
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
      if((ptrImageFileNames = realloc(ptrImageFileNames, counter * 512)) == NULL)
      	exit(0);

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
    if (SaveFileAs(&panoFileName, "Save Panorama as... ", "pano") != 0)
      exit(0);

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
      exit(0);
    }
    

    // parse input script and set up an array of input file names
    if (ParseScript(script, &alignInfo) == 0) {
      counter = alignInfo.numIm;
      if (counter != 0) {
      	if ((ptrImageFileNames = malloc(512 * counter)) == NULL) {
      	  PrintError("Not enough memory");
      	  exit(0);
      	}
      	
      	//Iterate over input images and populate input filename array
      	for (ebx = 0; ebx < counter; ebx ++) {
      	  strcpy(ptrImageFileNames[ebx].name, scriptFileName.name); //what does this do?!
      	  InsertFileName(&ptrImageFileNames[ebx], alignInfo.im[ebx].name);
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
      
      if (counter == 0) {
      	PrintError("No Input Images");
      	exit(0);
      }
      
      strcpy(scriptPathName.name, scriptFileName.name);
      
      if (makeTempPath(&scriptPathName) != 0) {
      	PrintError("Could not make Tempfile");
      	exit(0);
      }
      
      if ((scriptFD = fopen(scriptPathName.name, "w")) == NULL) {
      	PrintError("Could not open temporary Scriptfile");
      	exit(0);
      }
      
      temp = fwrite(script, 1, strlen(script), scriptFD); 
      
      if (strlen(script) != temp) {
        PrintError("Could not write temporary Scriptfile");
        exit(0);
      }
      
      fclose(scriptFD);
      
      if ((ptrImageFileNames = malloc(counter * 512)) == NULL) {
      	PrintError("Not enough memory\n");
      	exit(0);
      }
      
      for (inputFileCounter2 = 0; inputFileCounter2 < counter; inputFileCounter2++) {
      	aPrefs* preferences;
      	
      	if ( (preferences = readAdjustLine(&scriptPathName)) == NULL) {
      	  PrintError("Could not read ScriptFile");
      	  exit(0);
      	}
	
    	strcpy(ptrImageFileNames[inputFileCounter2].name, scriptFileName.name); //what does this do?
    	InsertFileName(&ptrImageFileNames[inputFileCounter2], preferences->im.name);  //why are we doing this again?
	
    	if (preferences->td != NULL)
    	  free(preferences->td);
    	
    	if (preferences->ts != NULL)
    	  free(preferences->ts);

    	free(preferences);
	
      } // end of for (inputFileCounter2 = 0; inputFileCounter2 < counter; inputFileCounter2++) {
      
      remove(scriptPathName.name);
      
      if (counter == 0) {
      	PrintError("No Input Images");
      	exit(0);
      }
      
    } //    if (counter == 0)
    free(script); 
    
  }
  
  // By now we should have loaded up the input filename array, the output 
  // panorama name, and the name of the script file (copied to a temporary
  // directory).  Now we can create the output image.
  CreatePanorama(ptrImageFileNames, counter, &panoFileName, &scriptFileName);

  return (0);

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
  char var48[8];
  char var40[8];
  char *tempString;        // It looks like a char *temp;          
  char var28[512];
  char var16[512];

  char           tmpStr[64];  // string
  fullPath       currentFullPath;
  fullPath       panoName;          // according to documention: QTVR, PNG, PICT, TIFF, etc plus options...*/
  fullPath       tempScriptFile ;
  char           word[256];
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
I AM NOT TOTALLY SURE ABOUT THIS
 804a01d:	dd 82 d4 06 00 00    	fldl   0x6d4(%edx)            // loads address into FL
 804a023:	d9 bd b2 eb ff ff    	fnstcw 0xffffebb2(%ebp)           ;;;;;;;;;;;>>> -5198
 804a029:	66 8b 8d b2 eb ff ff 	mov    0xffffebb2(%ebp),%cx           ;;;;;;;;;;;>>> -5198
 804a030:	66 81 c9 00 0c       	or     $0xc00,%cx
 804a035:	66 89 8d b0 eb ff ff 	mov    %cx,0xffffebb0(%ebp)           ;;;;;;;;;;;>>> -5200
 804a03c:	d9 ad b0 eb ff ff    	fldcw  0xffffebb0(%ebp)           ;;;;;;;;;;;>>> -5200

 804a042:	db 9d 7c e7 ff ff    	fistpl 0xffffe77c(%ebp)           ;;;;;;;;;;;>>> -6276 var00
 804a048:	d9 ad b2 eb ff ff    	fldcw  0xffffebb2(%ebp)           ;;;;;;;;;;;>>> -5198
 804a04e:	ff 85 7c e7 ff ff    	incl   0xffffe77c(%ebp)           ;;;;;;;;;;;>>> -6276 var00
#endif
											   
    } // begins 804a00e


    if (prefs->pano.cP.horizontal != 0) {
      assert(0); // I really don't want to see this code executed yet
      
      var01 = prefs->pano.cP.horizontal_params[0] ;// 0x75c //[3] 3 colours x horizontal shift value
      var01++;

#ifdef adsfasdf 
??
 804a063:	dd 80 5c 07 00 00    	fldl   0x75c(%eax)               // loads address into FL
 804a069:	d9 bd b2 eb ff ff    	fnstcw 0xffffebb2(%ebp)           ;;;;;;;;;;;>>> -5198
 804a06f:	66 8b 95 b2 eb ff ff 	mov    0xffffebb2(%ebp),%dx           ;;;;;;;;;;;>>> -5198
 804a076:	66 81 ca 00 0c       	or     $0xc00,%dx
 804a07b:	66 89 95 b0 eb ff ff 	mov    %dx,0xffffebb0(%ebp)           ;;;;;;;;;;;>>> -5200
 804a082:	d9 ad b0 eb ff ff    	fldcw  0xffffebb0(%ebp)           ;;;;;;;;;;;>>> -5200
 804a088:	db 9d 78 e7 ff ff    	fistpl 0xffffe778(%ebp)           ;;;;;;;;;;;>>> -6280  var01
 804a08e:	d9 ad b2 eb ff ff    	fldcw  0xffffebb2(%ebp)           ;;;;;;;;;;;>>> -5198
 804a094:	ff 85 78 e7 ff ff    	incl   0xffffe778(%ebp)           ;;;;;;;;;;;>>> -6280  var01
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
    
    //Check if the output format from "p" line in script requests cropped output
    //Only output cropped format for TIFF_m or TIFF_mask.  Other formats require
    //flattening, and the code to flatten cropped images isn't ready yet
    //TODO Max Lyons 20060228
    if (strstr(prefs->pano.name, "TIFF_m") && strstr(prefs->pano.name, "r:CROP") )
      croppedOutput = 1;
    else
      croppedOutput = 0;
      
    if (croppedOutput) {
      //Currently, cropped output doesn't work with the fast transform logic.  
      //I suspect a bug in the fast transform logic not dealing with the 
      //destination rectangle region correctly.  Will investigate later, but
      //for now disable the fast transform.  The speed gains from using 
      //cropped output far outweight the speed losses from disabling the 
      //fast transform. TODO: Max Lyons 20060228.
      //fastTransformStep = 0;
    }
    
    transform.interpolator = prefs->interpolator;
    transform.gamma = prefs->gamma;

    if(quietFlag == 0) {
      sprintf(tmpStr, "Converting Image %d", loopCounter);    
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

    if (quietFlag == 0) {
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
      croppedWidth = (ROIRect.right - ROIRect.left);// + 1;
      croppedHeight = (ROIRect.bottom - ROIRect.top);// + 1;

      TIFFSetField(tiffFile, TIFFTAG_IMAGEWIDTH, croppedWidth);
      TIFFSetField(tiffFile, TIFFTAG_IMAGELENGTH, croppedHeight);
    } else {
      TIFFSetField(tiffFile, TIFFTAG_IMAGEWIDTH, resultPanorama.width);
      TIFFSetField(tiffFile, TIFFTAG_IMAGELENGTH, resultPanorama.height);
    }
    
    
    if (image1.bitsPerPixel > 47) {
      bpp = image1.bitsPerPixel;
    } else {
      bpp = 8;
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
    if (strstr(prefs->pano.name, "c:LZW") != NULL)
    {
      TIFFSetField(tiffFile, TIFFTAG_COMPRESSION, (uint16)COMPRESSION_LZW);
      TIFFSetField(tiffFile, TIFFTAG_PREDICTOR, 2);   //using predictor usually increases LZW compression ratio for RGB data
    }
    else
    {
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
    //since it makes buffering simpler for readers. The “8K” value is fairly 
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
      TIFFSetField(tiffFile, TIFFTAG_RESOLUTIONUNIT, (uint16)2);      
      
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
      resultPanorama.selection.right    = ROIRect.right;    
      resultPanorama.selection.top      = ROIRect.top;      
    } else {
      resultPanorama.selection.left     = 0;
      resultPanorama.selection.right    = resultPanorama.width;    
      resultPanorama.selection.top      = 0;      
    }

    //if (loopCounter == 0) {
      //Set up metadata about final panorama...need to do this on each pass
      //because if we are using cropped output, then the output panorama size
      //might be different for each input image
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
    //} 

    resultPanorama.selection.bottom   = resultPanorama.selection.top + lines;
    
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
    while (resultPanorama.selection.top < (croppedOutput ? ROIRect.bottom : resultPanorama.height) ) {
  
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
        TIFFWriteScanline(tiffFile, *resultPanorama.data + (resultPanorama.bytesPerLine * ebx), outputScanlineNumber, 1);
        outputScanlineNumber++;
      }
  
      //Update progress bar
      if (croppedOutput)
        sprintf(tmpStr, "%d", (int)( (resultPanorama.selection.bottom-ROIRect.top)*100  / croppedHeight));
      else
        sprintf(tmpStr, "%d", (int)(resultPanorama.selection.bottom*100  / resultPanorama.height));
  
      if (quietFlag == 0) {
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
      if ( resultPanorama.selection.bottom > (croppedOutput ? ROIRect.bottom : resultPanorama.height) )
        resultPanorama.selection.bottom = (croppedOutput ? ROIRect.bottom : resultPanorama.height);
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

  if (!quietFlag) 
    Progress(_disposeProgress, "");

  // This is the end of the pixel remapping for all input images.
  // At this point we should have a collection of TIFF files containing
  // the warped input images.  For TIFF_m format this is all we need.  For
  // other formats, we may need to do extra work (feathering, flattening, etc.)
  
  //----------------------------------------------------------------------
  
  remove(tempScriptFile.name);
  
  if (resultPanorama.data != NULL) {
    myfree((void**)resultPanorama.data);
  }
  if (image1.data != NULL) {
    myfree((void**)image1.data);
  }
  
  // I have the feeling these functions are to correct brigthness
  // And they are related to the assembly above.
  // I have the feeling they are not required for panoramas 
  // that do not need any brightness adjustments

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

  //panoName contains the n"XXX" value from the script "p" lines (e.g. n"TIFF_m" or n"QTVR w400 h300 c1"
  tempString = panoName.name;

  //void nextWord( register char* word, char** ch )

  --tempString; /* nextWord does ++ before testing anything, this guarantess proper execution */
  nextWord(word, &tempString);

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
//int	getVRPanoOptions( VRPanoOptions *v, char *line )

  getVRPanoOptions(&defaultVRPanoOptions, tempString);

  if (strcmp(word, "TIFF_m") == 0) {//

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

  if (strcmp(word, "PSD_nomask") == 0 || strcmp(word, "PSD_m")==0) { // 
    ReplaceExt(panoFileName->name, ".psd");
    
    if (CreatePSD(fullPathImages,counterImageFiles, panoFileName) != 0) {
      PrintError("Error creating PSD file");
      return(-1);
    }
    
    for (loopCounter = 0; loopCounter < counterImageFiles; loopCounter++) { // 
      
      remove(fullPathImages[loopCounter].name);
      
    } // end of for loop beginning at
    
    free(fullPathImages);
    
    return(0);
  } // if strcmp(word, "PSD_nomask."
  

  if (AddStitchingMasks(fullPathImages, fullPathImages, counterImageFiles, prefs->sBuf.feather)!=0) {
    PrintError("Could not create stitching masks");
    goto mainError;
  }

  if (strcmp(word, "TIFF_mask") == 0) { //

    for (loopCounter = 0; loopCounter < counterImageFiles; loopCounter++) { //

      strcpy(var28, panoFileName->name);

      sprintf(var48, "%04d", loopCounter);

      strcat(var28, var48);

      ReplaceExt(var28, ".tif");

      rename(fullPathImages[loopCounter].name, var28);
      
    } // end of for loop started at 

  } //end of if (strcmp(word, "TIFF_mask") == 0)

  if (strcmp(word, "PSD_mask") == 0) {

    ReplaceExt(panoFileName->name, ".psd");

    if (CreatePSD(fullPathImages, counterImageFiles, panoFileName ) != 0) {
      PrintError("Error while creating PSD file");
      goto mainError;
    }

    for (loopCounter = 0; loopCounter < counterImageFiles; loopCounter++ ) { 

      remove(fullPathImages[loopCounter].name);

    } //

    return(0);

  } // if (strcmp(word, "TIFF_mask") == 0)

  /* Above this is multi-layer, below is one single output file */

  if (FlattenTIFF(fullPathImages,counterImageFiles,&fullPathImages[0], TRUE) != 0) { 
    PrintError("Error while flattening TIFF-image");
   goto mainError;
  }

  ReplaceExt(panoFileName->name, ".tif");

  rename(fullPathImages[0].name, panoFileName->name);

  free(fullPathImages);

  if (strcmp(word, "TIFF") == 0) { 
    return(0);
  }

  if (strcmp(word, "TIF") == 0) { 
    return(0);
  }

  if (readImage(&resultPanorama, panoFileName) != 0) {
    PrintError("Could not read result image %s", panoFileName->name);
    goto mainError;
  }

  remove(panoFileName->name);

  if (strcmp(word, "WTVR") == 0) { 

    return Unknown07(&resultPanorama, panoFileName);

  }

  if (strcmp(word, "IVR_java") == 0) { 

    if (panoProjection == 1) {
      return Unknown03(&resultPanorama, panoFileName);
    }

    return Unknown02(&resultPanorama, panoFileName);
    
  } // 

  if (strcmp(word, "VRML") == 0) {  //


    return Unknown05(&resultPanorama, panoFileName);

  }

  if (strncmp(word, "IVR", 3) == 0) {  // compare first 3 characters of it // end at 804ae10

    if (panoProjection == 1) {
      return Unknown01(&resultPanorama, panoFileName);
    } else {
      return Create_LP_ivr(&resultPanorama, panoFileName);
    }
  }

  if (strcmp(word, "PAN") == 0) {  // 
    return Unknown04(&resultPanorama, panoFileName);
  }  // 804ae10

  if (strcmp(word, "JPEG") == 0 || strcmp(word, "JPG") == 0) {
    if (!quietFlag) {
      printf("Creating JPEG (quality %d jpegProgressive %d)\n", defaultVRPanoOptions.cquality, defaultVRPanoOptions.progressive);
    }
    ReplaceExt(panoFileName->name, ".jpg");
//int writeJPEG( Image *im, fullPath *sfile, 	int quality, int progressive )
    return writeJPEG(&resultPanorama, panoFileName, defaultVRPanoOptions.cquality, defaultVRPanoOptions.progressive);
  }


  if (strcmp(word, "PSD") == 0) {  // 

    ReplaceExt(panoFileName->name, ".PSD");

    return (writePSD(&resultPanorama, panoFileName));

  } 

  if (strcmp(word, "PNG") == 0) { //end 

    ReplaceExt(panoFileName->name, ".PNG");

    return (writePNG(&resultPanorama, panoFileName));

  } // 
  assert(0); //It should never reach here 

mainError:
   return(-1);

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


int Create_LP_ivr(Image *image, fullPath* fullPathImage)
{
  fprintf(stderr,"Create_LP_ivr this function is not implemented yet\n");
  exit(1);
}

int Unknown01(Image *image, fullPath* fullPathImage)
{
  fprintf(stderr,"Unknown01 this function is not implemented yet\n");
  exit(1);
}

int Unknown02(Image *image, fullPath* fullPathImage)
{
  fprintf(stderr,"Unknown02 this function is not implemented yet\n");
  exit(1);
}

int Unknown03(Image *image, fullPath* fullPathImage)
{
  fprintf(stderr,"Unknown03 this function is not implemented yet\n");
  exit(1);
}

int Unknown04(Image *image, fullPath* fullPathImage)
{
  fprintf(stderr,"Unknown04 this function is not implemented yet\n");
  exit(1);
}

int Unknown05(Image *image, fullPath* fullPathImage)
{
  fprintf(stderr,"Unknown05 this function is not implemented yet\n");
  exit(1);
}

int Unknown07(Image *image, fullPath* fullPathImage)
{
  fprintf(stderr,"Unknown07 this function is not implemented yet\n");
  exit(1);
}

void ARGtoRGBAImage(Image *im)
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
    var28 = temp * temp;
    

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
          
          if ( %eax > var28 ) {
            
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
    
    var28 = currentColumn * currentColumn;
    
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
          
          if ( %eax > var28 ) {
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


int sorting_function(const void *p1, const void *p2)
{
  return strcmp(p1, p2);
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
	struct 	MakeParams	mpinv;
	fDesc 	invstack[15], finvD;
	int 	color               = 0;

  int             x, y, x_jump;
	double 			    x_d, y_d;	// Cartesian Coordinates of point in source (i.e. input) image
	double 		  	  Dx, Dy;		// Coordinates of corresponding point in destination (i.e. output) image

	double 			    w2 	= (double) TrPtr->dest->width  / 2.0 - 0.5;   //half destination image width
	double 			    h2 	= (double) TrPtr->dest->height / 2.0 - 0.5;   //half destination image height
	double 			    sw2 = (double) TrPtr->src->width   / 2.0 - 0.5;   //half source image width
	double 			    sh2 = (double) TrPtr->src->height  / 2.0 - 0.5;   //half source image height

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
	finvD.func = execute_stack; finvD.param = invstack;
  
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

 

