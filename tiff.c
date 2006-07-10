/* Panorama_Tools	-	Generate, Edit and Convert Panoramic Images
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


#include "filter.h"

#include "tiffio.h"

#include "ZComb.h"

int readplanarTIFF(Image * im, TIFF * tif);
void RGBAtoARGB(UCHAR * buf, int width, int bitsPerPixel);
void ARGBtoRGBA(UCHAR * buf, int width, int bitsPerPixel);
int readtif(Image * im, TIFF * tif);


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
    TIFFSetField(tif, TIFFTAG_ORIENTATION, ORIENTATION_TOPLEFT);
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
