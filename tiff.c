/* Panorama_Tools       -       Generate, Edit and Convert Panoramic Images
   Copyright (C) 1998,1999 - Helmut Dersch  der@fh-furtwangen.de
   
   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2, or (at your option)
   any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.  */

/*------------------------------------------------------------*/

// Functions to read tiff format


#include <stdio.h>
#include <assert.h>

#include "panorama.h"
#include "filter.h"

#include "tiffio.h"

#include "ZComb.h"


#include "pttiff.h"
#include "metadata.h"


int readplanarTIFF(Image * im, TIFF * tif);
void RGBAtoARGB(UCHAR * buf, int width, int bitsPerPixel);
void ARGBtoRGBA(UCHAR * buf, int width, int bitsPerPixel);
int readtif(Image * im, TIFF * tif);


int readplanarTIFF(Image * im, TIFF * tif)
{
    UCHAR *buf;
    pt_int32 y;
    short SamplesPerPixel;

    TIFFGetField(tif, TIFFTAG_SAMPLESPERPIXEL, &SamplesPerPixel);
    if (SamplesPerPixel > 4)
        return -1;
    if (SamplesPerPixel == 3)
    {
        im->bitsPerPixel = im->bitsPerPixel * 3 / 4;
        im->bytesPerLine = im->bytesPerLine * 3 / 4;
    }

    buf = (UCHAR *) malloc((size_t) TIFFScanlineSize(tif));
    if (buf == NULL)
    {
        PrintError("Not enough memory");
        return -1;
    }

    for (y = 0; y < im->height; y++)
    {
        TIFFReadScanline(tif, buf, (uint32) y, 0);
        RGBAtoARGB(buf, im->width, im->bitsPerPixel);
        memcpy(*(im->data) + y * im->bytesPerLine, buf,
               (size_t) (im->bytesPerLine));
    }
    free(buf);
    ThreeToFourBPP(im);
    return 0;

}



// image is allocated, but not image data

int readtif(Image * im, TIFF * tif)
{
    short BitsPerSample, tPhotoMetric, config;
    pt_int32 w, h;
    unsigned long **hdl_raster;

    if (tif == NULL || im == NULL)
        return -1;

    TIFFGetField(tif, TIFFTAG_IMAGEWIDTH, &w);
    TIFFGetField(tif, TIFFTAG_IMAGELENGTH, &h);
    TIFFGetField(tif, TIFFTAG_BITSPERSAMPLE, &BitsPerSample);
    TIFFGetField(tif, TIFFTAG_PHOTOMETRIC, &tPhotoMetric);
    TIFFGetField(tif, TIFFTAG_PLANARCONFIG, &config);

    SetImageDefaults(im);
    im->width = w;
    im->height = h;
    im->bitsPerPixel = 32 * BitsPerSample / 8;
    im->bytesPerLine = im->width * im->bitsPerPixel / 8;
    im->dataSize = im->bytesPerLine * im->height;


    hdl_raster = (unsigned long **) mymalloc((size_t) im->dataSize);
    if (hdl_raster == NULL)
    {
        PrintError("Not enough memory");
        return -1;
    }

    im->data = (UCHAR **) hdl_raster;

    if (tPhotoMetric == PHOTOMETRIC_RGB && config == PLANARCONFIG_CONTIG)
    {
        return readplanarTIFF(im, tif);
    }

    if (TIFFReadRGBAImage
        (tif, (uint32) w, (uint32) h, (uint32 *) * (im->data), 0))
    {
        // Convert agbr to argb; flip image vertically
        unsigned char *cline, *ct, *cb;
        int h2 = im->height / 2, y;
        // Only do the conversion once
        size_t localBytesPerLine = im->bytesPerLine;

        cline = (unsigned char *) malloc(localBytesPerLine);
        if (cline == NULL)
        {
            PrintError("Not enough memory");
            return -1;
        }

        ct = *im->data;
        cb = *im->data + (im->height - 1) * im->bytesPerLine;

        for (y = 0; y < h2;
             y++, ct += im->bytesPerLine, cb -= im->bytesPerLine)
        {
            RGBAtoARGB(ct, im->width, im->bitsPerPixel);
            RGBAtoARGB(cb, im->width, im->bitsPerPixel);
            memcpy(cline, ct, localBytesPerLine);
            memcpy(ct, cb, localBytesPerLine);
            memcpy(cb, cline, localBytesPerLine);
        }
        if (im->height != 2 * h2)
        {                       // odd number of scanlines
            RGBAtoARGB(*im->data + y * im->bytesPerLine, im->width,
                       im->bitsPerPixel);
        }
        if (cline)
            free(cline);
    }
    else
    {
        PrintError("Could not read tiff-data");
        return -1;
    }
    return 0;
}

/**
 * Populates the CropInfo struct with data about cropping of 
 * the TIFF file specified by filename
 */
void getCropInformationFromTiff(TIFF * tif, CropInfo * c)
{
    float x_position, x_resolution, y_position, y_resolution;

    //these are the actual, physical dimensions of the TIFF file
    TIFFGetField(tif, TIFFTAG_IMAGEWIDTH, &(c->cropped_width));
    TIFFGetField(tif, TIFFTAG_IMAGELENGTH, &(c->cropped_height));

    //If nothing is stored in these tags, then this must be an "uncropped" TIFF 
    //file, in which case, the "full size" width/height is the same as the 
    //"cropped" width and height
    if (TIFFGetField(tif, TIFFTAG_PIXAR_IMAGEFULLWIDTH, &(c->full_width)) ==
        0)
        (c->full_width = c->cropped_width);
    if (TIFFGetField(tif, TIFFTAG_PIXAR_IMAGEFULLLENGTH, &(c->full_height)) ==
        0)
        (c->full_height = c->cropped_height);

    if (TIFFGetField(tif, TIFFTAG_XPOSITION, &x_position) == 0)
        x_position = 0;
    if (TIFFGetField(tif, TIFFTAG_XRESOLUTION, &x_resolution) == 0)
        x_resolution = 0;
    if (TIFFGetField(tif, TIFFTAG_YPOSITION, &y_position) == 0)
        y_position = 0;
    if (TIFFGetField(tif, TIFFTAG_YRESOLUTION, &y_resolution) == 0)
        y_resolution = 0;

    //offset in pixels of "cropped" image from top left corner of 
    //full image (rounded to nearest integer)
    c->x_offset = (uint32) ((x_position * x_resolution) + 0.49);
    c->y_offset = (uint32) ((y_position * y_resolution) + 0.49);

    //printf("%s: %dx%d  @ %d,%d", filename, c->cropped_width, c->cropped_height, c->x_offset, c->y_offset);
}


