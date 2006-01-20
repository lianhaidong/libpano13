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
#include "ColourBrightness.h"


// Global variables for the program


stBuf global5640;
int   quietFlag;

VRPanoOptions defaultVRPanoOptions;
int jpegQuality;
int jpegProgressive;

#ifdef TEST_ENABLE_COLOUR_CORRECTION

int main(int argc, char *argv[])
{
  int i;
  fullPath files[20];

  fprintf(stderr, "Argc %d, arg %s\n", argc, argv[0]);
  if (argc > 20) {
    fprintf(stderr, "Too many files\n");
  }
  for (i = 0; i<argc-1; i++) { 
    fprintf(stderr, "Argc %d, arg %s\n", i, argv[i+1]);
    strcpy(files[i]. name, argv[i+1]);
    
  }
  ColourBrightness(&files, argc-1, 0, 0);
  return 0;
}

#else

#define PT_MENDER_VERSION  "PTmender Version 0.3, originally written by Helmut Dersch, rewritten by Daniel German\n"

int sorting_function(const void *, const void *);

int main(int argc,char *argv[])
{
  // It does nothing yet

  int ebx;
  int var44;
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

  while ((opt = getopt(argc, argv, "o:f:hsq")) != -1) {

// o and f -> set output file
// h       -> help?
// q       -> quiet?
// s       -> sort
    
    switch(opt) {  // fhoqs        f: 102 h:104  111 113 115  o:f:hsq
      
      //        This is 'o' option of switch!
      // 
    case 'o':
      if (StringtoFullPath(&panoFileName, optarg) != 0) { 
	PrintError("Syntax error: Not a valid pathname");
	return(-1);
      }
      break;

    case 'f':
      
      if (StringtoFullPath(&scriptFileName,optarg) != 0) { 
	PrintError("Syntax error: Not a valid pathname");
	return(-1);
      }
      break;
    case 's':
      sort = 1;
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

      //     readdir(DIR *dirp);
      while ((dirEntry =  readdir(directory)) != NULL) {
	edx = dirEntry->d_name;
	if (strcmp(dirEntry->d_name, ".") != 0 ||
	    strcmp(dirEntry->d_name, "..") != 0) {
    
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
    
	    /* if it not a text file then assume it is an image */

	    /* Allocate one more slot */

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
      if((ptrImageFileNames = realloc(ptrImageFileNames, counter * 512)) == NULL) {
	exit(0);
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

    /* Then append/replace the filename */
    
    if ((temp = strrchr(scriptFileName.name, PATH_SEP)) != NULL) {
      temp++;
    }

    strcpy(temp, "Script.txt");

  }  // end of if (scriptFileName[0] != 0) {

  /* Code to set the panorama filename if none given in the command line */
  
  if (strlen(panoFileName.name) == 0) {

    // This is what todo if at this point we dont have a panorama filename
    
    if (SaveFileAs(&panoFileName, "Save Panorama as... ", "pano") != 0) {
      exit(0);
    }
    /* SO at this point we have script and panorama filenames */
    if (counter != 0) {
      sort = 1;
    }
  }
  
  if (counter != 0) {
    
    if (sort != 0) {
      qsort(ptrImageFileNames, counter, 512, sorting_function);
    }
  } else {
    // We don't have any images yet. We read the 
    // Script and load them from it.
    script = LoadScript(&scriptFileName);
    
    if (script == NULL) {
      PrintError("Could not load script.");
      exit(0);
    }
    
    if (ParseScript(script, &alignInfo) == 0) {

      counter = alignInfo.numIm;
      if (counter != 0) {
	if ((ptrImageFileNames = malloc(512 * counter)) == NULL) {
	  PrintError("Not enough memory");
	  exit(0);
	}
	// For loop
	
	for (ebx = 0; ebx < counter; ebx ++) {
	  strcpy(ptrImageFileNames[ebx].name, scriptFileName.name);
	  InsertFileName(&ptrImageFileNames[ebx], alignInfo.im[ebx].name);
	} // for (ebx = 0; ebx < counter; ebx ++) {
      }//  if (counter != 0) 
      
      DisposeAlignInfo(&alignInfo);
    }  // if (ParseScript(script, &alignInfo) == 0)
    
    if (counter == 0) {
      
      //  Still no files ...

      // In the original program alignInfo is used as a fullPath
      // I don't understand why, and instead I created a local variable
      // to do it.


      fullPath scriptPathName;
      FILE* scriptFD;
      int temp;

      counter = numLines(script, 0x6f);
      
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
      
      for (var44 = 0; var44 < counter; var44++) {
	aPrefs* preferences;
	
	if ( (preferences = readAdjustLine(&scriptPathName)) == NULL) {
	  
	  PrintError("Could not read ScriptFile");
	  exit(0);
	  
	}
	
	strcpy(ptrImageFileNames[var44].name, scriptFileName.name);
	InsertFileName(&ptrImageFileNames[var44], preferences->im.name);
	
	if (preferences->td != NULL) {
	  free(preferences->td);
	}
	
	if (preferences->ts != NULL) {
	  free(preferences->ts);
	}
	free(preferences);
	
      } // end of for (var44 = 0; var44 < counter; var44++) {
      
      remove(scriptPathName.name);
      
      if (counter == 0) {
	PrintError("No Input Images");
	exit(0);
      }
      
    } //    if (counter == 0) {
    free(script); 
    
  }
  CreatePanorama(ptrImageFileNames, counter, &panoFileName, &scriptFileName);

  return (0);

}

#endif

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

  char       var5196[64];  // string
  fullPath       currentFullPath;
  fullPath       panoName; /* according to documention: QTVR, PNG, PICT, TIFF, etc plus options...*/
  fullPath       tempScriptFile ;
  char      word[256];
  Image  resultPanorama; //(resultPanorama.width)
  Image  image1;

  FILE *regFile;
  char *regScript;
  unsigned int regLen;
  unsigned int regWritten;

  unsigned int bits;

  TIFF *tiffFile;
  TrformStr transform;

  int ebx;

  /* Variables */
  colourCorrection = 0; // can have values of 1 2 or 3
  var00 = 0;
  var01 = 0;

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
  
  // Copies input script into temp script
  
  regWritten = fwrite(regScript, 1, regLen, regFile);
  
  // Count number of characters in script /
  if (regWritten != strlen(regScript)) {
    PrintError("Could not write temporary script");
    goto mainError;
  }
  // eax contains number of characters in script
      
  fclose(regFile);
  free(regScript);

  SetImageDefaults(&image1);

  SetImageDefaults(&resultPanorama);

  transform.src = &image1;

  transform.dest = &resultPanorama;

  transform.mode = 8; /* How to run transformation */

  transform.success = 1; /* 1 success 0 no */


  if ((fullPathImages = malloc(counterImageFiles * 512)) ==  NULL) {
    
    PrintError("Not enough memory");
    goto mainError;

  }
  // This loop seems to read every image and then do something with it
  
  for (loopCounter = 0; loopCounter < counterImageFiles; loopCounter++) {
    
    currentImagePtr = &image1;

    //aPrefs* readAdjustLine( fullPath *theScript )

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

// seems to copy the current file name to the var6256
    memcpy( &fullPathImages[loopCounter], &panoFileName, sizeof(fullPath));

    if (makeTempPath(&fullPathImages[loopCounter]) != 0) {

      PrintError("Could not make Tempfile");
      goto mainError;
    }

    GetFullPath(&fullPathImages[loopCounter], currentFullPath.name);

    // TIFF* TIFFOpen(const char* filename, const char* mode)

    if ((tiffFile = TIFFOpen(currentFullPath.name, "w")) == 0) {
      PrintError("Could not get filename");
      goto mainError;
    }

    panoProjection = prefs->pano.format;           //move aprefts + 0x698 bytes to var6268

    //copy string at (aprefs + 0xbe0) to var4620
    memcpy(&panoName,  &prefs->pano.name, sizeof(fullPath));

    memcpy(&global5640, &prefs->sBuf, sizeof(stBuf));

    transform.interpolator = prefs->interpolator;
    transform.gamma = prefs->gamma;

    sprintf(var5196, "Converting Image %d", loopCounter);

 
    if(quietFlag == 0) {

      Progress(_initProgress, var5196 );
      
    }
 
    // int readImage( Image *im, fullPath *sfile ) should return 0
    // currentImagePtr points to image1
 
    if (readImage(currentImagePtr, &ptrImageFileNames[loopCounter]) != 0) {
      PrintError("could not read image");
      goto mainError;
    }

    
    if (prefs->im.cP.cutFrame != 0) { // remove frame? 0 - no; 1 - yes
      
      if (CropImage(currentImagePtr, &(prefs->im.selection)) == 0) {
	prefs->im.selection.left  = 0;  
	prefs->im.selection.right = 0; 
	prefs->im.selection.bottom=0; 
	prefs->im.selection.top   = 0; 
      }
    }
    
    prefs->im.width = image1.width;

    prefs->im.height = image1.height;

    if (quietFlag == 0) {

      if (Progress(_setProgress, "5") == 0) {
	  TIFFClose(tiffFile);
	  remove(fullPathImages[loopCounter].name);
	  return(-1);
      }
    }
    
    if (loopCounter == 0) {

      if (prefs->pano.width == 0) {

	// if the pano did not set the width, then try to set it

	if (prefs->im.hfov != 0.0) {

	  prefs->pano.width = prefs->im.width * prefs->pano.hfov /prefs->im.hfov;
	  prefs->pano.width /=10; // Round to multiple of 10
	  prefs->pano.width *=10;
	  
	}
      }

      if (prefs->pano.height == 0) { // 

	prefs->pano.height = prefs->pano.width/2;

      } //


      resultPanorama.height = prefs->pano.height;


      resultPanorama.width = prefs->pano.width;

      if (resultPanorama.height == 0 || resultPanorama.width == 0) {
	PrintError("Please set Panorama width/height");
	goto mainError;
      }

      // Above this only happens with first image. 
    } //if (loopCounter == 0) 

    TIFFSetField(tiffFile, TIFFTAG_IMAGEWIDTH, resultPanorama.width);

    TIFFSetField(tiffFile, TIFFTAG_IMAGELENGTH, resultPanorama.height);

    if (image1.bitsPerPixel > 47) {
      bits = image1.bitsPerPixel;
    } else {
      bits = 8;
    }

    TIFFSetField(tiffFile, TIFFTAG_BITSPERSAMPLE, bits);

    assert(PHOTOMETRIC_RGB == 2);
    TIFFSetField(tiffFile, TIFFTAG_PHOTOMETRIC, PHOTOMETRIC_RGB); // 0x106

    assert(PLANARCONFIG_CONTIG == 1);
    assert(TIFFTAG_PLANARCONFIG == 0x11c);
    TIFFSetField(tiffFile, TIFFTAG_PLANARCONFIG, PLANARCONFIG_CONTIG); // 0x11c

    assert(TIFFTAG_SAMPLESPERPIXEL == 0x115);
    TIFFSetField(tiffFile, TIFFTAG_SAMPLESPERPIXEL, 4);

    assert(0x8005 == COMPRESSION_PACKBITS);
    TIFFSetField(tiffFile, TIFFTAG_COMPRESSION, COMPRESSION_PACKBITS);

    TIFFSetField(tiffFile, 0x112, 1);

    TIFFSetField(tiffFile, 0x116, resultPanorama.height);

    //////////////////////

    if (loopCounter == 0) {

/// THIS AGAIN HAPPENS ONLY WITH loopCounter == 0


      resultPanorama.bitsPerPixel = image1.bitsPerPixel ;

      resultPanorama.bytesPerLine = TIFFScanlineSize(tiffFile);

      resultPanorama.selection.left = 0;

      resultPanorama.selection.right = resultPanorama.width;

      lines = 500000 / resultPanorama.bytesPerLine;
 
      if (0 == lines) { 
	lines = 1;
      } 

      if (resultPanorama.height < lines) {  
	lines = resultPanorama.height;
      } 
      if ((
	   resultPanorama.data  = (unsigned char**)mymalloc(lines * resultPanorama.bytesPerLine )
	   ) == NULL) {
	PrintError("Not enough memory");
	exit(0);
      }
    } 

    if (resultPanorama.bitsPerPixel != image1.bitsPerPixel) {
      PrintError("All source images must have the same pixelsize.");
      exit(0);
    }

    resultPanorama.selection.top = 0;
 
    resultPanorama.selection.bottom = lines ;

    CopyPosition(currentImagePtr, &(prefs->im));

    image1.selection.top =  prefs->im.selection.top ;

    image1.selection.bottom = prefs->im.selection.bottom;

    image1.selection.left = prefs->im.selection.left;

    image1.selection.right = prefs->im.selection.right;

    CopyPosition(&resultPanorama, &(prefs->pano));

    Clear_Area_Outside_Selected_Region(currentImagePtr);

    prefs->pano.width = resultPanorama.width;

    prefs->pano.height = resultPanorama.height;

  makePano:

    MakePano(&transform, prefs);
      
    if (transform.success == 0) { //* Error 
      PrintError("Error converting image");
      goto mainError;
    }
    
    ARGtoRGBAImage(&resultPanorama);
    
    for (ebx = 0; ebx< resultPanorama.selection.bottom - resultPanorama.selection.top ; ebx++) {
      
      // Be careful here. resultPanorama.data is a char **, this arithmetic might be plain wrong. I suspect
      // it will segfault. We'll see

      TIFFWriteScanline(tiffFile, *resultPanorama.data + (resultPanorama.bytesPerLine * ebx), resultPanorama.selection.top + ebx  , 1);

      assert(resultPanorama.selection.bottom >= resultPanorama.selection.top);

    } // end of inner for loop

    sprintf(var5196, "%d", (int)(resultPanorama.selection.bottom*100  /resultPanorama.height)); /// 


    if (quietFlag == 0) {
      
      if (Progress(_setProgress, var5196) == 0) {
	// Cancelled by the user
	TIFFClose(tiffFile);
	remove(tempScriptFile.name);
	remove(fullPathImages[loopCounter].name);
	return(-1);
      }
    }
    
    resultPanorama.selection.top = resultPanorama.selection.bottom;
    
    resultPanorama.selection.bottom = resultPanorama.selection.top + lines;

    if (resultPanorama.selection.top >= resultPanorama.height ) { //
      goto closeTIFF;
    }

    if ( resultPanorama.selection.bottom <= resultPanorama.height) { 
      goto makePano;
      
    }

    resultPanorama.selection.bottom = resultPanorama.height;

    if (resultPanorama.selection.bottom <= resultPanorama.height ) {// 
      goto makePano;
    }

  closeTIFF:
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
  } // This is the end of the for loop loopCounter = 0 ...
  
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

/* All the folloging is strange. Look into what panoName is */
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
  

  if (CreateStitchingMasks(fullPathImages,counterImageFiles) != 0) {
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

  /* Above this is multi-layer, below is flattening */

  if (FlattenTIFF(fullPathImages,counterImageFiles) != 0) { 
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

  if (readImage(&resultPanorama, panoFileName) == 0) {
    PrintError("Could not read result image");
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

  if (strcmp(word, "JPEG") == 0 || strcmp(word, "JPG")) {
    ReplaceExt(panoFileName->name, ".JPG");
//int writeJPEG( Image *im, fullPath *sfile, 	int quality, int progressive )
    return writeJPEG(&resultPanorama, panoFileName, jpegQuality, jpegProgressive);
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


#ifdef THIS_IS_ALSO_IN_LIBPANO

This function is already implemented in libpano! I wonder why it was reimplemented here and
if there are other functions that are repeated. It is nice to see we got it right
 
//////////////////////////////////////////////////////////////////////
/*
 * This routine seems to replace the filename in fullPathName with the
 *  filename in newBaseFileName
 */
void InsertFileName(char *fullPathName, char* newBaseFileName)
{
  char *temp;
  if ((temp = strrchr(fullPathName, '/')) != 0) {
    temp++;
   } else {
    temp = fullPathName;
  }
  strcpy(temp, newBaseFileName);
}
#endif


// These are dummy functions to stop the linker from complaining. THEY need to be implemented at some time

int  CreatePSD(  fullPath *fullPathImages, int numberImages, fullPath* outputFileName)
{

  Image *ptrImage;
  char *var24;
  int i;
  stBuf stitchInfo; // length 524
  fullPath var92;
  char tempString[128];
  Image image;
  

  if ( numberImages == 0 ) {
    return 0;
  }
  
  if ( quietFlag == 0 ) {
    
    Progress(_initProgress, "Converting TIFF to PSD");
  }

  sprintf(tempString, "%d", 0x64/numberImages);

  if ( quietFlag == 0 ) {

    Progress(_setProgress, tempString);

  }
  

  SetImageDefaults(&image);

  if (  readTIFF(&image, &fullPathImages[0]) != 0 ) {


    PrintError("Could not read TIFF image No 0");
    
    
    if ( quietFlag == 0 ) {
      Progress(_disposeProgress, tempString);
    }

    return -1;

  }

  TwoToOneByte(&image);

  if (writePSDwithLayer(&image, outputFileName) != 0) {
    

    PrintError("Could not write PSD-file");

    if ( quietFlag != 0 ) 
      Progress(_disposeProgress, tempString);
    return -1;
  }

  myfree((void**)image.data);

 
  i = 1;

   var24 = tempString;

   ptrImage = &image;

   for (i = 1; i < numberImages; i++) {

     sprintf(var24, "%d", i * 100/numberImages);


     if ( quietFlag == 0 ) {
     
       if ( Progress(_setProgress,var24) == 0 ) {
	 remove(outputFileName->name);
	 return -1;
       }
     
     }

     if (readTIFF(ptrImage, &fullPathImages[i]) != 0) {
     
       PrintError("Could not read TIFF image No &d", i);
     
     
       if ( quietFlag == 0 ) {
       
	 Progress(_disposeProgress, var24);

       }
       return -1;
     
     }

     TwoToOneByte(ptrImage);

     stitchInfo.seam = 1;
     stitchInfo.feather = 0;

     strcpy(var92.name, outputFileName->name);


     if (makeTempPath(&var92) != 0) {
       PrintError("Could not make Tempfile");
       return -1;

     }

     if (addLayerToFile(ptrImage, outputFileName, &var92, &stitchInfo) != 0) {
       //       remove(outputFileName->name);
       PrintError("Could not write Panorama File");
       return -1;
     }

     remove(outputFileName->name);
   
     rename(var92.name, outputFileName->name);
   
     myfree((void**)image.data);
   }   
   return 0;
}


void ComputeStitchingMask8bits(Image *image)
{

  int column;
  int row;
  unsigned char *ptr;
  unsigned char *pixel;
  uint16  *ptrCounter;
  uint16 count;


  //  fprintf(stderr, "St1\n");

  for (column = 0; column < image->width; column ++) {

    count = 0;

    // Point to the given column in row 0

    ptr = *image->data + column * 4;

    //    fprintf(stderr, "St1.1 Column[%d]\n", column);



    for (row = 0; row < image->height; row ++) {

      //      fprintf(stderr, "St1.2 Column[%d] Row[%d]\n", column, row);

      pixel = row * image->bytesPerLine + ptr;

      if (*pixel == 0) {
	
	count = 0;
	
      } else {

	count ++;
	
      }
      // Use the GB pixel area to keep a count of how many pixels we have seen in 
      // the mask area.

      ptrCounter = (uint16*)(pixel +2);
      *ptrCounter = count;

    } //     for (row = 0; row < image->heght; row ++) {


    //    fprintf(stderr, "St1.3 Column[%d]\n", column);

    count = 0;
    row = image->height;

    while (--row  >= 0) {

      //      fprintf(stderr, "St1.4 Column[%d] Row[%d]\n", column, row);

      pixel = ptr + row + image->bytesPerLine;

      if ( *pixel == 0 ) {

	count = 0;

      } else {

	count ++;

      }

      ptrCounter = (uint16*)(pixel +2);

      if ( *ptrCounter < count) {
      
	count = *ptrCounter;
      
      
      } else {
      
	*ptrCounter  = count;
      
      }
    
    
    } //while
    
    //    fprintf(stderr, "St1.5 Column[%d]\n", column);

    
  } //

  ///////////// row by row

  //  fprintf(stderr, "St2\n");



  for (row = 0; row < image->height; row ++) {

    count = image->width;
    
    ptr = row * image->bytesPerLine + *(image->data);
    
    
    for (column = 0; column < image->width; column ++ ) {
      
      
      pixel = ptr + 4 * column;
      
      if ( *pixel == 0 ) {
	
	count = 0;
	
      } else {
	
	count ++;
	
      }
      
      ptrCounter = (uint16*)(pixel +2);
      
      if (*ptrCounter < count) {
	
	count = *ptrCounter;
	
      } else {
	
	*ptrCounter = count;
	
      } //

    } // for column

    //-----------------------------;;


    for (column = 0; column < image->width; column ++ ) {


      pixel = ptr + column * 4;

      if ( *pixel == 0 ) {
	count = 0;
      } else {
	count ++;
      }

      ptrCounter =  (uint16*)(pixel +2);

      if (*ptrCounter < count) {
	count = *ptrCounter;
      } else {
	*ptrCounter = count;
      }
    } // for

    //---------------------------------------;

    //  fprintf(stderr, "St3\n");

    count = image->width;
    column = image->width;

    while (--column >= 0) {

      pixel = ptr + column * 4;

      if (0 == *pixel ) {
	count = 0;
      } else {
	count ++;
      }

      ptrCounter =  (uint16*)(pixel +2);

      if (*ptrCounter < count) {
	count = *ptrCounter;
      } else {
	*ptrCounter = count;
      }
    } //    while (--column >= 0) {

    //--------------------------------;
    column = image->width;

    //  fprintf(stderr, "St4\n");

    while (--column >= 0) {

      pixel = ptr + 4 * column;

      if ( *pixel == 0 ) {
	count = 0;
      } else {
	count ++;
      }

      ptrCounter =  (uint16*)(pixel +2);

      if (*ptrCounter < count) {
	
	count = *ptrCounter ;
      
      } else {
	*ptrCounter = count;
      }
    } // end of while
    
  } // end of for row


}

void ComputeStitchingMask16bits(Image *image)
{
  fprintf(stderr, "Masking not supported for this image type (%d bitsPerPixel)\n", (int)image->bitsPerPixel);
  exit(1);
}


void ComputeStitchingMask(Image *image)
{
  if ( image->bitsPerPixel == 32 ) {
    ComputeStitchingMask8bits(image);
    return;
  } else  if ( image->bitsPerPixel == 64 ) {
    ComputeStitchingMask16bits(image);
    return;
  }
  fprintf(stderr, "Masking not supported for this image type (%d bitsPerPixel)\n", (int)image->bitsPerPixel);
  exit(1);
}

void SetBestAlphaChannel16bits(unsigned char *imagesBuffer, int numberImages, uint32 imageWidth, int bytesPerLine)
{
  assert(0); // it should not be here... yet
}
void SetBestAlphaChannel8bits(unsigned char *imagesBuffer, int numberImages, uint32 imageWidth, int bytesPerLine)
{

  unsigned char *pixel;
  uint16 *ptrCount;
  uint16 best;
  uint16 maskValue;
  int column;
  int j;

  

  for  (column=0, pixel = imagesBuffer;  column < imageWidth; column++, pixel +=4) {

    best = 0;

    ptrCount = (uint16*)(pixel + 2);

    maskValue = *ptrCount;

    best = 1;

    // find the image with the highest value

    for (j = 1; j < numberImages; j ++) {

      ptrCount = (uint16*)(pixel + bytesPerLine * j  + 2);

      if (*ptrCount > maskValue) {

	best = j;
	maskValue = *ptrCount;
      
      }
    } // for j

    if ( maskValue != 0 ) {

      // set the mask of the ones above, but not below... interesting...

      for (j = best+1;  j < numberImages; j ++ ) {
	unsigned char *pixel2;
  
	pixel2 = pixel +  bytesPerLine * j;

	if (0 != *pixel2) {
	  *pixel2 = 1;
	} 
      }
    }
  } // for i

}



void CalculateAlphaChannel(unsigned char *imagesBuffer, int numberImages, uint32 imageWidth, int bytesPerLine, int bitsPerPixel)
{

  if (bitsPerPixel == 32) {
    SetBestAlphaChannel8bits(imagesBuffer, numberImages, imageWidth, bytesPerLine);
  } else if (bitsPerPixel == 64) {
    SetBestAlphaChannel16bits(imagesBuffer, numberImages, imageWidth, bytesPerLine);
  } else {
    fprintf(stderr, "CalculateAlphaChannel not supported for this image type (%d bitsPerPixel)\n", bitsPerPixel);
    exit(1);
  }
}

void ApplyFeather8bits(Image *image, int featherSize)
{

  fprintf(stderr, "\nFeathering 8 bits not implemented yet\n");
  
}

void ApplyFeather16bits(Image *image, int featherSize)
{

  fprintf(stderr, "\nFeathering 16 bits not implemented yet\n");
  
}


void ApplyFeather(Image *image, int featherSize)
{

  //  fprintf(stderr, "To apply feather %d\n", featherSize);
  if (image->bitsPerPixel == 32) {
    ApplyFeather8bits(image, featherSize);
  } else if (image->bitsPerPixel == 64) {
    ApplyFeather16bits(image, featherSize);
  } else {
    fprintf(stderr, "Apply feather not supported for this image type (%d bitsPerPixel)\n", (int)image->bitsPerPixel);
    exit(1);
  }


}

void TiffSetImageParameters(TIFF *tiffFile, uint32 imageLength, uint32 imageWidth, uint16 bitsPerSample,  uint16 samplesPerPixel, uint32 rowsPerStrip)
{
  assert(tiffFile != NULL);

  assert(PHOTOMETRIC_RGB == 2);
  assert(TIFFTAG_PHOTOMETRIC == 0x106);
  assert(TIFFTAG_PLANARCONFIG == 0x11c);
  assert(PLANARCONFIG_CONTIG == 1);
  assert(0x8005 == COMPRESSION_PACKBITS);
  assert(TIFFTAG_ORIENTATION == 0x112);
  assert(TIFFTAG_ROWSPERSTRIP == 0x116);
  
  
  TIFFSetField(tiffFile, TIFFTAG_IMAGEWIDTH, imageWidth);
  TIFFSetField(tiffFile, TIFFTAG_IMAGELENGTH, imageLength);
  TIFFSetField(tiffFile, TIFFTAG_BITSPERSAMPLE, bitsPerSample);
  TIFFSetField(tiffFile, TIFFTAG_PHOTOMETRIC, PHOTOMETRIC_RGB);
  TIFFSetField(tiffFile,  TIFFTAG_PLANARCONFIG, PLANARCONFIG_CONTIG);
  TIFFSetField(tiffFile, TIFFTAG_SAMPLESPERPIXEL, samplesPerPixel);
  TIFFSetField(tiffFile, TIFFTAG_COMPRESSION, COMPRESSION_PACKBITS);
  TIFFSetField(tiffFile, TIFFTAG_ORIENTATION, ORIENTATION_TOPLEFT);
  TIFFSetField(tiffFile,  TIFFTAG_ROWSPERSTRIP, imageLength);

}





int CreateStitchingMasks(  fullPath *fullPathImages, int numberImages)
{
  
  unsigned char *ptrOtherImageData;
  unsigned char *ptrBuffer;
  unsigned char *imagesBuffer;
  fullPath *var08;
  fullPath *var04;
  int index;
  int bytesPerLine;  
  int bitsPerPixel;
  //  int bytesPerPixel;

  TIFF **tiffOutputImages;
  TIFF **tiffImages;
  TIFF* tiffFile;
  TIFF *outputTIFFFile;
  uint16 samplesPerPixel;

  uint16 bitsPerSample;
  uint32 imageLength;
  uint32 imageWidth;
  Image image ;
  char tempString[512];
  int j;
  int i;
  int row;


  if ( numberImages == 0 ) {
    return 0;
  }

  SetImageDefaults(&image);

  var04 = calloc(numberImages, sizeof(fullPath));

  if ( var04 == NULL ) {
    PrintError("Not enough memory");
    return -1;
  } 

  if ( quietFlag == 0 ) Progress(0, "Preparing Stitching Masks");

  // for each image, create merging mask and save to temporal file
  for (index = 0; index < numberImages; index ++ ) {
    
    sprintf(tempString, "%d", index * 100/numberImages);
    
    if ( quietFlag == 0 ) {
      if (Progress(_setProgress, tempString) ==0 ) {
	for (j = 0; j < index; j++) {
	  remove(var04[j].name);
	} //for 
	return -1;
      }
    }
    
    if (readTIFF(&image, &fullPathImages[index]) != 0) {
      PrintError("Could not read TIFF-file");
      return -1;
    }

    ComputeStitchingMask(&image);

    strcpy(var04[index].name, fullPathImages[0].name); // is this really zero????

    if (makeTempPath(&var04[index]) != 0) {
      PrintError("Could not make Tempfile");
      return -1;
    }

    writeTIFF(&image, &var04[index]);
    
    //    fprintf(stderr, "Written to file %s\n", var04[index].name);
    
    myfree((void**)image.data);

  } // for (index...

  fprintf(stderr, "Masks have been computed\n");
  // Get TIFF information
  
  if (GetFullPath(&fullPathImages[0], tempString) != 0) {
    PrintError("Could not get filename");
    return -1;
  }
  //  fprintf(stderr, "Getting image defaults from %s\n", tempString);
  
  // Process first TIFF to find common values for images

  if ((tiffFile = TIFFOpen(tempString, "r")) == 0) {
    PrintError("Could not open TIFF-file");
    return -1;
  }
  
  TIFFGetField(tiffFile, TIFFTAG_IMAGEWIDTH, &imageWidth);
  TIFFGetField(tiffFile, TIFFTAG_IMAGELENGTH, &imageLength);
  TIFFGetField(tiffFile, TIFFTAG_BITSPERSAMPLE, &bitsPerSample);
  TIFFGetField(tiffFile, TIFFTAG_SAMPLESPERPIXEL, &samplesPerPixel);
  
  bitsPerPixel = samplesPerPixel * bitsPerSample;
  
  bytesPerLine = TIFFScanlineSize(tiffFile);
  
  TIFFClose(tiffFile);
  
  // Allocate arrays of TIFF* for the input and output
  // images. process is one row at a time, with all images
  // processed at the same time
  tiffImages = (TIFF**)calloc(numberImages, sizeof(TIFF*));
  tiffOutputImages = (TIFF**)calloc(numberImages, sizeof(TIFF*));
  
  // Allocate space for just another set of temp filenames
  var08 = calloc(numberImages, sizeof(fullPath));
  
  if (var08 == NULL ) {
    PrintError("Not enough memory");
    return -1;
  }
  
  imagesBuffer = calloc(numberImages, bytesPerLine);
  
  if (imagesBuffer == NULL) {
    PrintError("Not enough memory");
    return(-1);
  }

  /// Alpha Channel calculation

  fprintf(stderr, "Start alpha channel calculation\n");
 
  if ( quietFlag == 0 ) {
    Progress(_initProgress, "Calculating Alpha Channel");
  }
    
  for (index = 0; index < numberImages; index++) {

    uint32 imageWidth;
      
    if (GetFullPath(&var04[index], tempString) != 0) {
      PrintError("Could not get filename");
      return -1;
    }

    if ((tiffImages[index] = TIFFOpen(tempString, "r")) == 0) {
      PrintError("Could not open TIFF-file");
      return -1;
    }
    /// DEBUG
    TIFFGetField(tiffImages[index], TIFFTAG_IMAGEWIDTH, &imageWidth);

    strcpy(var08[index].name, fullPathImages[0].name);

    if (makeTempPath(&var08[index]) != 0) {
      PrintError("Could not make Tempfile");
      return -1;
    }

    if (GetFullPath(&var08[index], tempString) != 0) {
      PrintError("Could not get filename");
      return -1;
    }

    tiffOutputImages[index] = TIFFOpen(tempString, "w");

   
    if ( tiffOutputImages[index] == NULL ) {
      PrintError("Could not create TIFF-file");
      return -1;
    }

    TiffSetImageParameters(tiffOutputImages[index], imageLength, imageWidth, bitsPerSample, samplesPerPixel, imageLength);

  }// for index...


  fprintf(stderr, "Files have been created, process each row\n");
 
  for (i = 0; i < imageLength; i++) {

    if ( i == (i / 20) * 20 ) {

      sprintf(tempString, "%lu", i * 100/imageLength);

      if ( quietFlag == 0 ) {

	if (Progress(_setProgress, tempString) == 0) {

	  for (index = 0 ; index < numberImages; index ++) {

	    TIFFClose(tiffImages[index]);

	    TIFFClose(tiffOutputImages[index]);

	    remove(var04[index].name);

	    remove(var08[index].name);
	  }
	  return -1;
	}
      } // if (imagelength...
    }

    //    fprintf(stderr, "To process row [%d] bytesperline %d\n", i, bytesPerLine);

    for (ptrBuffer = imagesBuffer, index = 0; index < numberImages; index ++, ptrBuffer += bytesPerLine) {

      TIFFReadScanline(tiffImages[index], ptrBuffer, i, 0);

      RGBAtoARGB(ptrBuffer, imageWidth, bitsPerPixel);
     
    }

    CalculateAlphaChannel(imagesBuffer, numberImages, imageWidth, bytesPerLine, bitsPerPixel);

    for (index = 0 , ptrBuffer = imagesBuffer; index < numberImages; index ++, ptrBuffer+= bytesPerLine) {

      ARGBtoRGBA(ptrBuffer, imageWidth, bitsPerPixel);

      TIFFWriteScanline(tiffOutputImages[index], ptrBuffer, i, 0);
     
    } //for
   
  } //for i

  free(imagesBuffer);

  for (index = 0; index < numberImages; index ++) {
    TIFFClose(tiffImages[index]);
    TIFFClose(tiffOutputImages[index]);
    remove(var04[index].name);
  } // for index.

  // At this point we have 2 files per original image: one with the
  // pano file, and the other one
  // with the mask


  if ( quietFlag == 0 ) {
    Progress(_setProgress, "Applying Feather Tool");
  }

  fprintf(stderr, "Feathering\n");

  for (index = 0; index < numberImages; index ++) {

    sprintf(tempString, "%d", 100 * index/ numberImages);

    if ( quietFlag == 0 ) {

      if (Progress(_setProgress, tempString) == 0) {

	for (j = index; j <= numberImages; j ++) {
	  remove(var08[j].name);
	} // for 

	for (j = 0; j < index; j ++) {
	  remove(var04[j].name);
	} // for 
	return -1;
      }

    }

    if (readTIFF(&image, &var08[index]) != 0) {
      PrintError("Could not open TIFF-file");
      return -1;
    }

    // Apply feather

    ApplyFeather(&image, global5640.feather);

    writeTIFF(&image, &var04[index]);

    myfree((void**)image.data);

    remove(var08[index].name);

  } //for index

  // AT this point the feather has been applied to the mask
  // We still have 2 files per image

  imagesBuffer = calloc(bytesPerLine * 2, 1);
 
  if (imagesBuffer == NULL) {
    PrintError("Not enough memory");
    return -1;
  }

  //  fprintf(stderr, "Inserting alpha channel\n");

  if ( quietFlag == 0 ) {
    Progress(_initProgress, "Inserting Alpha Channel");
  }

  for (index = 0; index < numberImages; index ++) { //

    sprintf(tempString, "%d", index * 100/ numberImages);
    if ( quietFlag == 0 ) {
      if (Progress(_setProgress, tempString) == 0) {
	return -1;
      }
    }

    if (GetFullPath(&var08[index], tempString) != 0) {
      PrintError("Could not get filename");
    }
  
    outputTIFFFile = TIFFOpen(tempString, "w");

    if ( outputTIFFFile == 0 ) {
      PrintError("Could not create TIFF-file");
      return -1;
    }

    if (GetFullPath(&fullPathImages[index], tempString) != 0) {
      PrintError("Could not get filename");
      return -1;
    }

    tiffFile = TIFFOpen(tempString, "r");

    if ( tiffFile == NULL ) {
      PrintError("Could not open TIFF-file");
      return -1;
    }

    if (GetFullPath(&var04[index], tempString) != 0) {

      PrintError("Could not make Tempfile");
      return -1;
    }

    tiffImages[index] = TIFFOpen(tempString, "r");

    if (tiffImages[index] == NULL ) {
      PrintError("Could not open TIFF-file");
      return -1;
    }

    TiffSetImageParameters(outputTIFFFile, imageLength, imageWidth, bitsPerSample, samplesPerPixel, imageLength);

    ptrOtherImageData = imagesBuffer + bytesPerLine;

    for (row = 0; row < imageLength; row ++) {

      unsigned char *source;
      unsigned char *destination;


      TIFFReadScanline(tiffFile, imagesBuffer, row, 0);

      TIFFReadScanline(tiffImages[index], ptrOtherImageData, row, 0);

      if ( bitsPerPixel == 32 ) {

	//	imagesBuffer + 3;

	destination = imagesBuffer + 3;
	source = ptrOtherImageData + 3;

	for (j = 0; j < imageWidth; j ++ ) {
	  *destination = *source;
	  destination +=4;
	  source +=4;
	} 

      } else {

	destination = imagesBuffer + 6;
	source = imagesBuffer + bytesPerLine + 6;

	for (j = 0; j < imageWidth ; j ++ ) {
	  *destination = *source;

	  source += 8;
	  destination += 8;

	} // for j
       
      } // end of if

      TIFFWriteScanline(outputTIFFFile, imagesBuffer, row, 0);
     
    } // for 


    TIFFClose(tiffImages[index]);
    remove(var04[index].name);

   
    TIFFClose(tiffFile);
    remove(fullPathImages[index].name);

    TIFFClose(outputTIFFFile);
    rename(var08[index].name, fullPathImages[index].name);

  } // for index

  free(imagesBuffer);
  free(var04);
  free(var08);
  free(tiffImages);
  free(tiffOutputImages);

  return 0;

}



int Create_LP_ivr(Image *image, fullPath* fullPathImage)
{
  fprintf(stderr,"Create_LP_ivr this function is not implemented yet\n");
  exit(1);
}

int FlattenTIFF(  fullPath *fullPathImages, int numberImages)
{
  fprintf(stderr,"FlattenTIFF this function is not implemented yet\n");
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
  /* This function seems to 'clear' the area outside left,top; right,bottom
     in the picture with zeros */

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

  /* it seems to work only for these types of images*/
  assert(image->bitsPerPixel == 0x20 ||
         image->bitsPerPixel == 0x40);
  
  top = image->selection.top;
  bottom = image->selection.bottom;
  left = image->selection.left;
  right = image->selection.right;
  
  if ( bottom == 0 ) {
    bottom = image->height;
  }
  
  if ( right == 0 ) {
    right = image->width;
  }
  


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
    
  /* Clear the area at above the image */

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
      
  /* Clear area below the picture */
  
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

