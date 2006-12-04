/*
 *  PTfeather.c
 *
 *  Many of the routines are based on the program PTStitcher by Helmut
 *  Dersch.
 * 
 *  Copyright Helmut Dersch and Daniel M. German
 *  
 *  Nov 2006
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


#include "filter.h"

#include "pttiff.h"
#include "file.h"
#include "PTcommon.h"
#include "ptstitch.h" 
#include "metadata.h"

#include <assert.h>
#include <float.h>

static void panoFeatherSnowPixel8Bit(unsigned char *pixel, int featherSize, int contribution)
{
    int newPixel = 0;
    int randomComponent;

    contribution = contribution / 0x100;

    // TODO: check this expression. It needs to be evaluated in the order specified by the
    // parenthesis
    
    // THIS IS LIKELY TO OVERFLOW a few times... Make sure we do the arithmetic in long long to avoid overflows
    randomComponent = ((rand() - RAND_MAX/2) * (0xfeLL /featherSize)) / RAND_MAX;
    
    newPixel = *pixel  -  contribution  + randomComponent;

    if ( newPixel <= 0 ) 
	// we can't make it zero. We rely on value 1 to know where the actual edge of an image is
	newPixel = 1;
    else if (newPixel > 0xff)
	*pixel = 0xff;
    else
	*pixel = newPixel;
}

static void panoFeatherSnowPixel16Bit(unsigned char *pixel, int featherSize, int contribution)
{
    int newPixel = 0;
    int randomComponent;

    uint16_t *pixel16;
    
    pixel16  = (uint16_t *) pixel;

    // TODO: check this expression. It needs to be evaluated in the order specified by the
    // parenthesis
    
    // THIS IS LIKELY TO OVERFLOW a few times... Make sure we do the arithmetic in long long to avoid overflows
    randomComponent = ((rand() - RAND_MAX/2) * (0xfe00LL /featherSize)) / RAND_MAX;
    
    newPixel = *pixel16  -  contribution  + randomComponent;

    if ( newPixel <= 0 ) 
	// we can't make it zero. We rely on value 1 to know where the actual edge of an image is
	newPixel = 1;
    else if (newPixel > 0xffff)
	*pixel16 = 0xffff;
    else
	*pixel16 = newPixel;
}

static void panoFeatherSnowPixel(unsigned char *pixel, int featherSize, int contribution, int bitsPerSample)
{
    if (bitsPerSample == 8) 
	panoFeatherSnowPixel8Bit(pixel, featherSize, contribution);
    else if (bitsPerSample == 16) 
	panoFeatherSnowPixel16Bit(pixel, featherSize, contribution);
    else 
	assert(0);
}



static void panoFeatherSnowingHorizontalLeft(int edi, int ratio, int column, int featherSize, unsigned char *ptrData, Image *image)
{
    int index;
    int currentColumn;

    unsigned char *ptrPixel;
    int bytesPerPixel = panoImageBytesPerPixel(image);
    int bitsPerSample = panoImageBitsPerSample(image);

    for (currentColumn = column +  edi/2 + 1, index=1;  currentColumn >=  column - edi/2; currentColumn--, index++ ) {

        // only operate within the image
        // and IF the mask is not zero
        // We do not want to "feather" outside the boundary
        ptrPixel = ptrData + currentColumn * bytesPerPixel;

        if (currentColumn < 0 || currentColumn >= panoImageWidth(image) || 
            *ptrPixel == 0) {
            continue;
        }

	panoFeatherSnowPixel(ptrPixel, featherSize, index * ratio, bitsPerSample);

    } ///for (currentColumn = column - edi/2;      currentColumn <= column; currentColumn++, index++ ) {
                                

}

static void panoFeatherSnowingVerticalTop(int edi, int ratio, int row, int featherSize, unsigned char *ptrData, Image *image)
{
    int index;
    int currentRow;

    unsigned char *ptrPixel;

    int bytesPerLine = panoImageBytesPerLine(image);
    int bitsPerSample = panoImageBitsPerSample(image);

    for (currentRow = row +  edi/2 + 1, index=1;  currentRow >=  row - edi/2; currentRow--, index++ ) {

        // only operate within the image
        // and IF the mask is not zero
        // We do not want to "feather" outside the boundary
        ptrPixel = ptrData + currentRow * bytesPerLine;

        if (currentRow < 0 || currentRow >= panoImageHeight(image) || 
            *ptrPixel == 0) {
            continue;
        }

	panoFeatherSnowPixel(ptrPixel, featherSize, index * ratio, bitsPerSample);

    } ///for (currentRow = row - edi/2;      currentRow <= row; currentRow++, index++ ) {
                                

}


static void panoFeatherSnowingVerticalBottom(int edi, int ratio, int row, int featherSize, unsigned char *ptrData, Image *image)
{
    int index;
    int currentRow;

    unsigned char *ptrPixel;

    int bytesPerLine = panoImageBytesPerLine(image);
    int bitsPerSample = panoImageBitsPerSample(image);


    for (currentRow = row -  edi/2, index=1;  currentRow <=  row + edi/2; currentRow++, index++ ) {

        // only operate within the image
        // and IF the mask is not zero
        // We do not want to "feather" outside the boundary
        ptrPixel = ptrData + currentRow * bytesPerLine;

        if (currentRow < 0 || currentRow >= panoImageHeight(image) || 
            *ptrPixel == 0) {
            continue;
        }

	panoFeatherSnowPixel(ptrPixel, featherSize, index * ratio, bitsPerSample);

    } ///for (currentRow = row - edi/2;      currentRow <= row; currentRow++, index++ ) {
                                

}




static void panoFeatherSnowingHorizontalRight(int edi, int ratio, int column, int featherSize, unsigned char *ptrData, Image *image)
{
    int index;
    int currentColumn;
    int bitsPerSample = panoImageBitsPerSample(image);
    unsigned char *ptrPixel;

    int bytesPerPixel = panoImageBytesPerPixel(image);

    for (currentColumn = column -  edi/2, index=1;  currentColumn <= column + edi/2; currentColumn++, index++ ) {

        // only operate within the image
        // and IF the mask is not zero
        // We do not want to "feather" outside the boundary
        ptrPixel = ptrData + currentColumn * bytesPerPixel;

        if (currentColumn < 0 || currentColumn >= panoImageWidth(image) || 
            *ptrPixel == 0) {
            continue;
        }

	panoFeatherSnowPixel(ptrPixel, featherSize, index * ratio, bitsPerSample);

    } ///for (currentColumn = column - edi/2;      currentColumn <= column; currentColumn++, index++ ) {

}

static void panoFeatherMaskReplace(Image* image, unsigned int from, unsigned int to)
{

    // Replace a given value in the first channel with the desired value

    int row;
    int column;
    uint16_t *pixel16;

    int bitsPerSample = panoImageBitsPerSample(image);

    int bytesPerPixel = panoImageBytesPerPixel(image);

    int bytesPerLine = panoImageBytesPerLine(image);

    int imageHeight = panoImageHeight(image);

    int imageWidth = panoImageWidth(image);

    unsigned char *pixel = panoImageData(image);


    for (row = 0; row < imageHeight; row ++) {

	pixel = panoImageData(image) + row * bytesPerLine;

        for (column = 0; column < imageWidth; column ++, pixel += bytesPerPixel) {
	    if (bitsPerSample == 8) {
		if ( *pixel == from ) {
		    *pixel = to;
		}
	    } 
	    else if (bitsPerSample == 16) {
		pixel16  = (uint16_t *) pixel;
		if (*pixel16 == from) {
		    *pixel16 = to;
		}
	    } else {
		assert(0);
	    }
        } // for column

    } // for row
    
}

static void panoFeatherImage(Image * image, int featherSize)
{

    int ratio;
    int difference;
    unsigned char *pixelPtr;
    unsigned char *ptrData;
    int column;
    int row;
    int gradient;

    int bytesPerPixel;
    int bytesPerLine;
    int imageWidth;
    int imageHeight;
    int imageIsCropped;
    int imageLeftOffset;
    int imageTopOffset;
    int imageFullWidth;
    int imageFullHeight;
    int bitsPerSample;

    unsigned char *imageData;

    if (featherSize == 0) 
	return;


    // Use local variables so we don't have to make function calls for each 
    // iteration
    bitsPerSample = panoImageBitsPerSample(image);
    bytesPerPixel = panoImageBytesPerPixel(image);
    bytesPerLine  = panoImageBytesPerLine(image);
    imageHeight = panoImageHeight(image);
    imageWidth = panoImageWidth(image);
    imageIsCropped = panoImageIsCropped(image);
    imageData = panoImageData(image);
    imageFullWidth = panoImageFullWidth(image);
    imageFullHeight = panoImageFullHeight(image);

    imageLeftOffset  = panoImageOffsetX(image);
    imageTopOffset = panoImageOffsetY(image);

    // This is sort of a hack. We replace 0's in the mask with 1's 
    // we have to "undo" it at the end

    panoFeatherMaskReplace(image, 0, 1);

    ratio = 0xfe00 / featherSize;

    // Horizontal first

    assert(bitsPerSample == 8 || 
	   bitsPerSample == 16);
    
    ptrData = imageData;
    
    for ( row = 0; row < imageHeight; row++, ptrData += bytesPerLine) {
	int widthToProcess;

	column = 0;
	
	pixelPtr = ptrData;
	
	// The following code deals with images that are cropped. We should feather edges only 
	// if they are not the absolute edge of an image. 

	// by default we start in column zero 
	column = 0;
	widthToProcess = imageWidth;
	if (imageIsCropped) {
	    // we need to deal with edges that are not "real" edges (as in the uncropped image

	    if ( imageLeftOffset > 0) {
		// we have a mask to the left... so we start in column "-1"
		column = -1;
	    }

	    if (imageLeftOffset + widthToProcess < imageFullWidth) {
		// then "add" one pixel to the right */
		widthToProcess ++;
	    }
	}


	for (/*empty, see initialization above */; column < widthToProcess -1; 
						 column ++, pixelPtr+=bytesPerPixel) {
	    
	    // Values of mask in this pixel and next
	    int thisPixel; 
	    int nextPixel;


	    if (column < 0) {
		// this is the imaginary pixel to the left of the edge that should be feathered
		thisPixel = 1;
	    } else  {
		thisPixel = panoStitchPixelChannelGet(pixelPtr, bytesPerPixel, 0);
	    }

	    if (column >= imageWidth -1) {
		// this is the imaginary pixel to the right of the edge that should be feathered
		nextPixel = 1;
	    } else {
		nextPixel = panoStitchPixelChannelGet(pixelPtr + bytesPerPixel, bytesPerPixel, 0);
	    }

	    if (thisPixel != 0  && nextPixel != 0) {
		
		difference = thisPixel - nextPixel;
		
		// This operation needs to be done here, otherwise 0x100/ratio will underflow
		if (bitsPerSample == 8) {
		    gradient = (abs(difference) * 0x100LL) / ratio;
		} 
		else if (bitsPerSample == 16) {
		    gradient = abs(difference) / ratio;
		} else 
		    assert(0);
		
		if ( gradient > 1 ) { //
		    
		    // if difference is positive, which means we are moving from a darker are to a softer area
		    if ( difference > 0 ) {
			
			panoFeatherSnowingHorizontalRight(gradient, ratio, column, featherSize, ptrData, image);
                        
		    } else if ( difference < 0 ) {
			panoFeatherSnowingHorizontalLeft(gradient, ratio, column, featherSize, ptrData, image);
                        
		    } else { // difference == 0
			; // do nothing in this case
		    }
		    
		} // 
                
	    } // 
            
	} // for column...
	
    } // for row
    
    // We need to do the same in the orthogonal direction
    // Sometimes I wished I had  iterators over an image...


    ptrData = imageData;
    
    for (column = 0; column < image->width; column ++, ptrData+=bytesPerPixel) {
	int heightToProcess;

	// The following code deals with images that are cropped. We should feather edges only 
	// if they are not the absolute edge of an image. 

	// by default we start in column zero 
	row = 0;
	heightToProcess = imageHeight;

	if (imageIsCropped) {
	    // we need to deal with edges that are not "real" edges (as in the uncropped image
	    int imageTopOffset;

	    imageTopOffset  = panoImageOffsetY(image);

	    if ( imageTopOffset > 0) {
		// we have a mask to the left... so we start in column "-1"
		row = -1;
	    }

	    if (imageTopOffset + heightToProcess < imageFullHeight) {
		// then "add" one pixel to the right */
		heightToProcess ++;
	    }
	}

	pixelPtr = ptrData;
	for (/*empty, see initialization above */; row < heightToProcess - 1; 
						 row++, pixelPtr += bytesPerLine) {
	    int thisPixel;
	    int nextPixel;

	    // get pixel in current row
	    // with pixel in the next row
	    
	    if (row < 0) {
		// this is the imaginary pixel to the left of the edge that should be feathered
		thisPixel = 1;
	    } else  {
		thisPixel = panoStitchPixelChannelGet(pixelPtr, bytesPerPixel, 0);
	    }

	    if (row >= imageHeight -1) {
		// this is the imaginary pixel to the right of the edge that should be feathered
		nextPixel = 1;
	    } else {
		nextPixel = panoStitchPixelChannelGet(pixelPtr + bytesPerLine, bytesPerPixel, 0);
	    }


	    if (thisPixel != 0  && nextPixel != 0) {

		difference = thisPixel - nextPixel;
		
		// This operation needs to be done here, otherwise 0x100/ratio will underflow
		if (bitsPerSample == 8) {
		    gradient = (abs(difference) * 0x100LL) / ratio;
		} 
		else if (bitsPerSample == 16) {
		    gradient = abs(difference) / ratio;
		} else 
		    assert(0);
		
		if ( gradient > 1 ) { //

		    // if difference is positive, which means we are moving from a darker are to a softer area
		    if ( difference > 0 ) {
			
			panoFeatherSnowingVerticalBottom(gradient, ratio, row, featherSize, ptrData, image);
                        
		    } else if ( difference < 0 ) {
			
			panoFeatherSnowingVerticalTop(gradient, ratio, row, featherSize, ptrData, image);
                        
		    } else { // difference == 0
			; // do nothing in this case
		    }
		    
		} // 
                
	    } // 
            
	} // for column...
	
    } // for row
    

    panoFeatherMaskReplace(image, 1, 0);

}


int panoFeatherFile(fullPath * inputFile, fullPath * outputFile,
		     int featherSize)
{
    Image image;
    if (panoTiffRead(&image, inputFile->name) == 0) {
        PrintError("Could not open TIFF-file [%s]", inputFile->name);
        return 0;
    }

    if (panoImageBitsPerSample(&image) == 8 ||  
	panoImageBitsPerSample(&image) == 16) {
        panoFeatherImage(&image, featherSize);
    }
    else {
        fprintf(stderr,
                "Apply feather not supported for this image type (%d bitsPerPixel)\n",
                (int) image.bitsPerPixel);
        exit(1);
    }

    if (panoTiffWrite(&image, outputFile->name) == 0) {
        PrintError("Could not write TIFF-file [%s]", outputFile->name);
        return 0;
    }

    ///XXXXXXXXXXXXX we need to properly release the memory allocated 
    // including themetatada


    myfree((void **) image.data);

    return 1;

}