void setCropInformationInTiff(TIFF * tiffFile, CropInfo * crop_info)
{
    char *errMsg = "Could not set TIFF tag";
    float pixels_per_resolution_unit = 150.0;

    //If crop_info==NULL then this isn't a "cropped TIFF", so don't include 
    //cropped TIFF tags
    if (crop_info == NULL)
        return;

    //The X offset in ResolutionUnits of the left side of the image, with 
    //respect to the left side of the page.
    if (TIFFSetField
        (tiffFile, TIFFTAG_XPOSITION,
         (float) crop_info->x_offset / pixels_per_resolution_unit) == 0)
        dieWithError(errMsg);
    //The Y offset in ResolutionUnits of the top of the image, with 
    //respect to the top of the page.
    if (TIFFSetField
        (tiffFile, TIFFTAG_YPOSITION,
         (float) crop_info->y_offset / pixels_per_resolution_unit) == 0)
        dieWithError(errMsg);

    //The number of pixels per ResolutionUnit in the ImageWidth
    if (TIFFSetField
        (tiffFile, TIFFTAG_XRESOLUTION,
         (float) pixels_per_resolution_unit) == 0)
        dieWithError(errMsg);
    //The number of pixels per ResolutionUnit in the ImageLength (height)
    if (TIFFSetField
        (tiffFile, TIFFTAG_YRESOLUTION,
         (float) pixels_per_resolution_unit) == 0)
        dieWithError(errMsg);

    //The size of the picture represented by an image.  Note: 2 = Inches.  This
    //is required so that the computation of pixel offset using XPOSITION/YPOSITION and
    //XRESOLUTION/YRESOLUTION is valid (See tag description for XPOSITION/YPOSITION).
    if (TIFFSetField(tiffFile, TIFFTAG_RESOLUTIONUNIT, (uint16_t) 2) == 0)
        dieWithError(errMsg);

    // TIFFTAG_PIXAR_IMAGEFULLWIDTH and TIFFTAG_PIXAR_IMAGEFULLLENGTH
    // are set when an image has been cropped out of a larger image.  
    // They reflect the size of the original uncropped image.
    // The TIFFTAG_XPOSITION and TIFFTAG_YPOSITION can be used
    // to determine the position of the smaller image in the larger one.
    if (TIFFSetField
        (tiffFile, TIFFTAG_PIXAR_IMAGEFULLWIDTH, crop_info->full_width) == 0)
        dieWithError(errMsg);
    if (TIFFSetField
        (tiffFile, TIFFTAG_PIXAR_IMAGEFULLLENGTH,
         crop_info->full_height) == 0)
        dieWithError(errMsg);
}



int readTIFF(Image * im, fullPath * sfile)
{
    char filename[512];
    TIFF *tif;
    int result = 0;

#ifdef __Mac__
    unsigned char the_pcUnixFilePath[512];      //added by Kekus Digital
    Str255 the_cString;
    Boolean the_bReturnValue;
    CFStringRef the_FilePath;
    CFURLRef the_Url;           //till here
#endif

    if (FullPathtoString(sfile, filename))
    {
        PrintError("Could not get filename");
        return -1;
    }

#ifdef __Mac__
    CopyCStringToPascal(filename, the_cString); //Added by Kekus Digital
    the_FilePath =
        CFStringCreateWithPascalString(kCFAllocatorDefault, the_cString,
                                       kCFStringEncodingUTF8);
    the_Url =
        CFURLCreateWithFileSystemPath(kCFAllocatorDefault, the_FilePath,
                                      kCFURLHFSPathStyle, false);
    the_bReturnValue =
        CFURLGetFileSystemRepresentation(the_Url, true, the_pcUnixFilePath,
                                         512);

    strcpy(filename, the_pcUnixFilePath);       //till here
#endif

    tif = TIFFOpen(filename, "r");

    if (!tif)
    {
        PrintError("Could not open tiff-file");
        return -1;
    }
    result = readtif(im, tif);

    //Store name of TIFF file
    strncpy(im->name, filename, 255);

    getCropInformationFromTiff(tif, &(im->cropInformation));

    TIFFClose(tif);
    return result;
}


