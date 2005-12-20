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
 *
 * 
 */

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#include <tiffio.h>
#include <filter.h>
#include "panorama.h"

#include "ptmender.h"

// Global variables for the program


stBuf global5640;
int   quietFlag;

VRPanoOptions defaultVRPanoOptions;
int jpegQuality;
int jpegProgressive;

//



int main(int argc,char *argv[])
{
  // It does nothing yet

  fprintf(stderr, "This program is work in progress. I does nothing yet\n");
  exit(1);
}


int CreatePanorama(fullPath *ptrImageFileNames[], int counterImageFiles, fullPath *panoFileName, fullPath *scriptFileName)
{

  Image *currentImagePtr;
  aPrefs *prefs;
  int var01;
  int var00;
  int var72;
  int panoProjection;

  int var60;
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


  var72 = 0;
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

    if (readAdjustLine(&tempScriptFile) != 0) {
      PrintError("Could not read Scriptfile");
      goto mainError;
    }

    var72 = prefs->sBuf.colcorrect; // 1, 2, or 3 //

    if (prefs->pano.cP.radial != 0) {

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

#ifdef adfasdf

?? radial_params[0][4] // correction radious

 804a01d:	dd 82 d4 06 00 00    	fldl   0x6d4(%edx)            // loads address into FL
// stores control word in var98?
 804a023:	d9 bd b2 eb ff ff    	fnstcw 0xffffebb2(%ebp)           ;;;;;;;;;;;>>> -5198
// now it 
 804a029:	66 8b 8d b2 eb ff ff 	mov    0xffffebb2(%ebp),%cx           ;;;;;;;;;;;>>> -5198
//3072?
 804a030:	66 81 c9 00 0c       	or     $0xc00,%cx
 804a035:	66 89 8d b0 eb ff ff 	mov    %cx,0xffffebb0(%ebp)           ;;;;;;;;;;;>>> -5200
 804a03c:	d9 ad b0 eb ff ff    	fldcw  0xffffebb0(%ebp)           ;;;;;;;;;;;>>> -5200

 804a042:	db 9d 7c e7 ff ff    	fistpl 0xffffe77c(%ebp)           ;;;;;;;;;;;>>> -6276 var00
 804a048:	d9 ad b2 eb ff ff    	fldcw  0xffffebb2(%ebp)           ;;;;;;;;;;;>>> -5198
 804a04e:	ff 85 7c e7 ff ff    	incl   0xffffe77c(%ebp)           ;;;;;;;;;;;>>> -6276 var00
#endif
											   
    } // begins 804a00e


    if (prefs->pano.cP.horizontal != 0) {
      prefs->pano.cP.horizontal_params[0] ;// 0x75c //[3] 3 colours x horizontal shift value
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
    memcpy(&panoFileName, &fullPathImages[loopCounter], sizeof(fullPath));

    if (makeTempPath(&fullPathImages[loopCounter]) == 0) {

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

    transform.gamma = prefs->gamma;

    // 804a178:	dd 82 0c 11 00 00    	fldl   0x110c(%edx) // loads address into FL
    // 804a17e:	dd 5d f8             	fstpl  0xfffffff8(%ebp)           ;;;;;;;;;;;>>> -8
    //
    // 804a181:	83 c4 fc             	add    $0xfffffffc,%esp           ;;;;;;;;;;;>>> -4
											   
    sprintf(var5196, "Converting Image %d", loopCounter);

 
    if(quietFlag == 0) {

      Progress(_initProgress, var5196 );
      
    }
 
    //int readImage( Image *im, fullPath *sfile ) should return 0
    // currentImagePtr points to image1
 
    if (readImage(currentImagePtr, ptrImageFileNames[loopCounter]) != 0) {
      PrintError("could not read image");
      goto mainError;
    }

    
    if (prefs->im.cP.cutFrame == 0) { // remove frame? 0 - no; 1 - yes
      
      if (CropImage(currentImagePtr, &(prefs->im.selection)) == 0) {
	prefs->im.selection.left  = 0;  //0x674
	prefs->im.selection.right = 0; //0x678
	prefs->im.selection.bottom=0; // 0x670
	prefs->im.selection.top   = 0;   // 0x66c
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

// All the following happens only in the first pass

      if (prefs->pano.width == 0) {

	/*

prefs->im.hfov //0x28
 804a299:	dd 41 28             	fldl   0x28(%ecx)  // loads address into FL
 804a29c:	d9 ee                	fldz   
 804a29e:	da e9                	fucompp 
 804a2a0:	df e0                	fnstsw %ax
 804a2a2:	80 e4 45             	and    $0x45,%ah
 804a2a5:	80 fc 40             	cmp    $0x40,%ah
 804a2a8:	74 65                	je     label804a30f <strcpy@plt+0xd8b>

// This code looks like this, from another program
//gl->pano.width = gl->im[n].width * gl->pano.hfov / gl->im[n].hfov;

// The result is moved to the pano.width

prefs->im.width //0x8
 804a2aa:	db 41 08             	fildl  0x8(%ecx)
prefs->pano.hfov //0x69c
 804a2ad:	dc 89 9c 06 00 00    	fmull  0x69c(%ecx)
prefs->im.hfov //0x28
 804a2b3:	dc 71 28             	fdivl  0x28(%ecx)
 804a2b6:	d9 bd b2 eb ff ff    	fnstcw 0xffffebb2(%ebp)           ;;;;;;;;;;;>>> -5198
 804a2bc:	66 8b 95 b2 eb ff ff 	mov    0xffffebb2(%ebp),%dx           ;;;;;;;;;;;>>> -5198
 804a2c3:	66 81 ca 00 0c       	or     $0xc00,%dx
 804a2c8:	66 89 95 b0 eb ff ff 	mov    %dx,0xffffebb0(%ebp)           ;;;;;;;;;;;>>> -5200
 804a2cf:	d9 ad b0 eb ff ff    	fldcw  0xffffebb0(%ebp)           ;;;;;;;;;;;>>> -5200
 804a2d5:	db 9d ac eb ff ff    	fistpl 0xffffebac(%ebp)           ;;;;;;;;;;;>>> -5204
 804a2db:	8b 8d ac eb ff ff    	mov    0xffffebac(%ebp),%ecx           ;;;;;;;;;;;>>> -5204
 804a2e1:	d9 ad b2 eb ff ff    	fldcw  0xffffebb2(%ebp)           ;;;;;;;;;;;>>> -5198
 804a2e7:	bb 67 66 66 66       	mov    $0x66666667,%ebx
 804a2ec:	89 d8                	mov    %ebx,%eax
 804a2ee:	f7 e9                	imul   %ecx
 804a2f0:	89 d3                	mov    %edx,%ebx
 804a2f2:	89 d8                	mov    %ebx,%eax
 804a2f4:	c1 f8 02             	sar    $0x2,%eax
 804a2f7:	89 ca                	mov    %ecx,%edx
 804a2f9:	c1 fa 1f             	sar    $0x1f,%edx
 804a2fc:	29 d0                	sub    %edx,%eax
 804a2fe:	8d 04 80             	lea    (%eax,%eax,4),%eax
 804a301:	01 c0                	add    %eax,%eax

prefs->pano.width = %eax 

 804a303:	8b 8d 74 e7 ff ff    	mov    prefs,%ecx           ;;;;;;;;;;;>>> -6284
 804a309:	89 81 7c 06 00 00    	mov    %eax,0x67c(%ecx)

label804a30f:
*/
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

      var60 = 500,000 / resultPanorama.bytesPerLine;

 
      if (var60 == 0) { 
	var60 = 1;
      } 

      if (resultPanorama.height < var60) {  
	var60 = resultPanorama.height;
      } 
      if ((
	   resultPanorama.data  = (unsigned char**)mymalloc(var60 * resultPanorama.bytesPerLine )
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
 
    var60  = resultPanorama.selection.bottom;

    CopyPosition(currentImagePtr, &(prefs->im));

    image1.selection.top =  prefs->im.selection.top ;

    image1.selection.bottom = prefs->im.selection.bottom;

    image1.selection.left = prefs->im.selection.left;

    image1.selection.right = prefs->im.selection.right;

    CopyPosition(&resultPanorama, &(prefs->pano));

    Unknown09(currentImagePtr);

    prefs->pano.width = resultPanorama.width;

    prefs->pano.height = resultPanorama.height;

  makePano:

    MakePano(&transform, prefs);
      
    if (transform.success == 0) { //* Error 
      PrintError("Error converting image");
      goto mainError;
    }
    
    Unknown08(&resultPanorama);
    
    for (ebx = 0; ebx< resultPanorama.selection.bottom - resultPanorama.selection.top ; ebx++) {
      TIFFWriteScanline(tiffFile, resultPanorama.data + (resultPanorama.bytesPerLine * ebx), resultPanorama.selection.top + ebx  , 1);


      ++ebx;
      
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
    
    resultPanorama.selection.bottom = resultPanorama.selection.top + var60;

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
  
  if (var00 == 0) {
    Unknown28(fullPathImages,counterImageFiles, var00 -1, 1);
  }
  
  if (var01 == 0) { // goto label804a84f;
    Unknown28(fullPathImages, counterImageFiles, var01 - 1, 2);
  } // 

  Unknown28(fullPathImages,counterImageFiles, (var72 / 4) - 1, 0);

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

  if (strcmp(word, "PSD_nomask") == 0 || strcmp(word, "PSD_m")) { // 
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
if there are other functions that are repeated.
 
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

/*
 * replace the extension in filename with
   extension
*/
void ReplaceExt(char* filename, char *extension)
{
  char *temp;
  temp = strrchr(filename, '.');
  if (temp != NULL) {
    strcpy(temp, extension);
  } else {
    strcat(filename, extension);
  }
  return;
}


// These are dummy functions to stop the linker from complaining. THEY need to be implemented at some time

int  CreatePSD(  fullPath *fullPathImages, int numberImages, fullPath* fullPathImage)
{
  fprintf(stderr,"this function is not implemented yet\n");
  exit(1);
}

int CreateStitchingMasks(fullPath *fullPathImages, int numberImages)
{
  fprintf(stderr,"this function is not implemented yet\n");
  exit(1);
}

int Create_LP_ivr(Image *image, fullPath* fullPathImage)
{
  fprintf(stderr,"this function is not implemented yet\n");
  exit(1);
}

int FlattenTIFF(  fullPath *fullPathImages, int numberImages)
{
  fprintf(stderr,"this function is not implemented yet\n");
  exit(1);
}

int Unknown01(Image *image, fullPath* fullPathImage)
{
  fprintf(stderr,"this function is not implemented yet\n");
  exit(1);
}

int Unknown02(Image *image, fullPath* fullPathImage)
{
  fprintf(stderr,"this function is not implemented yet\n");
  exit(1);
}

int Unknown03(Image *image, fullPath* fullPathImage)
{
  fprintf(stderr,"this function is not implemented yet\n");
  exit(1);
}

int Unknown04(Image *image, fullPath* fullPathImage)
{
  fprintf(stderr,"this function is not implemented yet\n");
  exit(1);
}

int Unknown05(Image *image, fullPath* fullPathImage)
{
  fprintf(stderr,"this function is not implemented yet\n");
  exit(1);
}

int Unknown07(Image *image, fullPath* fullPathImage)
{
  fprintf(stderr,"this function is not implemented yet\n");
  exit(1);
}

void Unknown08(Image *resultPanorama)
{
  fprintf(stderr,"this function is not implemented yet\n");
  exit(1);
}

void Unknown09(Image *currentImagePtr)
{
  fprintf(stderr,"this function is not implemented yet\n");
  exit(1);
}

void Unknown28(  fullPath *fullPathImages, int p1, int p2, int p3)
{
  fprintf(stderr,"this function is not implemented yet\n");
  exit(1);
}
