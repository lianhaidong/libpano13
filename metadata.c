/*
 *  metadata.c
 *
 *  Many of the routines are based on the program PTStitcher by Helmut
 *  Dersch.
 * 
 *  Copyright Helmut Dersch,  Daniel M. German
 *  
 *  Dec 2006
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

// update metadata from an image. IT does not update cropped information

#include <stdio.h>
#include "panorama.h"
#include "metadata.h"


int panoMetadataUpdateFromImage(Image *im) 
{
    im->metadata.imageWidth = im->width;
    im->metadata.imageHeight = im->height;
    im->metadata.bytesPerLine = im->bytesPerLine;
    im->metadata.bitsPerSample = im->bitsPerPixel / 4;
    im->metadata.samplesPerPixel = 4;
    im->metadata.bytesPerPixel = im->bitsPerPixel/8;
    im->metadata.bitsPerPixel = im->bitsPerPixel;
    return 1;
}

// Sometimes we need to convert an image from cropped to uncropped. This function 
// takes care of setting the metadata accordingly
void panoUnCropMetadata(pano_ImageMetadata * metadata)
{
    metadata->imageWidth = metadata->cropInfo.fullWidth;
    metadata->imageHeight = metadata->cropInfo.fullHeight;
    metadata->isCropped = 0;
    metadata->bytesPerLine = metadata->imageWidth * metadata->bytesPerPixel;
}

void panoMetadataCropSizeUpdate(pano_ImageMetadata * metadata, pano_CropInfo *cropInfo)
{
    // Update metadata with cropped info.

    metadata->imageWidth = cropInfo->croppedWidth;
    metadata->imageHeight = cropInfo->croppedHeight;
    metadata->bytesPerLine = metadata->imageWidth * metadata->bytesPerPixel;

    // now the crop info in the metadata
   
    metadata->cropInfo.croppedWidth = cropInfo->croppedWidth;
    metadata->cropInfo.croppedHeight = cropInfo->croppedHeight;
    metadata->cropInfo.xOffset += cropInfo->xOffset;
    metadata->cropInfo.yOffset += cropInfo->yOffset;
    
    // The full size remains the same, 
    // The rest of the metadata should be the same
}