int writeCroppedTIFF(Image * im, fullPath * sfile, CropInfo * crop_info)
{
    char string[512];
    TIFF *tif;
    UCHAR *buf;
    unsigned int y;
    size_t bufsize;

#ifdef __Mac__
    unsigned char the_pcUnixFilePath[512];      //added by Kekus Digital
    Str255 the_cString;
    Boolean the_bReturnValue;
    CFStringRef the_FilePath;
    CFURLRef the_Url;           //till here
#endif

    if (FullPathtoString(sfile, string))
    {
        PrintError("Could not get filename");
        return -1;
    }

#ifdef __Mac__
    CopyCStringToPascal(string, the_cString);   //added by Kekus Digital
    the_FilePath =
        CFStringCreateWithPascalString(kCFAllocatorDefault, the_cString,
                                       kCFStringEncodingUTF8);
    the_Url =
        CFURLCreateWithFileSystemPath(kCFAllocatorDefault, the_FilePath,
                                      kCFURLHFSPathStyle, false);
    the_bReturnValue =
        CFURLGetFileSystemRepresentation(the_Url, true, the_pcUnixFilePath,
                                         512);

    strcpy(string, the_pcUnixFilePath); //till here
#endif

    tif = TIFFOpen(string, "w");
    if (!tif)
    {
        PrintError("Could not create TIFF-file");
        return -1;
    }

    // Rik's mask-from-focus hacking
    if (ZCombSeeImage(im, string))
    {
        PrintError("failed ZCombSeeImage");
    }
    // end Rik's mask-from-focus hacking (for the moment...)

    TIFFSetField(tif, TIFFTAG_IMAGEWIDTH, im->width);
    TIFFSetField(tif, TIFFTAG_IMAGELENGTH, im->height);
    TIFFSetField(tif, TIFFTAG_PHOTOMETRIC, PHOTOMETRIC_RGB);
    TIFFSetField(tif, TIFFTAG_PLANARCONFIG, PLANARCONFIG_CONTIG);

    // Thomas Rauscher, Add for 32 bit (float) support
    //  
    //      TIFFSetField(tif, TIFFTAG_BITSPERSAMPLE, im->bitsPerPixel < 48 ? 8 : 16 );
    //  TIFFSetField(tif, TIFFTAG_SAMPLESPERPIXEL, im->bitsPerPixel == 24 || im->bitsPerPixel == 48 ? 3 : 4);

    switch (im->bitsPerPixel)
    {
    case 24:
        TIFFSetField(tif, TIFFTAG_BITSPERSAMPLE, 8);
        TIFFSetField(tif, TIFFTAG_SAMPLESPERPIXEL, 3);
        break;
    case 32:
        TIFFSetField(tif, TIFFTAG_BITSPERSAMPLE, 8);
        TIFFSetField(tif, TIFFTAG_SAMPLESPERPIXEL, 4);
        break;
    case 48:
        TIFFSetField(tif, TIFFTAG_BITSPERSAMPLE, 16);
        TIFFSetField(tif, TIFFTAG_SAMPLESPERPIXEL, 3);
        break;
    case 64:
        TIFFSetField(tif, TIFFTAG_BITSPERSAMPLE, 16);
        TIFFSetField(tif, TIFFTAG_SAMPLESPERPIXEL, 4);
        break;
    case 96:
        TIFFSetField(tif, TIFFTAG_BITSPERSAMPLE, 32);
        TIFFSetField(tif, TIFFTAG_SAMPLESPERPIXEL, 3);
        TIFFSetField(tif, TIFFTAG_SAMPLEFORMAT, SAMPLEFORMAT_IEEEFP);
        break;
    case 128:
        TIFFSetField(tif, TIFFTAG_BITSPERSAMPLE, 32);
        TIFFSetField(tif, TIFFTAG_SAMPLESPERPIXEL, 4);
        TIFFSetField(tif, TIFFTAG_SAMPLEFORMAT, SAMPLEFORMAT_IEEEFP);
        break;
    }
    TIFFSetField(tif, TIFFTAG_COMPRESSION, COMPRESSION_PACKBITS);


    //"1" indicates that The 0th row represents the visual top of the image, 
    //and the 0th column represents the visual left-hand side.
    TIFFSetField(tif, TIFFTAG_ORIENTATION, ORIENTATION_TOPLEFT);


    //TIFFTAG_ROWSPERSTRIP indicates the number of rows per "strip" of TIFF data.  The original PTStitcher
    //set this value to the panorama height whch meant that the entire image
    //was contained in one strip.  This is not only explicitly discouraged by the 
    //TIFF specification ("Use of a single strip is not recommended. Choose RowsPerStrip 
    //such that each strip is about 8K bytes, even if the data is not compressed, 
    //since it makes buffering simpler for readers. The 8K value is fairly 
    //arbitrary, but seems to work well."), but is also makes it impossible
    //for programs to read the output from Pano Tools to perform random 
    //access on the data which leads to unnecessarily inefficient approaches to 
    //manipulating these images).
    //
    //In practice, most panoramas generated these days (Feb 2006) contain more than 
    //2000 pixels per row (equal to 8KB mentioned above), so it is easiest to
    //hard-code this value to one, which also enables complete random access to 
    //the output files by subsequent blending/processing applications


    TIFFSetField(tif, TIFFTAG_ROWSPERSTRIP, 1);

    if (crop_info != NULL)
        setCropInformationInTiff(tif, crop_info);

    bufsize = TIFFScanlineSize(tif);
    if (bufsize < im->bytesPerLine)
        bufsize = im->bytesPerLine;
    buf = (UCHAR *) malloc(bufsize);
    if (buf == NULL)
    {
        PrintError("Not enough memory");
        return -1;
    }

    for (y = 0; y < im->height; y++)
    {
        memcpy(buf, *(im->data) + y * im->bytesPerLine,
               (size_t) im->bytesPerLine);
        ARGBtoRGBA(buf, im->width, im->bitsPerPixel);
        TIFFWriteScanline(tif, buf, y, 1);
    }
    TIFFClose(tif);
    free(buf);
    return 0;

}

int writeTIFF(Image * im, fullPath * sfile)
{
    return writeCroppedTIFF(im, sfile, &(im->cropInformation));
}



void RGBAtoARGB(UCHAR * buf, int width, int bitsPerPixel)
{
    int x;
    switch (bitsPerPixel)
    {
    case 32:
        {
            UCHAR pix;
            for (x = 0; x < width; x++, buf += 4)
            {
                pix = buf[3];
                buf[3] = buf[2];
                buf[2] = buf[1];
                buf[1] = buf[0];
                buf[0] = pix;
            }
        }
        break;
    case 64:
        {
            USHORT *bufs = (USHORT *) buf, pix;
            for (x = 0; x < width; x++, bufs += 4)
            {
                pix = bufs[3];
                bufs[3] = bufs[2];
                bufs[2] = bufs[1];
                bufs[1] = bufs[0];
                bufs[0] = pix;
            }
        }
        break;
    case 128:
        {
            float *bufs = (float *) buf, pix;
            for (x = 0; x < width; x++, bufs += 4)
            {
                pix = bufs[3];
                bufs[3] = bufs[2];
                bufs[2] = bufs[1];
                bufs[1] = bufs[0];
                bufs[0] = pix;
            }
        }
        break;
    }
}

void ARGBtoRGBA(UCHAR * buf, int width, int bitsPerPixel)
{
    int x;
    switch (bitsPerPixel)
    {
    case 32:
        {
            UCHAR pix;
            for (x = 0; x < width; x++, buf += 4)
            {
                pix = buf[0];
                buf[0] = buf[1];
                buf[1] = buf[2];
                buf[2] = buf[3];
                buf[3] = pix;
            }
        }
        break;
    case 64:
        {
            USHORT *bufs = (USHORT *) buf, pix;
            for (x = 0; x < width; x++, bufs += 4)
            {
                pix = bufs[0];
                bufs[0] = bufs[1];
                bufs[1] = bufs[2];
                bufs[2] = bufs[3];
                bufs[3] = pix;
            }
        }
        break;
    case 128:
        {
            float *bufs = (float *) buf, pix;
            for (x = 0; x < width; x++, bufs += 4)
            {
                pix = bufs[0];
                bufs[0] = bufs[1];
                bufs[1] = bufs[2];
                bufs[2] = bufs[3];
                bufs[3] = pix;
            }
        }
        break;
    }
}



//////////////////////////////////////////////////////////////////////
// NEW tiff routines
//////////////////////////////////////////////////////////////////////
#define PANO_DEFAULT_PIXELS_PER_RESOLUTION 150.0
#define PANO_DEFAULT_TIFF_RESOLUTION_UNITS  RESUNIT_INCH
#define PANO_DEFAULT_TIFF_COMPRESSION       




