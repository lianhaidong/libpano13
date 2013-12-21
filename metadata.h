/*
 *  metadata.h
 *
 *  Many of the routines are based on the program PTStitcher by Helmut
 *  Dersch.
 * 
 *  Copyright Helmut Dersch and Daniel M. German
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
 *  You should have received a copy of the GNU General Public License
 *  along with this software; see the file COPYING.  If not, a copy
 *  can be downloaded from http://www.gnu.org/licenses/gpl.html, or
 *  obtained by writing to the Free Software Foundation, Inc.,
 *  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 *
 *  Author: Daniel M German dmgerman at uvic doooot ca
 * 
 */

#ifndef __METADATA_H__

#define __METADATA_H__

int panoMetadataUpdateFromImage(Image *im);
void panoUnCropMetadata(pano_ImageMetadata * metadata);
void panoMetadataCropSizeUpdate(pano_ImageMetadata * metadata, pano_CropInfo *cropInfo);
int panoImageIsCropped(Image *image);
int panoImageBytesPerPixel(Image *image);
int panoImageBitsPerSample(Image *image);
int panoImageBytesPerSample(Image *image);
int panoImageBytesPerLine(Image *image);
int panoImageFullWidth(Image *image);
int panoImageFullHeight(Image *image);
int panoImageOffsetX(Image *image);
int panoImageOffsetY(Image *image);
int panoImageWidth(Image *image);
int panoImageHeight(Image *image);

unsigned char *panoImageData(Image *image);

#endif