int panoTiffGetCropInformation(pano_Tiff * file)
{
/*
  Read the crop information of a TIFF file

  Cropped TIFFs have the following properties:

  * Their image width and length is the size of the cropped region
  * The full size of the image is in PIXAR_IMAGEFULLWIDTH and PIXAR_IMAGEFULLHEIGHT
  * If these 2 records do not exist then assume it is uncropped


 */

    float x_position, x_resolution, y_position, y_resolution;
    pano_CropInfo *c;

    assert(file != NULL);
    assert(file->tiff != NULL);

    c = &(file->metadata.cropInfo);
    c->croppedWidth = 0;

    file->metadata.isCropped = FALSE;

    if (TIFFGetField(file->tiff, TIFFTAG_IMAGEWIDTH, &(c->croppedWidth)) == 0
        || TIFFGetField(file->tiff, TIFFTAG_IMAGELENGTH,
                        &(c->croppedHeight)) == 0) {
        PrintError("Error reading file size from TIFF");
        return FALSE;
    }

    //If nothing is stored in these tags, then this must be an "uncropped" TIFF 
    //file, in which case, the "full size" width/height is the same as the 
    //"cropped" width and height
    if (TIFFGetField
        (file->tiff, TIFFTAG_PIXAR_IMAGEFULLWIDTH, &(c->fullWidth)) == 0) {
        c->fullWidth = c->croppedWidth;
    }
    else {
        file->metadata.isCropped = TRUE;
    }

    if (TIFFGetField
        (file->tiff, TIFFTAG_PIXAR_IMAGEFULLLENGTH, &(c->fullHeight)) == 0) {
        c->fullHeight = c->croppedHeight;
    }
    else {
        file->metadata.isCropped = TRUE;
    }

    // The position of the file is given in "real" dimensions, we have to rescale them
    if (TIFFGetField(file->tiff, TIFFTAG_XPOSITION, &x_position) == 0)
        x_position = 0;
    if (TIFFGetField(file->tiff, TIFFTAG_XRESOLUTION, &x_resolution) == 0)
        x_resolution = 0;
    if (TIFFGetField(file->tiff, TIFFTAG_YPOSITION, &y_position) == 0)
        y_position = 0;
    if (TIFFGetField(file->tiff, TIFFTAG_YRESOLUTION, &y_resolution) == 0)
        y_resolution = 0;

    //offset in pixels of "cropped" image from top left corner of 
    //full image (rounded to nearest integer)
    c->xOffset = (uint32) ((x_position * x_resolution) + 0.49);
    c->yOffset = (uint32) ((y_position * y_resolution) + 0.49);

    //printf("%s: %dx%d  @ %d,%d", filename, c->cropped_width, c->cropped_height, c->x_offset, c->y_offset);

    //printf("get 3 width %d length %d\n", (int) c->croppedWidth,
    //(int) c->croppedHeight);
    //printf("get 3 full %d length %d\n", (int) c->fullWidth,
    //           (int) c->fullHeight);
    //printf("cropped %d\n", (int) file->metadata.isCropped);

    return TRUE;

}

// checks if an "absolute" row is inside the ROI
int panoTiffRowInsideROI(pano_Tiff * image, int row)
{
    // We are in the ROI if the row is bigger than the yoffset
    // and the row is less or equal to the offset + height


    assert(image != NULL);
    assert(row >= 0);

    return panoROIRowInside(&(image->metadata.cropInfo), row);


}



int panoTiffIsCropped(pano_Tiff * file)
{
    return file->metadata.isCropped;
}

int panoTiffBytesPerLine(pano_Tiff * file)
{
    return file->metadata.bytesPerLine;
}

int panoTiffSamplesPerPixel(pano_Tiff * file)
{
    return file->metadata.samplesPerPixel;
}



int panoTiffBitsPerPixel(pano_Tiff * file)
{
    return file->metadata.bitsPerPixel;
}

int panoTiffBytesPerPixel(pano_Tiff * file)
{
    return file->metadata.bytesPerPixel;
}

int panoTiffImageHeight(pano_Tiff * file)
{
    return file->metadata.imageHeight;
}

int panoTiffImageWidth(pano_Tiff * file)
{
    return file->metadata.imageWidth;
}

int panoTiffXOffset(pano_Tiff * file)
{
    return file->metadata.cropInfo.xOffset;
}

int panoTiffYOffset(pano_Tiff * file)
{
    return file->metadata.cropInfo.yOffset;
}

pano_ImageMetadata *panoTiffImageMetadata(pano_Tiff * file)
{
    return &file->metadata;
}

int panoTiffFullImageWidth(pano_Tiff * file)
{
    return file->metadata.cropInfo.fullWidth;
}

int panoTiffFullImageHeight(pano_Tiff * file)
{
    return file->metadata.cropInfo.fullHeight;
}





// Read an "absolute" row relative to the cropped area of the TIFF
int panoTiffReadScanLineFullSize(pano_Tiff * file, void *buffer, int row)
{
    // Reads a scan line only if inside ROI, otherwise it only "zeroes" data
    int bytesPerLine;
    int bytesPerPixel;

    assert(file != NULL);

    if (row > panoTiffFullImageHeight(file)) {
        PrintError("Trying to read row %d beyond end of file", row);
        return FALSE;
    }
    bytesPerPixel = panoTiffBytesPerPixel(file);

    bytesPerLine = panoTiffFullImageWidth(file) * bytesPerPixel;

	//printf("Bytes per line %d %d\n", bytesPerLine, panoTiffFullImageWidth(file));

    assert(panoTiffIsCropped(file) ||
           panoTiffFullImageWidth(file) == panoTiffImageWidth(file));


    bzero(buffer, bytesPerLine);

    if (panoTiffRowInsideROI(file, row)) {
        if (TIFFReadScanline
            (file->tiff, buffer + panoTiffXOffset(file) * bytesPerPixel,
             row - panoTiffYOffset(file), 0) != 1) {
            PrintError("Error reading row %d in tiff file", row);
            return FALSE;
        }
    }
    return TRUE;
}

int panoTiffWriteScanLineFullSize(pano_Tiff * file, void *buffer, int row)
{
    // Reads a scan line only if inside ROI, otherwise it only "zeroes" data
    int bytesPerLine;
    int bytesPerPixel;

    assert(file != NULL);

    if (row > panoTiffFullImageHeight(file)) {
        PrintError("Trying to read row %d beyond end of file", row);
        return FALSE;
    }
    bytesPerPixel = panoTiffBytesPerPixel(file);

    bytesPerLine = panoTiffFullImageWidth(file) * bytesPerPixel;

    assert(panoTiffIsCropped(file) ||
           panoTiffFullImageWidth(file) == panoTiffImageWidth(file));


    if (panoTiffRowInsideROI(file, row)) {
        if (TIFFWriteScanline
            (file->tiff, buffer + panoTiffXOffset(file) * bytesPerPixel,
             row - panoTiffYOffset(file), 0) != 1) {
            PrintError("Error writing row %d in tiff file", row);
            return FALSE;
        }
    }
    return TRUE;
}



int panoTiffSetCropInformation(pano_Tiff * file)
{
    pano_CropInfo *cropInfo;
    pano_ImageMetadata *metadata;
    TIFF *tiffFile;
    int result;

    assert(file != NULL);

    tiffFile = file->tiff;
    assert(tiffFile != NULL);
    metadata = &(file->metadata);
    cropInfo = &(metadata->cropInfo);


    if (!panoTiffIsCropped(file))
        return TRUE;

    //The X offset in ResolutionUnits of the left side of the image, with 
    //respect to the left side of the page.
    //The Y offset in ResolutionUnits of the top of the image, with 
    //respect to the top of the page.

    result =
        TIFFSetField(tiffFile, TIFFTAG_XPOSITION,
                     (float) cropInfo->xOffset /
                     metadata->xPixelsPerResolution)
        && TIFFSetField(tiffFile, TIFFTAG_YPOSITION,
                        (float) cropInfo->yOffset /
                        metadata->yPixelsPerResolution);

    //The number of pixels per ResolutionUnit in the ImageWidth
    //The number of pixels per ResolutionUnit in the ImageLength (height)
    result = result &&
        TIFFSetField(tiffFile, TIFFTAG_XRESOLUTION,
                     metadata->xPixelsPerResolution)
        && TIFFSetField(tiffFile, TIFFTAG_YRESOLUTION,
                        metadata->yPixelsPerResolution);

    //The size of the picture represented by an image.  This
    //is required so that the computation of pixel offset using XPOSITION/YPOSITION and
    //XRESOLUTION/YRESOLUTION is valid (See tag description for XPOSITION/YPOSITION).
    result = result &&
        TIFFSetField(tiffFile, TIFFTAG_RESOLUTIONUNIT,
                     metadata->resolutionUnits);

    // TIFFTAG_PIXAR_IMAGEFULLWIDTH and TIFFTAG_PIXAR_IMAGEFULLLENGTH
    // are set when an image has been cropped out of a larger image.  
    // They reflect the size of the original uncropped image.
    // The TIFFTAG_XPOSITION and TIFFTAG_YPOSITION can be used
    // to determine the position of the smaller image in the larger one.
    result = result &&
        TIFFSetField(tiffFile, TIFFTAG_PIXAR_IMAGEFULLWIDTH,
                     cropInfo->fullWidth)
        && TIFFSetField(tiffFile, TIFFTAG_PIXAR_IMAGEFULLLENGTH,
                        cropInfo->fullHeight);
    if (!result) {
        PrintError("Unable to set metadata of output tiff file");
        return FALSE;
    }
    return result;
}

char *panoTiffGetString(pano_Tiff *tiffFile, ttag_t tiffTag)
{
    char *temp;
    char *returnValue;
    if (TIFFGetField(tiffFile->tiff, tiffTag, &temp) == 0) {
    // If the tag does not exist just return
        return NULL;
    } 
    else {
    // Allocate its memory
    returnValue = calloc(strlen(temp) + 1, 1);

    if (returnValue == NULL) 
        return NULL;
    // copy to the new location and return
    strcpy(returnValue, temp);
    return returnValue;
    }
}

int panoTiffGetImageProperties(pano_Tiff * tiff)
{
/*
  Retrieve the properties of the image that we need to keep
 */


    TIFF *tiffFile;
    pano_ImageMetadata *metadata;
    int result;
    void *ptr;

    assert(tiff != NULL);

    tiffFile = tiff->tiff;

    metadata = &tiff->metadata;

    assert(tiffFile != NULL);

    //printf("get\n");

    if (!panoTiffGetCropInformation(tiff)) {
        goto error;
    }


    // These are tags that are expected to be present

    result = TIFFGetField(tiffFile, TIFFTAG_IMAGEWIDTH, &metadata->imageWidth)
        && TIFFGetField(tiffFile, TIFFTAG_IMAGELENGTH, &metadata->imageHeight)
        && TIFFGetField(tiffFile, TIFFTAG_BITSPERSAMPLE,
                        &metadata->bitsPerSample)
        && TIFFGetField(tiffFile, TIFFTAG_SAMPLESPERPIXEL,
                        &metadata->samplesPerPixel)
        && TIFFGetField(tiffFile, TIFFTAG_COMPRESSION,
                        &metadata->compression.type)
        && TIFFGetField(tiffFile, TIFFTAG_ROWSPERSTRIP,
                        &metadata->rowsPerStrip);


    if (!result)
        goto error;

    if (metadata->compression.type == COMPRESSION_LZW) {
        //predictor only exists in LZW compressed files
        if (!TIFFGetField
            (tiffFile, TIFFTAG_PREDICTOR,
             &(metadata->compression.predictor))) {
            goto error;
        }
    }

    metadata->bytesPerLine = TIFFScanlineSize(tiffFile);
    if (metadata->bytesPerLine <= 0) {
        PrintError("File did not include proper bytes per line information.");
        return 0;
    }

    // These are optional tags
    
    

    if (TIFFGetField(tiffFile, TIFFTAG_ICCPROFILE, &(metadata->iccProfile.size),
             &ptr)) {

        if ((metadata->iccProfile.data = calloc(metadata->iccProfile.size, 1)) == NULL) {
            PrintError("Not enough memory");
            return 0;
        }
        memcpy(metadata->iccProfile.data, ptr, metadata->iccProfile.size);
    }

    tiff->metadata.copyright = panoTiffGetString(tiff, TIFFTAG_COPYRIGHT);
    tiff->metadata.datetime = panoTiffGetString(tiff, TIFFTAG_DATETIME);
    tiff->metadata.imageDescription = panoTiffGetString(tiff, TIFFTAG_IMAGEDESCRIPTION);
    tiff->metadata.artist = panoTiffGetString(tiff, TIFFTAG_ARTIST);
    

    //printf("...........................REad and allocate ICC Profile %d %x\n",
    //(int)(metadata->iccProfile.size), (int)(metadata->iccProfile.data));

    if (TIFFGetField
        (tiffFile, TIFFTAG_RESOLUTIONUNIT, &(metadata->resolutionUnits)) == 0)
        metadata->resolutionUnits = PANO_DEFAULT_TIFF_RESOLUTION_UNITS; 

    if (TIFFGetField
        (tiffFile, TIFFTAG_XRESOLUTION,
         &(metadata->xPixelsPerResolution)) == 0)
        metadata->xPixelsPerResolution =
            PANO_DEFAULT_PIXELS_PER_RESOLUTION;

    if (TIFFGetField
        (tiffFile, TIFFTAG_YRESOLUTION,
         &(metadata->yPixelsPerResolution)) == 0)
        metadata->yPixelsPerResolution =
            PANO_DEFAULT_PIXELS_PER_RESOLUTION;

    // Compute rest of the fields 

    // let us truly hope the size of a byte never changes :)
    metadata->bytesPerPixel =
        (metadata->samplesPerPixel * metadata->bitsPerSample) / 8;
    metadata->bitsPerPixel = metadata->bytesPerPixel * 8;

    //printf("get2 bits per sample %d\n", metadata->bitsPerSample);
    //    printf("get2 bits per pixel  %d\n", metadata->bitsPerPixel);
    //printf("get2 bytes per pixel %d\n", metadata->bytesPerPixel);

    return 1;

  error:
    PrintError("Error retrieving metadata from TIFF file");
    return 0;

}

int panoTiffSetImageProperties(pano_Tiff * file)
{
    int returnValue = 1;
    TIFF *tiffFile;
    pano_ImageMetadata *metadata;

    assert(file != NULL);

    tiffFile = file->tiff;
    assert(tiffFile != NULL);

    metadata = &(file->metadata);

    assert(metadata != NULL);

    // Each of the invocations below returns 1 if ok, 0 if error. 

    //printf("samples per pixel %d\n", (int) metadata->samplesPerPixel);
    //printf("samples width %d\n", (int) metadata->imageWidth);
    //printf("compression %d\n", (int) metadata->compression.type);
    returnValue =
        TIFFSetField(tiffFile, TIFFTAG_IMAGEWIDTH, metadata->imageWidth)
        && TIFFSetField(tiffFile, TIFFTAG_IMAGELENGTH, metadata->imageHeight)
        && TIFFSetField(tiffFile, TIFFTAG_BITSPERSAMPLE,
                        metadata->bitsPerSample)
        && TIFFSetField(tiffFile, TIFFTAG_PHOTOMETRIC, PHOTOMETRIC_RGB)
        && TIFFSetField(tiffFile, TIFFTAG_PLANARCONFIG, PLANARCONFIG_CONTIG)
        && TIFFSetField(tiffFile, TIFFTAG_SAMPLESPERPIXEL,
                        metadata->samplesPerPixel)
        && TIFFSetField(tiffFile, TIFFTAG_COMPRESSION,
                        metadata->compression.type)
        && TIFFSetField(tiffFile, TIFFTAG_ORIENTATION, ORIENTATION_TOPLEFT)
        && TIFFSetField(tiffFile, TIFFTAG_ROWSPERSTRIP,
                        metadata->rowsPerStrip)
        && TIFFSetField(tiffFile, TIFFTAG_RESOLUTIONUNIT,
                        metadata->resolutionUnits)
        && TIFFSetField(tiffFile, TIFFTAG_XRESOLUTION,
                        metadata->xPixelsPerResolution)
        && TIFFSetField(tiffFile, TIFFTAG_YRESOLUTION,
                        metadata->yPixelsPerResolution);

    if (returnValue && metadata->bitsPerSample == 32)
    {
    // If it is 96 or 128 it is  floatint point
        returnValue = TIFFSetField(tiffFile, TIFFTAG_SAMPLEFORMAT, SAMPLEFORMAT_IEEEFP);
    }
    // Take care of special cases

    // Only set ICCprofile if size > 0
    if (returnValue && metadata->iccProfile.size > 0) {
        returnValue =
            TIFFSetField(tiffFile, TIFFTAG_ICCPROFILE,
             (uint32)metadata->iccProfile.size,
             (void*)(metadata->iccProfile.data));
             //100, data);
    }

    if (returnValue && metadata->compression.type == COMPRESSION_LZW) {
        returnValue =
            TIFFSetField(tiffFile, TIFFTAG_PREDICTOR,
                         metadata->compression.predictor);
    }

    // String fields
    //printf("TO set tricky fields\n");

    if (returnValue && metadata->copyright != NULL)
        returnValue = TIFFSetField(tiffFile, TIFFTAG_COPYRIGHT, metadata->copyright);

    if (returnValue && metadata->artist != NULL)
        returnValue = TIFFSetField(tiffFile, TIFFTAG_ARTIST, metadata->artist);

    if (returnValue && metadata->datetime!=NULL)
        returnValue = TIFFSetField(tiffFile, TIFFTAG_DATETIME, metadata->datetime);

    if (returnValue && metadata->imageDescription != NULL)
        returnValue = TIFFSetField(tiffFile, TIFFTAG_IMAGEDESCRIPTION, metadata->imageDescription);
    
    
    returnValue = returnValue &&
        TIFFSetField(tiffFile, TIFFTAG_SOFTWARE, "Created by Panotools version " VERSION);
    
    //    printf("TO set crop\n");
    if (returnValue && panoTiffIsCropped(file)) {
      //printf("TO set crop 2\n");
        returnValue = panoTiffSetCropInformation(file);
    }

    return returnValue;

}



int panoTiffReadPlannar(Image * im, pano_Tiff * tif)
{
    UCHAR *buf;
    uint32 row;
    short samplesPerPixel;
    int bytesRead;
    int bitsPerPixel;

    samplesPerPixel = panoTiffSamplesPerPixel(tif);


    // We can't read more than 4 samples per pixel
    if (samplesPerPixel > 4 || samplesPerPixel < 3) {
        PrintError("We only support 3 or 4 samples per pixel in TIFF");
        return 0;
    }
    // REmember, we need the values of the TIFF,
    // not the values in the image struct
    // (which will might not be the same
    bytesRead = panoTiffBytesPerLine(tif);
    bitsPerPixel = panoTiffBitsPerPixel(tif);

    //    printf("9 Bytes per line %d (im %d) %d %d\n", panoTiffBytesPerLine(tif),
    //     im->bytesPerLine, im->bitsPerPixel, panoTiffBitsPerPixel(tif));
    buf = calloc(bytesRead, 1);
    if (buf == NULL) {
        PrintError("Not enough memory");
        return 0;
    }

    for (row = 0; row < im->height; row++) {
        if (TIFFReadScanline(tif->tiff, buf, row, 0) != 1) {
            PrintError("Error reading TIFF file");
            goto error;
        }

        RGBAtoARGB(buf, im->width, bitsPerPixel);

        memcpy(*(im->data) + row * im->bytesPerLine, buf,
               (size_t) bytesRead);
    }

    // If we don't have an alpha channel we need to rebuild it
    if (samplesPerPixel == 3) {
        ThreeToFourBPP(im);
    }
    return 1;

 error:
    free(buf);
    return 0;
}







void panoTiffClose(pano_Tiff * file)
{
    panoMetadataFree(&file->metadata);
    TIFFClose(file->tiff);
    free(file);
}


// create the tiff according to most of the needed data
pano_Tiff *panoTiffCreateGeneral(char *fileName,
                                 pano_ImageMetadata * metadata, int uncropped)
{
    pano_Tiff *panoTiff;

    // Allocate the struct's memory
    if ((panoTiff = calloc(sizeof(pano_Tiff), 1)) == NULL) {
        PrintError("Not enough memory");
        return NULL;
    }

    // Open file and retrieve metadata
    panoTiff->tiff = TIFFOpen(fileName, "w");
    if (panoTiff->tiff == NULL) {
        PrintError("Unable to create output file [%s]", fileName);
        return NULL;
    }

    //printf("Copy metadata from %d\n", (int) metadata->cropInfo.fullWidth);
    if (!panoMetadataCopy(&panoTiff->metadata, metadata)) {
        panoTiffClose(panoTiff);
        return NULL;
    }

    if (uncropped) {
        panoUnCropMetadata(&panoTiff->metadata);
    }

    //printf("Copy metadata %d\n", (int) panoTiff->metadata.cropInfo.fullWidth);
    if (!panoTiffSetImageProperties(panoTiff)) {
        panoTiffClose(panoTiff);
        return NULL;
    }
    //printf("After set image properties\n");

    // return value
    return panoTiff;
}

pano_Tiff *panoTiffCreateUnCropped(char *fileName,
                                   pano_ImageMetadata * metadata)
{
    // If the file is uncropped it creates a cropped version
    return panoTiffCreateGeneral(fileName, metadata, TRUE);
}


pano_Tiff *panoTiffCreate(char *fileName, pano_ImageMetadata * metadata)
{
    return panoTiffCreateGeneral(fileName, metadata, FALSE);
}

pano_Tiff *panoTiffOpen(char *fileName)
{
    pano_Tiff *panoTiff;


    // Allocate the struct's memory
    if ((panoTiff = calloc(sizeof(*panoTiff), 1)) == NULL) {
        PrintError("Not enough memory");
        return NULL;
    }

    // Open file and retrieve metadata
    panoTiff->tiff = TIFFOpen(fileName, "r");

    if (panoTiff->tiff != NULL) {
        if (!panoTiffGetImageProperties(panoTiff)) {
            TIFFClose(panoTiff->tiff);
            free(panoTiff);
            return NULL;
        }
    }
    // return value
    return panoTiff;
}


// image is allocated, but not image data

int panoTiffReadData(Image * im, pano_Tiff * tif)
{
    short tPhotoMetric, config;

    assert(im != NULL);
    // Assume that it is unallocated
    assert(im->data == NULL);

    assert(tif != NULL);

    TIFFGetField(tif->tiff, TIFFTAG_PHOTOMETRIC, &tPhotoMetric);
    TIFFGetField(tif->tiff, TIFFTAG_PLANARCONFIG, &config);


    // Set general parameters. The width and height are of the data

    if ((im->data = (unsigned char **) mymalloc( im->dataSize) ) == NULL) {
        PrintError("Not enough memory");
        return 0;
    }
        
    if (tPhotoMetric == PHOTOMETRIC_RGB && config == PLANARCONFIG_CONTIG) {
        if (!panoTiffReadPlannar(im, tif))
            goto error;
    return TRUE;
    }

    // I changed the stopOnError to 1 so the function reports an error
    // as soon as it happens

    if (TIFFReadRGBAImage(tif->tiff, (uint32) panoTiffImageWidth(tif), 
                          (uint32) panoTiffImageHeight(tif), 
                          (uint32 *) * (im->data), 1)) {
        // Convert agbr to argb; flip image vertically

        unsigned char *cline, *ct, *cb;
        int h2 = im->height / 2, y;
        // Only do the conversion once
        size_t localBytesPerLine = im->bytesPerLine;

        cline = (unsigned char *) calloc(localBytesPerLine, 1);
        if (cline == NULL)
        {
            PrintError("Not enough memory");
            goto error;
        }

        ct = *im->data;
        cb = *im->data + (im->height - 1) * im->bytesPerLine;

        for (y = 0; y < h2;
             y++, ct += im->bytesPerLine, cb -= im->bytesPerLine) {
            RGBAtoARGB(ct, im->width, im->bitsPerPixel);
            RGBAtoARGB(cb, im->width, im->bitsPerPixel);
            memcpy(cline, ct, localBytesPerLine);
            memcpy(ct, cb, localBytesPerLine);
            memcpy(cb, cline, localBytesPerLine);
        }
        if (im->height != 2 * h2) {                       // odd number of scanlines
            RGBAtoARGB(*im->data + y * im->bytesPerLine, im->width,
                       im->bitsPerPixel);
        }
        free(cline);
    }
    else {
        PrintError("Could not read tiff-data");
        goto error;
    }
    return 1;

 error:
    myfree((void**)im->data);
    return 0;
}




// Output an image to a file
int panoTiffWrite(Image * im, char *fileName)
{
    pano_Tiff *tif = NULL;
    void *buf = 0;
    unsigned int y;
    size_t bufsize;

    // Make sure that the metadata is there...
    assert(im->metadata.imageWidth != 0 &&
       im->metadata.imageHeight != 0);
    

    // first verify the value of some of the metadata fields
    
    assert(im->bitsPerPixel != 0);

    switch (im->bitsPerPixel) {
    case 96:
    case 24:
    case 48:
        im->metadata.samplesPerPixel = 3;
        break;
    case 32:
    case 64:
    case 128:
        im->metadata.samplesPerPixel = 4;
        break;
    default:
        PrintError("Illegal value for bits per pixel in TIFF image to write %s", fileName);
        return FALSE;
    }
    im->metadata.bitsPerSample = im->bitsPerPixel/im->metadata.samplesPerPixel;


    tif = panoTiffCreate(fileName, &im->metadata);


    if (!tif) {
        PrintError("Could not create TIFF-file");
        return 0;
    }

    // Rik's mask-from-focus hacking
    if (ZCombSeeImage(im, fileName)) {
        PrintError("failed ZCombSeeImage");
    }
    // end Rik's mask-from-focus hacking (for the moment...)

    bufsize = TIFFScanlineSize(tif->tiff);

    if (bufsize < im->bytesPerLine)
        bufsize = im->bytesPerLine;

    buf = calloc(bufsize, 0);
    if (buf == NULL) {
        PrintError("Not enough memory");
        goto error;
    }

    for (y = 0; y < im->height; y++) {
        memcpy(buf, *(im->data) + y * im->bytesPerLine,
               (size_t) im->bytesPerLine);
        ARGBtoRGBA(buf, im->width, im->bitsPerPixel);
        if (TIFFWriteScanline(tif->tiff, buf, y, 1) != 1) {
            PrintError("Unable to write to TIFF");
            goto error;
        }
    }
    panoTiffClose(tif);
    free(buf);
    return 1;
    
 error:
    if (buf != NULL) 
        free(buf);
    
    if (tif != NULL)
        panoTiffClose(tif);
    
    return 0;
}


int panoUpdateMetadataFromTiff(Image *im, pano_Tiff *tiff) 
{
    int bytesPerLine;
    
    if (!panoMetadataCopy(&im->metadata, &tiff->metadata)) {
        return FALSE;
    }
    //    printf("IMage width %d %d\n",im->width, panoTiffImageWidth(tiff));
    //printf("Bites per pixel %d\n",(int)im->bitsPerPixel);

    im->width = panoTiffImageWidth(tiff);
    im->height = panoTiffImageHeight(tiff);
    
    // We will allocate enough memory for the 3 samples + Alpha Channel
    // Regardless of the actual number of samples in the image

    im->bytesPerLine = panoTiffBytesPerLine(tiff);
    im->bitsPerPixel = panoTiffBitsPerPixel(tiff);

    // Even if we only find 3 samples we will end with 4 
    switch (panoTiffSamplesPerPixel(tiff))
    {
    case 3:
        bytesPerLine = panoTiffBytesPerLine(tiff) * 4 / 3;
        
        im->metadata.bytesPerLine = bytesPerLine;
        
        im->metadata.bitsPerPixel = im->bitsPerPixel * 4/ 3;
        im->metadata.samplesPerPixel = 4;
        im->metadata.bytesPerPixel =
            (4 * im->metadata.bitsPerSample) / 8;
        break;
    case 4:
        bytesPerLine = panoTiffBytesPerLine(tiff);
        break;
    default:
        PrintError("We only support 3 or 4 samples per pixel");
        return 0;
    }
    
    im->dataSize = bytesPerLine * im->height;
    
    // compute how much space we need
    //printf("Data size %d bytesperline %d width %d height %d\n",  
    //(int)im->dataSize,
    //       (int)im->bytesPerLine, (int)im->width,(int)im->height
    //);
    
    return TRUE;
}


/*
  Read a TIFF file and place it in a Image data structure
  Read also the metadata including crop information
*/

int panoTiffRead(Image * im, char *fileName)
{
    pano_Tiff *tiff;
    int result = FALSE;
    
    SetImageDefaults(im);
    
    //printf("Reading tiff\n");
    if ((tiff = panoTiffOpen(fileName)) == NULL) {
        PrintError("Could not open tiff-file %s", fileName);
        goto end;
    }
    //printf("to update metadata tiff\n");

    // Update metadata in the image
    if (!panoUpdateMetadataFromTiff(im, tiff)) {
        goto end;
    }
    
    //printf("to read data\n");
    
    if (!panoTiffReadData(im, tiff)) {
        PrintError("Unable to read data from TIFF file %s", fileName);
        goto end;
    }

    //Store name of TIFF file
    strncpy(im->name, fileName, PANO_PATH_LEN);


    //printf("after update metadata tiff\n");
    result = TRUE;
   
 end:

    //printf("ENd of Reading tiff\n");

    //panoDumpMetadata(&im->metadata,"Read metadata");

    panoTiffClose(tiff);
    return result;
}



void panoTiffErrorHandler(const char *module, const char *fmt, va_list ap)
{
    PrintError("Error in TIFF file (%s) ", module);
    PrintError((char *) fmt, ap);
}

void panoTiffSetErrorHandler(void)
{
  // TODO
  // This routines need to be properly implemented. Currently it does nothing

    //MRDL: Reluctantly commented these out...the calls to TIFFSetWarningHandler and 
    //TIFFSetErrorHandler cause to GCC to abort, with a series of errors like this:
    //../../../LibTiff/tiff-v3.6.1/libtiff/libtiff.a(tif_unix.o)(.text+0x11a): In function `TIFFOpen':
    //../../../libtiff/tiff-v3.6.1/libtiff/../libtiff/tif_unix.c:144: multiple definition of `TIFFOpen'
    //../libpano12.a(dyces00121.o)(.text+0x0): first defined here
    // Make sure we have a tiff error handler

#ifdef TOBEIMPLEMENTED

    TIFFSetWarningHandler(panoTiffErrorHandler);
    TIFFSetErrorHandler(panoTiffErrorHandler);

#endif
}


/* panotools is only able to operate on images that have the same size and same depth.
   if the colour profiles exist they should be the same too

   Some checksk are optional

*/
int panoTiffVerifyAreCompatible(fullPath * tiffFiles, int numberImages,
                                 int optionalCheck)
{
    int currentImage;
    pano_Tiff *firstFile;
    pano_Tiff *otherFile;

    pano_CropInfo *firstCropInfo = NULL;
    pano_CropInfo *otherCropInfo = NULL;

    assert(tiffFiles != NULL);
    assert(numberImages > 1);


    panoTiffSetErrorHandler();

    // Open TIFFs

    firstFile = panoTiffOpen(tiffFiles[0].name);
    firstCropInfo = &firstFile->metadata.cropInfo;


    if (firstFile == NULL) {
        PrintError("Unable to read tiff file %s", tiffFiles[0].name);
        return FALSE;
    }

    // Compare the metadata of the current file with each of the other ones
    for (currentImage = 1; currentImage < numberImages; currentImage++) {

        otherFile = panoTiffOpen(tiffFiles[currentImage].name);
	otherCropInfo = &otherFile->metadata.cropInfo;

        if (otherFile == NULL) {
            PrintError("Unable to read tiff file %s",
                       tiffFiles[currentImage].name);
            return FALSE;
        }


        // THey should have the same width
        if (panoTiffFullImageWidth(firstFile) !=
            panoTiffFullImageWidth(otherFile)) {
            PrintError
                ("Image 0 and %d do not have the same width: %d vs %d\n",
                 currentImage, (int) firstCropInfo->fullWidth,
                 (int) otherCropInfo->fullWidth);
            return FALSE;
        }

        // THey should have the same height
        if (panoTiffFullImageHeight(firstFile) !=
            panoTiffFullImageHeight(otherFile)) {
            PrintError
                ("Image 0 and %d do not have the same length: %d vs %d\n",
                 currentImage, (int) firstCropInfo->fullHeight,
                 (int) otherCropInfo->fullHeight);
            return FALSE;
        }

        // THey should have the same colour depth
        if (panoTiffBytesPerPixel(firstFile) !=
            panoTiffBytesPerPixel(otherFile)) {
            PrintError("Image 0 and %d do not have the same colour depth\n",
                       currentImage);
            return FALSE;
        }
        //printf("compatible 1\n");
        // THey should have the same number of channels per pixel
        if (panoTiffSamplesPerPixel(firstFile) !=
            panoTiffSamplesPerPixel(otherFile)) {
            PrintError
                ("Image 0 and %d do not have the same number of channels per pixel\n",
                 currentImage);
            return FALSE;
        }

        if (optionalCheck) {

            // Compare profiles

            if (firstFile->metadata.iccProfile.size > 0) {

                //  They should be the same size and have the same contents
                if (firstFile->metadata.iccProfile.size !=
                    otherFile->metadata.iccProfile.size
                    || memcmp(firstFile->metadata.iccProfile.data,
                              otherFile->metadata.iccProfile.data,
                              firstFile->metadata.iccProfile.size) != 0) {
                    PrintError
                        ("Image 0 and %d have different colour profiles\n",
                         currentImage);
                    return FALSE;
                }
            }
        }
        panoTiffClose(otherFile);

    }                           // for loop

    panoTiffClose(firstFile);
    //printf("THe files are compatible\n");

    return TRUE;

}

