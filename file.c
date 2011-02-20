/* Panorama_Tools   -   Generate, Edit and Convert Panoramic Images
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

// Functions to read and write Photoshop, and write tiffs

// Updated by Jim Watters 2003 Nov 21
// Every layer of a multi layer PSD file should have a shape mask.  
// The shape mask defines the shape of the image from the background.
// 3 problems with PTSTitcher
//  - The Seam options for middle(s0 'blend')and edge(s1 'paste') are ignored by PTStitcher
//  - Both the PSD options of "With_Mask" and "Without_Mask" both create mask except:
//       - the "With_Mask" puts the seam in the center and uses the feathering to blend
//       - the "Without_Mask" puts the seam at the edge and ignores the feathering, does no blending
//  - 16bit input images are reduced to 8bit and not able to create 16bit psd files.
// For backward compatability reasons continue to create mask for "WithOut_Mask"
// When creating multi image PSD files try to create shape mask (Alpha) and clip mask properly
// Updated writePSDwithLayer and addLayer to create both proper shape mask and clip mask
// As a hack to check to see if a shape mask can be created from the alpha channel do a check for feathering in the Alpha channel
// If Alpha channel has feathering create a new shape mask
// Add two new functions hasFeather and writeTransparentAlpha
// Enabled 16bit multilayer PSD files; except they still are 8bit; They are already converted in PTStitcher

// Updated by Jim Watters 2003 Nov 24
// fixed bug with odd data size for psd files
// There is only one pad char at the end of all layer and channel data, if everything is a odd length

// Update by Jim Watters 2003 Dec 29
// Also fix ReadPSD to handle multilayers

// Update by Rik Littlefield 2004 June 29
// Fix getImageRectangle to not pagefault so badly.
// Dynamically allocate scanline buffer in writeWhiteBackground,
// to avoid buffer overflow and crash on large images.
// Fix 16bit Radial Shift and Adjust - Kevin & Jim 2004 July
// 2006 Max Lyons         - Various modifications
#include <assert.h>
#include "filter.h"

#include "file.h"
#include "pttiff.h"
#include "metadata.h"

// local functions

static int              writeImageDataPlanar    ( Image *im, file_spec fnum );
static int              readImageDataPlanar     (Image *im, file_spec fnum ) ;
static int              ParsePSDHeader          ( char *header, Image *im, Boolean *pbBig );
static int              writeChannelData        ( Image *im, file_spec fnum, int channel, PTRect *r );
static int              writeLayerAndMask       ( Image *im, file_spec fnum, Boolean bBig );
static void             getImageRectangle       ( Image *im, PTRect *r );
static int              fileCopy                ( file_spec src, file_spec dest, size_t numBytes, unsigned char *buf);
static void             orAlpha                 ( unsigned char* alpha, unsigned char *buf, Image *im, PTRect *r );
static void             writeWhiteBackground    ( pt_int32 width, pt_int32 height, file_spec fnum, Boolean bBig  );
static int              addLayer                ( Image *im, file_spec src, file_spec fnum , stBuf *sB, Boolean bBig );
static int              hasFeather              ( Image *im );
static int              writeTransparentAlpha   ( Image *im, file_spec fnum, PTRect *theRect );


	

char *psdBlendingModesNames[] = {
    "Normal", 
    "Color",
    "Darken",
    "Difference",
    "Dissolve",
    "Hard Light",
    "Hue",
    "Lighten",
    "Luminosity",
    "Multiply",
    "Overlay",
    "Sof Light",
    "Saturation",
    "Screen"
};

		
char *psdBlendingModesInternalName[] = {
  "norm",
  "colr",
  "dark",
  "diff",
  "diss",
  "hLit",
  "hue",
  "lite",
  "lum",
  "mul",
  "over",
  "sLit",
  "sat",
  "scrn"
};








#define PSDHLENGTH 26

size_t panoPSDResourceWrite(file_spec fnum, pt_uint16 resource, pt_int32 len, size_t dataLen, char *resourceData)
{
//struct _ColorModeDataBlock
//{
//    BYTE Type[4];  /* Always "8BIM" */
//    WORD ID;       /* (See table below) */
//    BYTE Name[];   /* Even-length Pascal-format string, 2 bytes or longer */
//    LONG Size;     /* Length of resource data following, in bytes */
//    BYTE Data[];   /* Resource data, padded to even length */
//};
//
    size_t     startLocation = ftell(fnum);

    panoWriteUCHAR( fnum, '8' );
    panoWriteUCHAR( fnum, 'B' );
    panoWriteUCHAR( fnum, 'I' );
    panoWriteUCHAR( fnum, 'M' );
    panoWriteSHORT( fnum, resource);
    panoWriteSHORT( fnum, 0); // Null string 2 bytes (length)
    panoWriteINT32( fnum, len); //size of the record (4 bytes)

    if (dataLen > 0 && resourceData != NULL) {
        mywrite( fnum, dataLen, resourceData );
    }

    return ftell(fnum) - startLocation;
}

size_t panoPSDPICTResourceWrite(file_spec fnum, unsigned char resource, unsigned char record, size_t len, char *recordData)
{
  // See IPTC Information Intercharge Model Exchange Version 4.
    size_t     startLocation = ftell(fnum);

    panoWriteUCHAR( fnum, 0x1c );
    panoWriteUCHAR( fnum, resource );
    panoWriteUCHAR( fnum, record );
    panoWriteSHORT( fnum, (short)len ); //length

    if (len !=0 && recordData != NULL ) {

        mywrite( fnum, len,  recordData);
        if (len % 2 == 0) {
            // Odd size record! it should be aligned to word, but not reported in the lenght
            // and the record header is odd length (5 bytes) so...
            //    printf("Writing extra record to align\n");

            panoWriteUCHAR( fnum, 0 );
        }
    }

    return ftell(fnum) - startLocation;
}

#define CREATED_BY_PSD "Panotools " VERSION
#define IPTC_VERSION_ID 0
#define IPTC_ORIGINATING_PROGRAM_ID 0x41
#define IPTC_DESCRIPTION_WRITER_ID  0x7a

int panoPSDResourcesBlockWrite(Image *im, file_spec   fnum)
{
    size_t    rtn = 0;
    pt_int32  nResLength = 0;
    int       saveLocation = 0;
    int       saveLocationForSize=0;
    size_t    temp;


    // This function is a bit cryptic mainly because PSD forces the lenght to be known before a record is written. 
    // What I have decided is to make the writing process non-sequential.

    // Write an empty length first, then "rewind" and write its real
    // one. It will make things way easier and more maintainable.

    // Write 4 bytes
    saveLocationForSize = ftell(fnum);

    panoWriteINT32( fnum, 1234 );            // Image Resources size
    if (im->metadata.iccProfile.size > 0) {
        // Currently we only include ICC profile if it exists

        // We need to write an image Resources block
        // Write Image resources header
        // Write ICC Profile block
        panoPSDResourceWrite(fnum, 0x040f, im->metadata.iccProfile.size,
            im->metadata.iccProfile.size, im->metadata.iccProfile.data);
    }

    // we will refactor this chunck
    char IPTCVersion[3];
    IPTCVersion[0] = 0;
    IPTCVersion[1] = 2;
    IPTCVersion[2] = 0;

    // To write PICT we need to create a resource of type IPCT
    // Inside it write the sequence of PICT records

    // THIS IS UNTIL I FIX THIS CODE. IPTC records should be aligned and the header takes
    // 5 bytes
    // We also need to make it easier to add new records by
    //simplifying the computation of the length of the subrecords

    temp = (short)strlen(CREATED_BY_PSD);

    /// 8 of the VERSION ID + length of Descriptor (5 header + 2 strlen + string )
    nResLength = (pt_int32)(8 + 5 + 2 + temp);
    panoPSDResourceWrite(fnum, 0x0404, nResLength + nResLength%2, 0, NULL);
    panoPSDPICTResourceWrite(fnum, 0x02, IPTC_VERSION_ID, 2, IPTCVersion);
    panoPSDPICTResourceWrite(fnum, 0x02, IPTC_DESCRIPTION_WRITER_ID, temp, NULL );
    panoWriteSHORT( fnum, (short)temp );
      
    mywrite( fnum, temp, CREATED_BY_PSD);

    if( (ftell(fnum)-saveLocationForSize)%2)
    {
        panoWriteUCHAR( fnum, 0 );
    }

  // TODO:
  // caption abstract:    0x78 "script... (2000 bytes max)
  // if we have them: 
  // copyright
  // Set the time that PTmender runs
  // imagedescription
  // artist               0x80 (32 max)


  // Write length
  saveLocation = ftell(fnum);
  fseek(fnum,  saveLocationForSize, SEEK_SET);
  assert(saveLocation > saveLocationForSize);
  panoWriteINT32( fnum, saveLocation - saveLocationForSize-4 );            // Image Resources size
  fseek(fnum, saveLocation, SEEK_SET);

  return (int)rtn;
}


int writePSD ( Image *im, fullPath* fname )
{
  return writePS(im, fname, FALSE);
}

// Save image as the background, as a PSD or PSB file
// Image is background, there are no layers
int writePS(Image *im, fullPath *sfile, Boolean bBig )
{
    file_spec   fnum;
    int         channels;
    int         BitsPerChannel;

    // Check to see if we need to create PSB instead of PSD file
    if(im->height > 30000 || im->width > 30000)
      bBig = TRUE;

    GetChannels( im, channels );
    GetBitsPerChannel( im, BitsPerChannel );
    
    if(  myopen( sfile, write_bin, fnum )   )
    {   
        PrintError("Error Writing Image File");
        return -1;
    }
    //    printf("It is sropped %d\n", panoImageIsCropped(im));

    // Write PSD and PSB Header
    // WRITEINT32( '8BPS' );
    panoWriteUCHAR( fnum, '8' );
    panoWriteUCHAR( fnum, 'B' );
    panoWriteUCHAR( fnum, 'P' );
    panoWriteUCHAR( fnum, 'S' );
    panoWriteSHORT( fnum, bBig? 2 : 1 ); // PSD file must be 1, PSB files must be 2
    panoWriteINT32( fnum, 0 ); panoWriteSHORT( fnum, 0 ); // 6 bytes zeroed

    panoWriteSHORT( fnum, channels );// No of channels
        
    // TODO: IF THE SOURCE FILE IS CROPPED THIS IS WRITING ONLY THE CROPPED AREA

    //    printf("It is sropped %d %d %d %d %d\n", panoImageIsCropped(im),
    //	   panoImageHeight(im), panoImageWidth(im), panoImageFullHeight(im), panoImageFullWidth(im));


    // The Photoshop file has the dimensions of the full size image, regardless
    // of whether the TIFF file is "cropped" or not.
    // Layers of a PSD file can have the data constrained to only the cropped area, but that is not what we are doing here.
    // PSD file is limited to 30000 in width and height, PSB are limited to 300000 in width and height
    panoWriteINT32( fnum, panoImageHeight(im)); // Rows
    panoWriteINT32( fnum, panoImageWidth(im));  // Columns
            
    panoWriteSHORT( fnum, BitsPerChannel );           // BitsPerChannel
    
    switch( im->dataformat ) // Color mode
    {
        case _Lab:  panoWriteSHORT( fnum, 9 );
                    break;
        case _RGB:  panoWriteSHORT( fnum, 3 );
                    break;
        default:    panoWriteSHORT( fnum, 3 );            
    }

    //printf(".................HHHHHHHHHHHHHHHHHHHHHHHHHHHHH\n");

    panoWriteINT32( fnum, 0 );                // Color Mode block

    //    panoWriteINT32( fnum, 0);
    panoPSDResourcesBlockWrite(im, fnum);
    
    panoWriteINT32or64( fnum, 0, bBig );            // Layer & Mask 
    
    // Image data 
    
    writeImageDataPlanar( im,  fnum );

    myclose (fnum );
    return 0;
}


int writePSDwithLayer ( Image *im, fullPath *fname) 
{
  return writePSwithLayer( im, fname, FALSE);
}

// Save image as single layer PSD or PSB file
// Image is layer in front of white background
int writePSwithLayer(Image *im, fullPath *sfile, Boolean bBig )
{
    file_spec   fnum;
    int         BitsPerChannel;

    // Check to see if we need to create PSB instead of PSD file
    if(im->height > 30000 || im->width > 30000)
      bBig = TRUE;

    //Determine if this is a "cropped" tiff, and, if so, what is 
    //the full size from which this is cropped?
    
    // Jim Watters 2003/11/18: Photoshop CS does 16bit channels if 16 bit in, allow 16 bit out
    // TwoToOneByte( im ); // Multilayer image format doesn't support 16 bit channels
    
    GetBitsPerChannel( im, BitsPerChannel );
    
    if(  myopen( sfile, write_bin, fnum )   )
    {   
        PrintError("Error Writing Image File");
        return -1;
    }

    // Write PSD and PSB Header
    // panoWriteINT32( fnum, '8BPS' );
    panoWriteUCHAR( fnum, '8' );
    panoWriteUCHAR( fnum, 'B' );
    panoWriteUCHAR( fnum, 'P' );
    panoWriteUCHAR( fnum, 'S' );
    panoWriteSHORT( fnum, bBig? 2 : 1 ); // PSD file must be 1, PSB files must be 2
    panoWriteINT32( fnum, 0 ); panoWriteSHORT( fnum, 0 ); // 6 bytes zeroed

    panoWriteSHORT( fnum, 3 );            // No of channels; Background always white, 3 channels
        
    //panoWriteINT32( fnum, im->height );
    //panoWriteINT32( fnum, im->width  );
    
    //printf("It is cropped %d\n", panoImageIsCropped(im));

    //The Photoshop file has the dimensions of the full size image, regardless
    //of whether the TIFF file is "cropped" or not.
    // PSD file is limited to 30000 in width and height, PSB are limited to 300000 in width and height
    panoWriteINT32( fnum, panoImageFullHeight(im)); // Rows
    panoWriteINT32( fnum, panoImageFullWidth(im));  // Columns
            
    panoWriteSHORT( fnum, BitsPerChannel );        // BitsPerChannel
    switch( im->dataformat )
    {
        case _Lab:  panoWriteSHORT( fnum, 9 );
                    break;
        case _RGB:  panoWriteSHORT( fnum, 3 );
                    break;
        default:    panoWriteSHORT( fnum, 3 );            
    }

    panoWriteINT32( fnum, 0 );                // Color Mode

    panoPSDResourcesBlockWrite(im, fnum);

    writeLayerAndMask( im, fnum, bBig );

    writeWhiteBackground( im->width * (BitsPerChannel/8), im->height, fnum, bBig );
        
    myclose (fnum );

    return 0;
}


// Add image as additional layer into PSD-file (exported function)
int addLayerToFile( Image *im, fullPath* sfile, fullPath* dfile, stBuf *sB)
{
    file_spec   src;
    file_spec   fnum;
    Image       sim;            //  background image
    char        header[128], *h;
    size_t      count, i, srcCount = 0;
    pt_uint32   len;
    unsigned char **buf;
    int         BitsPerChannel, result = 0;
    Boolean     bBig = FALSE;

    // Jim Watters 2003/11/18: Photoshop CS does 16bit channels if 16 bit in, allow 16 bit out
    // TwoToOneByte( im ); // Multilayer image format doesn't support 16 bit channels
    
    GetBitsPerChannel( im, BitsPerChannel );
    

    if( myopen( sfile,read_bin, src ) )
    {   
        PrintError("Error Opening Image File");
        return -1;
    }
        
    // Read psd header

    h = header;
    count = PSDHLENGTH;
    
    myread( src,count,h); srcCount += count;
    if( count != PSDHLENGTH )
    {
        PrintError("Error Reading Image File");
        myclose( src );
        return -1;
    }
    
    if( ParsePSDHeader( header, &sim, &bBig ) != 0 )
    {
        PrintError("addLayerToFile: Wrong File Format");
        myclose( src );
        return -1;
    }
    
    // Check if image can be inserted
    //    printf("Image size %d %d im %d %d\n", sim.width, sim.height, panoImageWidth(im),  panoImageHeight(im));
    if( sim.width != panoImageFullWidth(im) || sim.height != panoImageFullHeight(im) )
    {   
        PrintError("Can't add layer: Images have different size");
        return -1;
    }
    
    // Read (and ignore) Color mode data
    panoReadINT32( src, &len ); 
    srcCount += (4 + len);
    count = 1;
    for( i=0; i<len; i++ )
    {
        myread(src,count,h);
    }

    // Read (and ingnore) Image resources
    panoReadINT32( src, &len ); 
    srcCount += (4 + len);
    count = 1;
    for( i=0; i<len; i++ )
    {
        myread(src,count,h);
    }

    myclose( src );

    if( myopen( sfile, read_bin, src ) )
    {   
        PrintError("Error Opening Image File");
        return -1;
    }

    if( myopen( dfile, write_bin, fnum ) )
    {   
        PrintError("Error Opening Image File");
        return -1;
    }

    // Read and write Fileheader
    buf = (unsigned char**)mymalloc( srcCount );
    if( buf == NULL )
    {
        PrintError("Not enough memory");
        result = -1;
        goto _addLayerToFile_exit;
    }
    fileCopy( src, fnum, srcCount, *buf );  
    myfree( (void**)buf );

    // Add one layer    
    if( addLayer( im,  src, fnum, sB, bBig ) != 0 )
    {
        result = -1;
        goto _addLayerToFile_exit;
    }

    
    writeWhiteBackground( sim.width * (BitsPerChannel/8), sim.height, fnum, bBig );
    

_addLayerToFile_exit:
    myclose( src ); myclose( fnum );
    return result;
}




static int writeImageDataPlanar( Image *im, file_spec fnum )
{
    register int            x,y,idy, bpp;
    unsigned char           **channel;
    register unsigned char  *ch, *idata;
    int                     color;
    size_t                  count;
    int                     BitsPerChannel, channels;
    
    GetBitsPerChannel( im, BitsPerChannel );
    GetChannels( im,channels);
    
    printf("Bitx per channel %d channels %d\n", BitsPerChannel, channels);

    bpp = im->bitsPerPixel / 8;
            


    // Write Compression info
    
    panoWriteSHORT( fnum, 0 ); // Raw data


    // Buffer to hold data in one channel
    
    count = (size_t)im->width * im->height * (BitsPerChannel / 8);
    
    channel = (unsigned char**)mymalloc( count );
    
    if( channel == NULL )
    {
        PrintError("Not Enough Memory");
        return -1;
    }

    if( BitsPerChannel ==  8 )
    {
        for( color = 0; color<3; color++)
        {
            ch = *channel; idata = &(*im->data)[color + channels - 3];
        
            for(y=0; y<im->height;y++)
            {
                idy = y * im->bytesPerLine;
                for(x=0; x<im->width;x++)
                {
                    *ch++ = idata [ idy + x * bpp ];
                }
            }
    
            mywrite( fnum, count, *channel );
        }
    }
    else // 16
    {
        unsigned short storage;//Kekus 2003 16 bit support
        for( color = 0; color<3; color++)
        {
            ch = *channel; idata = &(*im->data)[2*(color + channels - 3)];
        
            for(y=0; y<im->height;y++)
            {
                idy = y * im->bytesPerLine;
                for(x=0; x<im->width;x++)
                {
                    //Kekus:2003/Nov/18  // JMW 2004 July
                    storage = *(unsigned short*)&idata [ idy + x * bpp ];
                    SHORTNUMBER( storage, ch );
                    //Kekus.
                }
            }
    
            mywrite( fnum, count, *channel );
        }
    }
    
        

    if( im->bitsPerPixel == 32 )
    {
        // Write 1byte alpha channel
        
        ch = *channel; idata = &(*im->data)[0];
        
        for(y=0; y<im->height;y++)
        {
            idy = y * im->bytesPerLine;
            for(x=0; x<im->width;x++)
                {
                    *ch++ = idata [ idy + x * bpp ];
                }
        }
    
        mywrite( fnum, count, *channel );
    }
    else if( im->bitsPerPixel == 64 )
    {
        // Write 2byte alpha channel
        unsigned short storage;
        
        ch = *channel; idata = &(*im->data)[0];
        
        for(y=0; y<im->height;y++)
        {
            idy = y * im->bytesPerLine;
            for(x=0; x<im->width;x++)
            {
                storage = *(unsigned short*)&idata [ idy + x * bpp ];
                SHORTNUMBER( storage, ch );
            }
        }
    
        mywrite( fnum, count, *channel );
    }
    
    myfree( (void**)channel );
    return 0;
}


// Write white background, RLE-compressed

static void writeWhiteBackground( pt_int32 width, pt_int32 height, file_spec fnum, Boolean bBig  )
{
    pt_int32        w8, w;
    size_t          count;
    char           *d;
    char          **scanline;
    
    int numChannels = 3, i, bytecount, dim = height*numChannels;
    
    size_t maxscanline = (width/128)*2 + 2;
    scanline = (char**)mymalloc( maxscanline );
    if( scanline == NULL )
    {
        PrintError("Not enough memory");
        return;
    }

    
    panoWriteSHORT( fnum, 1 );    // RLE compressed

    w8 = width;
    
    d = *scanline;
    // Set up scanline
    for(w=w8; w>128; w-=128)
    {
        *d++ = -127;
        *d++ = 255U;
    }

    switch(w)
    {
        case 0: break;
        case 1: *d++ = 0;
                *d++ = 255U;
                break;
        default: *d++ = 1-(char)w;
                 *d++ = 255U;
                 break;
    }
    
    bytecount = (int)(d - *scanline);
    
    // Scanline counts (rows*channels)
    for(i=0; i < dim; i++)
    {
      if(bBig)
      {
        panoWriteINT32( fnum, bytecount );
      }
      else
      {
        panoWriteSHORT( fnum, bytecount );
      }
    }

    // RLE compressed data
    count = bytecount;
    for(i=0; i < dim; i++)
    {
        mywrite( fnum, count, *scanline );
    }
    myfree((void**)scanline);
    
}

// image is allocated, but not image data
// mode = 0: only load image struct
// mode = 1: also allocate and load data

int readPSD(Image *im, fullPath *sfile, int mode)
{
    file_spec   src;
    char    header[128], *h;
    ULONG   len, i;
    size_t  count;
    Boolean bBig = FALSE;


    if( myopen( sfile, read_bin, src ) )
    {   
        PrintError("Error Opening Image File");
        return -1;
    }
        
    // Read psd header

    h = header;
    count = PSDHLENGTH;
    
    myread( src,count,h);
    if( count != PSDHLENGTH )
    {
        PrintError("Error Reading Image File");
        myclose( src );
        return -1;
    }
    
    if( ParsePSDHeader( header, im, &bBig ) != 0 )
    {
        PrintError("readPSD: Wrong File Format");
        myclose( src );
        return -1;
    }

    if( mode == 0 )
    {
        myclose( src );
        return 0;
    }
    
    im->data = (unsigned char**) mymalloc( im->dataSize );
    if( im->data == NULL )
    {
        PrintError("Not enough memory to read image");
        myclose( src );
        return -1;
    }
    // Read (and ingnore) Color mode data
    panoReadINT32( src, &len );
    count = 1;
    for( i=0; i<len; i++ )
        myread(src,count,h);

    // Read (and ingnore) Image resources
    panoReadINT32( src, &len );
    count = 1;
    for( i=0; i<len; i++ )
        myread(src,count,h);
        
    // Read (and ingnore) Layer mask info
    panoReadINT32( src, &len );
    count = 1;
    for( i=0; i<len; i++ )
        myread(src,count,h);
        
    
    if( readImageDataPlanar( im, src ) != 0 )
    {
        PrintError("Error reading image");
        myclose( src );
        return -1;
    }
    
    myclose (src );
    return 0;
}

static int ParsePSDHeader( char *header, Image *im, Boolean *pbBig )
{
    register char *h = header;
    short s;
    int channels;

    if(pbBig == NULL)
    {
            PrintError( "ParsePSDHeader: Error pbBig is NULL");
            return -1;
    }

    *pbBig = FALSE; 
    
    if( *h++ != '8' || *h++ != 'B' || *h++ != 'P' || *h++ != 'S'  ||
        *h++ != 0   || (*h++ != 1 && *(h-1) != 2 ) ||
        *h++ != 0   || *h++ != 0   || *h++ != 0   || *h++ != 0   || *h++ != 0   || *h++ != 0 )
    {
            PrintError( "ParsePSDHeader: Error reading PSD Header: %c%c%c%c", header[0], header[1], header[2], header[3] );
            return -1;
    }

    if(header[5] == 2) 
      *pbBig = TRUE;
    
    NUMBERSHORT( s, h );
    
    channels = s;
    
    if( channels < 3 ) //!= 3 && channels != 4 )
    {
        PrintError( "Number of channels must be 3 or larger" );
        return -1;
    }
    if( channels > 4 ) channels = 4;
    
    NUMBERLONG( im->height, h );
    NUMBERLONG( im->width,  h );
    
    
    NUMBERSHORT( s, h );
    

    if( s!= 8 && s!= 16)
    {
        PrintError( "Depth must be 8 or 16 Bits per Channel" );
        return -1;
    }

    im->bitsPerPixel = s * channels;
    
    NUMBERSHORT( s, h );
    
    switch( s )
    {
        case 3:     im->dataformat = _RGB; break;
        case 9:     im->dataformat = _Lab; break;
        default:    PrintError( "Color mode must be RGB or Lab" );return -1;
    }
    
    im->bytesPerLine = im->width * (im->bitsPerPixel/8);
    im->dataSize = (size_t)im->height * im->bytesPerLine;
    
    return 0;
}


static int readImageDataPlanar(Image *im, file_spec src ) 
{
    register int            x,y,idy, bpp;
    unsigned char           **channel = NULL;
    register unsigned char  *h, *idata;
    int                     result = 0, i, chnum,BitsPerChannel, channels;
    size_t                  count;
    unsigned short          usvar;
    
    GetBitsPerChannel( im, BitsPerChannel );
    GetChannels( im, channels );
    bpp = im->bitsPerPixel / 8;


    // Read Compression info
    panoReadSHORT( src, &usvar );  
    if( usvar!= 0 )
    {
        PrintError("Image data must not be compressed");
        return -1;
    }
    
    // Allocate memory for one channel
    
    count = (size_t)im->width * im->height * (BitsPerChannel/8) ;
    channel = (unsigned char**)mymalloc( count );
    
    if( channel == NULL )
    {
        PrintError("Not Enough Memory");
        return -1;
    }

    
    for(i = 0; i < channels; i++)       // Read each channel
    {
        chnum = i + channels - 3; 
        if(chnum == 4) chnum = 0;   // Order: r g b (alpha)
        

        myread(src,count,*channel);
        if( count != (size_t)im->width * im->height * (BitsPerChannel/8))
        { 
            PrintError("Error Reading Image Data");
            result = -1;
            goto readImageDataPlanar_exit;
        }
        
        h = *channel; 
        
        if( BitsPerChannel == 8 )
        {
            idata = &(*im->data)[chnum];
            for(y=0; y<im->height;y++)
            {
                idy = y * im->bytesPerLine;
                for(x=0; x<im->width;x++)
                {
                    idata [ idy + x * bpp ] = *h++;
                }
            }
        }
        else // 16
        {
            idata = &(*im->data)[chnum*2];
            for(y=0; y<im->height;y++)
            {
                idy = y * im->bytesPerLine;
                for(x=0; x<im->width;x++)
                {
                    NUMBERSHORT( usvar, h );
                    *((unsigned short*)&idata [ idy + x * bpp ]) = usvar;
                }
            }
        }
    }
    
readImageDataPlanar_exit:
    if( channel != NULL )
        myfree( (void**)channel );
    return result;
}





// Write image as separate first layer

static int writeLayerAndMask( Image *im, file_spec fnum, Boolean bBig )
{
    PTRect          theRect;
    int64_t         lenLayerInfo;
    int64_t         channelLength;
    int             i, BitsPerChannel, channels, psdchannels;
    int             oddSized = 0;
    int             hasClipMask = 0;    // Create a mask
    int             hasShapeMask = 0;   // Create alpha channel

    GetBitsPerChannel( im, BitsPerChannel );
    GetChannels( im, channels );
    psdchannels = channels;

    if( channels > 3 ) // Alpha channel present
    {
        hasClipMask = 1;
        psdchannels++;
        if ( !hasFeather( im ) )// If there is feathering do Not use Alpha channel for Shape Mask
        {                       // Only use alpha if there is no feathering
            hasShapeMask = 1;   // The Alpha channel can be used as a Shape Mask
        }
    }

    //we only write to the PSD file the smallest region of the input file that 
    //contains "non-empty" image data.  The bounds of this region are recorded in
    //in "theRect".  If this input image is a "cropped" image, then theRect will
    //just use the data region specified "crop_info".
    getImageRectangle( im, &theRect );

    channelLength = ((int64_t)theRect.right-theRect.left) * (theRect.bottom-theRect.top) * (BitsPerChannel/8)  + 2;

    if(bBig)
    {
      lenLayerInfo = 2 + 4*4 + 2 + psdchannels * 10 + 4 + 4 + 4 * 1 + 4 + 12 + psdchannels * channelLength;
    }
    else
    {
      lenLayerInfo = 2 + 4*4 + 2 + psdchannels * 6 + 4 + 4 + 4 * 1 + 4 + 12 + psdchannels * channelLength;
    }
    
    if(hasClipMask)
        lenLayerInfo += 20;
    
    // Length of the layers info section, rounded up to a multiple of 2.
    if( lenLayerInfo%2 ) // odd
    {
        lenLayerInfo += 1; oddSized = 1;
    }

    panoWriteINT32or64( fnum, lenLayerInfo + 4 + (bBig?8:4), bBig );
    // Layer info. See table 2–13.

    panoWriteINT32or64( fnum, lenLayerInfo, bBig );
    
    // 2 bytes Count Number of layers. If <0, then number of layers is absolute value,
    // and the first alpha channel contains the transparency data for
    // the merged result.
    panoWriteSHORT( fnum, 1 );

    // ********** Layer Structure ********************************* //  
    panoWriteINT32( fnum, theRect.top );                  // Layer top 
    panoWriteINT32( fnum, theRect.left );                 // Layer left
    panoWriteINT32( fnum, theRect.bottom ) ;              // Layer bottom 
    panoWriteINT32( fnum, theRect.right  ) ;              // Layer right 

    panoWriteSHORT( fnum, psdchannels );                  // The number of channels in the layer.

    // ********** Channel information ***************************** //
    panoWriteSHORT( fnum, 0 );                            // red
    panoWriteINT32or64( fnum, channelLength, bBig );      // Length of following channel data.
    panoWriteSHORT( fnum, 1 );                            // green
    panoWriteINT32or64( fnum, channelLength, bBig );      // Length of following channel data.
    panoWriteSHORT( fnum, 2 );                            // blue
    panoWriteINT32or64( fnum, channelLength, bBig );      // Length of following channel data.
    if( hasClipMask )                                     // Mask channel
    {
        panoWriteSHORT( fnum, -1);                        // Shape Mask
        panoWriteINT32or64( fnum, channelLength, bBig );  // Length of following channel data.
        panoWriteSHORT( fnum, -2);                        // Clip Mask
        panoWriteINT32or64( fnum, channelLength, bBig );  // Length of following channel data.
    }

    // ********** End Channel information ***************************** //
    
    // panoWriteINT32( fnum, '8BIM'); // Blend mode signature Always 8BIM.
    panoWriteUCHAR( fnum, '8' );
    panoWriteUCHAR( fnum, 'B' );
    panoWriteUCHAR( fnum, 'I' );
    panoWriteUCHAR( fnum, 'M' );
    // panoWriteINT32( fnum, 'norm'); // Blend mode key
    panoWriteUCHAR( fnum, 'n' );
    panoWriteUCHAR( fnum, 'o' );
    panoWriteUCHAR( fnum, 'r' );
    panoWriteUCHAR( fnum, 'm' );

    panoWriteUCHAR( fnum, 255);                           // 1 byte Opacity 0 = transparent ... 255 = opaque
    panoWriteUCHAR( fnum, 0 );                            // 1 byte Clipping 0 = base, 1 = non–base
    panoWriteUCHAR( fnum, hasShapeMask );                 // 1 byte Flags bit 0 = transparency protected bit 1 = visible
    panoWriteUCHAR( fnum, 0 );                            // 1 byte (filler) (zero)

    if(hasClipMask)
    {
        panoWriteINT32( fnum, 32);                        // Extra data size Length of the extra data field. This is the total length of the next five fields.
        panoWriteINT32( fnum, 20 );                       // Yes Layer Mask data
        panoWriteINT32( fnum, theRect.top );              // Layer top 
        panoWriteINT32( fnum, theRect.left );             // Layer left
        panoWriteINT32( fnum, theRect.bottom ) ;          // Layer bottom 
        panoWriteINT32( fnum, theRect.right  ) ;          // Layer right 
        panoWriteUCHAR( fnum, 0 ) ;                       // Default color 0 or 255
        panoWriteUCHAR( fnum, 0 ) ;                       // Flag: bit 0 = position relative to layer bit 1 = layer mask disabled bit 2 = invert layer mask when blending
        panoWriteUCHAR( fnum, 0 ) ;                       // padding
        panoWriteUCHAR( fnum, 0 ) ;                       // padding
    }
    else
    {
        panoWriteINT32( fnum, 12);                        // Extra data size Length of the extra data field. This is the total length of the next five fields.
        panoWriteINT32( fnum, 0 );                        // No Layer Mask data
    }

    panoWriteINT32( fnum, 0 );                            // Layer blending ranges

    // Layer name  001
    panoWriteUCHAR( fnum, 3 );                            // length of name
    panoWriteUCHAR( fnum, '0' );
    panoWriteUCHAR( fnum, '0' );
    panoWriteUCHAR( fnum, '1' );
    
    
    // ************* End Layer Structure ******************************* //

    // ************* Write Channel Image data ************************** //

    // Write color channels
    for( i=0; i<3; i++)
    {
        if( writeChannelData( im, fnum, i + channels - 3,  &theRect ) ) 
            return -1;
    }
    if( hasShapeMask )
    {
        if( writeChannelData( im, fnum, 0,  &theRect ) ) 
            return -1;
    }
    else
    {
        if( writeTransparentAlpha(im, fnum, &theRect ) )
            return -1;
    }
    if( hasClipMask )
    {
        if( writeChannelData( im, fnum, 0,  &theRect ) ) 
            return -1;
    }
        
    if( oddSized )          // pad byte
    {
        panoWriteUCHAR( fnum, 0 );
    }

    // ************* End Write Channel Image data ************************** //
    
    // ************* Global layer mask info ******************************** //
    
    panoWriteINT32( fnum, 0 );        // Length of global layer mask info section.
    
    // panoWriteSHORT( fnum, 0 );     // 2 bytes Overlay color space
    // panoWriteSHORT( fnum, 0 );     // 4 * 2 byte color components
    // panoWriteSHORT( fnum, 0 );
    // panoWriteSHORT( fnum, 0 );
    // panoWriteSHORT( fnum, 0 );
    // panoWriteSHORT( fnum, 0 );     // 2 bytes Opacity 0 = transparent, 100 = opaque.
    // panoWriteUCHAR( fnum, 128 );   // 1 byte Kind 0=Color selected—i.e. inverted; 
                                      // 1=Color protected;128=use value stored per layer.
                                      //  This value is preferred. The others are for back-ward
                                      //  compatibility with beta versions.
    // panoWriteUCHAR( fnum, 0 );

    return 0;

}




static int writeChannelData( Image *im, file_spec fnum, int channel, PTRect *theRect )
{
    register int            x, y, idx, idy, bpp, BitsPerChannel, channels;
    unsigned char           **ch;
    register unsigned char  *c, *idata;
    size_t                  count;
    pt_int32                outputRegionWidth, outputRegionHeight;

    GetBitsPerChannel( im, BitsPerChannel );
    GetChannels( im, channels );
    
    // Write Compression info

    panoWriteSHORT( fnum, 0 );     // Raw data

    bpp = im->bitsPerPixel/8;

    count = ((size_t)theRect->right - theRect->left) * (theRect->bottom - theRect->top) * (BitsPerChannel/8);
    
    ch = (unsigned char**)mymalloc( count );
    
    if( ch == NULL )
    {
        PrintError("Not Enough Memory");
        return -1;
    }

    //note: theRect specifies the position in the output layer where the data should
    //be placed.  crop_info contains information about where the source data
    //is positioned relative to the full output size
    outputRegionWidth  = (theRect->right - theRect->left);
    outputRegionHeight = (theRect->bottom - theRect->top);
    
    if (outputRegionWidth > panoImageWidth(im) || outputRegionHeight > panoImageHeight(im)) 
    {
        printf("output region (%d x %d) is larger than input image data region (%d x %d)\n", 
			   (int)outputRegionWidth, (int)outputRegionHeight, (int)panoImageWidth(im), (int)panoImageHeight(im));
        return 1;
    }
    
    c = *ch; idata = &((*(im->data))[channel*(BitsPerChannel/8)]);
    
    if(BitsPerChannel == 8)
    {
        for(y=theRect->top; y<theRect->bottom;y++)
        {
            idy = (y - panoImageOffsetY(im)) * im->bytesPerLine;

            if (idy < 0) {  //should never happen
                PrintError("writeChannelData: index error");
                return 1;
            }
            
            for(x=theRect->left; x<theRect->right;x++)
            {
                idx = ((x - panoImageOffsetX(im)) * bpp);
                *c++ = idata [ idy + idx ];
            }
        }
    }
    else // 16
    {
        unsigned short storage;
        for(y=theRect->top; y<theRect->bottom;y++)
        {
            idy = (y - panoImageOffsetY(im)) * im->bytesPerLine;
            
            if (idy < 0) {  //should never happen
                PrintError("writeChannelData: index error");
                return 1;
            }
                        
            for(x=theRect->left; x<theRect->right;x++)
            {
                idx = ((x - panoImageOffsetX(im)) * bpp);            
                storage = *(unsigned short*)&idata [ idy + idx ];
                SHORTNUMBER( storage, c );
            }
        }
    }   
    mywrite( fnum, count, *ch );
    
        
    myfree( (void**)ch );
    return 0;
}


static int writeTransparentAlpha( Image *im, file_spec fnum, PTRect *theRect )
{
    register int            y, bpp, BitsPerChannel;
    size_t                  line, count;
    unsigned char           **ch;
    register unsigned char  *c;


    GetBitsPerChannel( im, BitsPerChannel );
    bpp = im->bitsPerPixel/8;
    line = ((size_t)theRect->right - theRect->left) * (BitsPerChannel/8);
    ch = (unsigned char**)mymalloc( line );
    if( ch == NULL )
    {
        PrintError("Not Enough Memory");
        return -1;
    }

    c = *ch;

    // Write Compression info
    panoWriteSHORT( fnum, 0 );     // Raw data

    // fill in line
    memset( c, 255, line );

    // write out the results
    for(y=theRect->top; y<theRect->bottom;y++)
    {
        count = line;
        mywrite( fnum, count, *ch );
    }
    
        
    myfree( (void**)ch );
    return 0;
}


// Return the smallest rectangle enclosing the image; 
// Use alpha channel and rgb data 
static void getImageRectangle( Image *im, PTRect *theRect )
{
    register unsigned char *alpha, *data;
    register int x,y,cy,bpp,channels;
    int BitsPerChannel;

    //If this is a cropped TIFF then we can get the image rectangle ROI from
    //the metadata in crop_info, rather than having to parse the entire
    //file to look for ROI
    if ( panoImageIsCropped(im)) {
        theRect->left   = panoImageOffsetX(im);
        theRect->top    = panoImageOffsetY(im);
        //Slightly counter-intuitive...right and bottom are one pixel larger than expected
        //so that the width and height can be calculated by subtracting
        //left from right or top from bottom
        theRect->right  = theRect->left + panoImageWidth(im);
        theRect->bottom = theRect->top  + panoImageHeight(im);
        
        return;
    }
    

    //the crop_info seems indicates that this isn't a cropped TIFF...we have
    //to parse the entire file to determine the ROI that contains non-empty
    //image data
    GetChannels( im, channels );
    GetBitsPerChannel( im, BitsPerChannel );
    bpp = im->bitsPerPixel/8;


    theRect->top            = im->height;
    theRect->left           = im->width;
    theRect->bottom         = 0;
    theRect->right          = 0;

    if( channels == 4 ){ //  alpha channel present: use image and alpha
        if( BitsPerChannel == 8 ){
            for(y=0; y<im->height; y++){
                for(x=0, alpha = *(im->data) + y * im->bytesPerLine; 
                    x<im->width; 
                    x++, alpha += 4){
                    data = alpha + 1;
                    if( *alpha || 
                        *(data) || *(data+1) || *(data+2) ){
                        if (y   < theRect->top)    theRect->top = y;
                        if (y+1 > theRect->bottom) theRect->bottom = y+1;
                        if (x   < theRect->left)   theRect->left = x;
                        if (x+1 > theRect->right)  theRect->right = x+1;
                    }
                }
            }
        }else{ // 16
            for(y=0; y<im->height; y++){
                for(x=0, alpha = *(im->data) + y * im->bytesPerLine; 
                    x<im->width; 
                    x++, alpha += 8){
                    data = alpha + 2;
                    if( *((USHORT*)alpha) || 
                        *((USHORT*)data) || *(((USHORT*)data)+1) || *(((USHORT*)data)+2) ){
                        if (y   < theRect->top)    theRect->top = y;
                        if (y+1 > theRect->bottom) theRect->bottom = y+1;
                        if (x   < theRect->left)   theRect->left = x;
                        if (x+1 > theRect->right)  theRect->right = x+1;
                    }
                }
            }
        }
    }
    else //  no alpha channel: Use non black rectangle only
    {
        alpha       = *(im->data);

        if( BitsPerChannel == 8 )
        {
            for(y=0; y<im->height; y++)
            {
                cy = y * im->bytesPerLine;
                for(x=0; x<im->width; x++)
                {
                    data = alpha + cy + bpp*x;
                    if( *data || *(data+1) || *(data+2) )
                    {
                        if (y   < theRect->top)    theRect->top = y;
                        if (y+1 > theRect->bottom) theRect->bottom = y+1;
                        if (x   < theRect->left)   theRect->left = x;
                        if (x+1 > theRect->right)  theRect->right = x+1;
                    }
                }
            }
        }
        else // 16
        {
            for(y=0; y<im->height; y++)
            {
                cy = y * im->bytesPerLine;
                for(x=0; x<im->width; x++)
                {
                    data = alpha + cy + bpp*x;
                    if( *((USHORT*)data) || *(((USHORT*)data)+1) || *(((USHORT*)data)+2) )
                    {
                        if (y   < theRect->top)    theRect->top = y;
                        if (y+1 > theRect->bottom) theRect->bottom = y+1;
                        if (x   < theRect->left)   theRect->left = x;
                        if (x+1 > theRect->right)  theRect->right = x+1;
                    }
                }
            }
        }
    }
    if (theRect->bottom <= theRect->top) { // no valid pixels found
        theRect->top            = 0;
        theRect->left           = 0;
        theRect->bottom         = im->height;
        theRect->right          = im->width;
    }

    //printf("Image Rectangle = %d,%d - %d,%d\n", theRect->left, theRect->top, theRect->right, theRect->bottom);
#if 0
    { char msg[1000];
      sprintf(msg,"width=%d, height=%d, top = %d, bottom = %d, left = %d, right = %d\n",
              im->width, im->height, theRect->top, theRect->bottom, theRect->left, theRect->right);
      PrintError(msg);
    }
#endif
}       



                                    
// Image is added to layer structure of file.
// There must be one valid layer structure, and the
// filepointer is at the beginning of it in both src and dest

static int addLayer( Image *im, file_spec src, file_spec fnum, stBuf *sB, Boolean bBig )
{
    pt_uint32       var;
    int64_t         var64;
    PTRect          theRect, *nRect = NULL;
    int             bpp, oddSized = 0, oddSizedOld = 0;
    int64_t         channelLength, lenLayerInfo;
    int             result = 0;
    size_t          count;
    char            sLayerName[5];
    unsigned char   ch;
    int             i, channels, psdchannels;
    SHORT           numLayers;
    ULONG           k;
    unsigned int    BitsPerChannel;
    USHORT         *uChannel = NULL;
    USHORT         *chid = NULL;
    ULONG          *uFlag = NULL;
    ULONG          *uExtra = NULL;
    ULONG          *uMask = NULL;
    ULONG          *uMaskData = NULL;
    ULONG          *cNames = NULL; //used to store a 4 character name
    int64_t        *chlength = NULL;
    unsigned char   **alpha = NULL, **buf = NULL;
    int             hasClipMask = 0;    // Create a mask
    int             hasShapeMask = 0;   // Create alpha channel
    char          * blendingModeKey;
    int             j;

    GetChannels( im, channels );
    psdchannels = channels;
    GetBitsPerChannel( im, BitsPerChannel );
    bpp = im->bitsPerPixel/8;
    
    if( channels > 3 ) // Alpha channel present
    {
        hasClipMask = 1;
        psdchannels++;
        if ( !hasFeather( im ) )// If there is feathering do Not use Alpha channel for Shape Mask
        {                       // Only use alpha if there is no feathering
            hasShapeMask = 1;   // The Alpha channel can be used as a Shape Mask
        }
        if( sB->seam == _middle )   // we have to stitch
        {
            alpha = (unsigned char**) mymalloc( (size_t)im->width * im->height * (BitsPerChannel/8));
        }
    }

    if( alpha != NULL )
    {
        memset( *alpha, 0 , (size_t)im->width * im->height * (BitsPerChannel/8));
    }

    getImageRectangle( im,  &theRect );
    channelLength = ((size_t)theRect.right-theRect.left) * (theRect.bottom-theRect.top)  * (BitsPerChannel/8) + 2;
    
    
    // Read layerinfo up to channeldata
    panoReadINT32or64( src, &var64, bBig );           // Length of the miscellaneous information section (ignored)
    panoReadINT32or64( src, &var64, bBig );           // Length of the layers info section, rounded up to a multiple of 2(ignored)

    panoReadSHORT( src, (USHORT *)&numLayers );     // Number of layers If it is a negative number, its absolute value is the number of layers and the first alpha channel contains the transparency data for the merged result.
    lenLayerInfo = 2;

    chlength = (int64_t*) malloc( numLayers * 8);
    uChannel = (USHORT*) malloc( numLayers * sizeof( USHORT ));
    cNames   = (ULONG*) malloc( numLayers * sizeof( ULONG ));
    nRect    = (PTRect*) malloc( numLayers * sizeof( PTRect ));
    uExtra   = (ULONG*) malloc( numLayers * sizeof( ULONG ));
    uMask    = (ULONG*) malloc( numLayers * sizeof( ULONG ));
    uMaskData= (ULONG*) malloc( numLayers * 5 * sizeof( ULONG ));
    chid     = (USHORT*) malloc( numLayers * 5 * sizeof( USHORT ));// five posible channels
    uFlag    = (ULONG*) malloc( numLayers * 5 * sizeof( ULONG ));
    if( chlength == NULL || uChannel== NULL || nRect == NULL ||
        cNames == NULL || uExtra == NULL || uMask == NULL || 
        uMaskData == NULL || chid == NULL || uFlag == NULL )
    {
        PrintError("Not enough memory 1");
        result = -1;
        goto _addLayer_exit;
    }

    for(i=0; i<numLayers; i++) 
    {
        panoReadINT32( src, (ULONG *)&nRect[i].top   ) ;  // Layer top 
        panoReadINT32( src, (ULONG *)&nRect[i].left  ) ;  // Layer left
        panoReadINT32( src, (ULONG *)&nRect[i].bottom) ;  // Layer bottom 
        panoReadINT32( src, (ULONG *)&nRect[i].right ) ;  // Layer right 

        panoReadSHORT( src, &uChannel[i] );               // The number of channels in the layer.

        // ********** Channel information ***************************** //

        panoReadSHORT( src, &chid[i*5 +0]);               // red
        panoReadINT32or64( src, &chlength[i], bBig);      // Length of following channel data.
        panoReadSHORT( src, &chid[i*5 +1] );              // green
        panoReadINT32or64( src, &chlength[i], bBig);      // Length of following channel data.
        panoReadSHORT( src, &chid[i*5 +2] );              // blue
        panoReadINT32or64( src, &chlength[i], bBig);      // Length of following channel data.
        if( uChannel[i] > 3 )                             // 1st alpha channel
        {
            panoReadSHORT( src, &chid[i*5 +3] );          // transparency mask
            panoReadINT32or64( src, &chlength[i], bBig);  // Length of following channel data.
        }
        if( uChannel[i] > 4 )                             // 2nd alpha channel
        {
            panoReadSHORT( src, &chid[i*5 +4] );          // transparency mask
            panoReadINT32or64( src, &chlength[i], bBig);  // Length of following channel data.
        }

        // ********** End Channel information ***************************** //
    
        panoReadINT32( src, &var );                       // Blend mode signature Always 8BIM.
        panoReadINT32( src, &var );                       // Blend mode key

        panoReadINT32( src, &uFlag[i] );                  // Four flag bytes: Opacity, clipping, flag, filler

        panoReadINT32( src, &uExtra[i] );                 // Extra data size Length of the extra data field. This is the total length of the next five fields.
        panoReadINT32( src, &uMask[i] );                  // Layer Mask data length
        if(uMask[i])
            for(k=0; k<uMask[i]/4; k++)                   // either 0 or 20
            {
                panoReadINT32( src, &uMaskData[i*5 +k] );  // Layer Mask Data
            }
        panoReadINT32( src, &var );                       // Layer blending ranges

        panoReadINT32( src, &cNames[i] );                 // Layer name.  Panotools only uses 4 bytes to make 3 char name.

        if(bBig)
        {
          var64 = 4*4 + 2 + uChannel[i] * 10 + 4 + 4 + 4 * 1 + 4 + 4 + uMask[i] + 8 + uChannel[i] * chlength[i]; // length
        }
        else
        {
          var64 = 4*4 + 2 + uChannel[i] * 6 + 4 + 4 + 4 * 1 + 4 + 4 + uMask[i] + 8 + uChannel[i] * chlength[i]; // length
        }
        lenLayerInfo += var64;
    }

    if( 1 == lenLayerInfo%2 ) //odd
    {
        oddSizedOld = 1;
    }
    
    // length of new channel
    if(bBig)
    {
      var64 = 4*4 + 2 + psdchannels * 10 + 4 + 4 + 4 * 1 + 4 + 12 + psdchannels * channelLength;
    }
    else
    {
      var64 = 4*4 + 2 + psdchannels * 6 + 4 + 4 + 4 * 1 + 4 + 12 + psdchannels * channelLength;
    }
    
    if(hasClipMask)
        var64 += 20;

    lenLayerInfo += var64;

    // Length of the layers info section, rounded up to a multiple of 2.
    if( 1 == lenLayerInfo%2 ) // odd
    {
        oddSized = 1; lenLayerInfo++;
    }
    
    // Length of the layers info section, rounded up to a multiple of 2.
    panoWriteINT32or64( fnum, lenLayerInfo + 4 + (bBig?8:4), bBig );

    panoWriteINT32or64( fnum, lenLayerInfo, bBig );


    // 2 bytes Count Number of layers. If <0, then number of layers is absolute value,
    // and the first alpha channel contains the transparency data for
    // the merged result.
    panoWriteSHORT( fnum, numLayers + 1); 
    // Write previous layers, read channel data 
    for(i=0; i<numLayers; i++) 
    {
        panoWriteINT32( fnum, nRect[i].top    );          // Layer top 
        panoWriteINT32( fnum, nRect[i].left   );          // Layer left
        panoWriteINT32( fnum, nRect[i].bottom );          // Layer bottom 
        panoWriteINT32( fnum, nRect[i].right  );          // Layer right 

        panoWriteSHORT( fnum, uChannel[i] );      // The number of channels in the layer.

        // ********** Channel information ***************************** //
        
        panoWriteSHORT( fnum, chid[i*5 +0] );             // red
        panoWriteINT32or64( fnum, chlength[i], bBig );    // Length of following channel data.
        panoWriteSHORT( fnum, chid[i*5 +1] );             // green
        panoWriteINT32or64( fnum, chlength[i], bBig );    // Length of following channel data.
        panoWriteSHORT( fnum, chid[i*5 +2] );             // blue
        panoWriteINT32or64( fnum, chlength[i], bBig );    // Length of following channel data.
        if( uChannel[i] > 3 )                             // 1st alpha channel
        {
            panoWriteSHORT( fnum, chid[i*5 +3] );         // channel ID
            panoWriteINT32or64( fnum, chlength[i], bBig );// Length of following channel data.
        }
        if( uChannel[i] > 4 )                             // 2nd alpha channel
        {
            panoWriteSHORT( fnum, chid[i*5 +4] );         // channel ID
            panoWriteINT32or64( fnum, chlength[i], bBig );// Length of following channel data.
        }
        // ********** End Channel information ***************************** //
    
        // panoWriteINT32( fnum, '8BIM'); // Blend mode signature Always 8BIM.
        panoWriteUCHAR( fnum, '8' );
        panoWriteUCHAR( fnum, 'B' );
        panoWriteUCHAR( fnum, 'I' );
        panoWriteUCHAR( fnum, 'M' );
        // panoWriteINT32( fnum, 'norm'); // Blend mode key
        if (i == 0) {
            //use norm
            panoWriteUCHAR( fnum, 'n' );
            panoWriteUCHAR( fnum, 'o' );
            panoWriteUCHAR( fnum, 'r' );
            panoWriteUCHAR( fnum, 'm' );
        } else {
            // use the desired one. 
            // XXX TODO: read the blending more from the layer so we can write it the same way back
            assert(sizeof(psdBlendingModesNames) == sizeof(psdBlendingModesInternalName));
            blendingModeKey = psdBlendingModesInternalName[sB->psdBlendingMode];
            for (j = 0; j< 4; j++ ) {
              panoWriteUCHAR( fnum, blendingModeKey[j]);
            }
        }

        panoWriteINT32( fnum, uFlag[i] );       // Opacity, clipping, flag, filler
        panoWriteINT32( fnum, uExtra[i] );      // Extra data size Length of the extra data field. This is the total length of the next five fields.
        panoWriteINT32( fnum, uMask[i] );       // Layer Mask data
        if(uMask[i])
            for(k=0; k<uMask[i]/4; k++)     // either 0 or 20
            {
                panoWriteINT32( fnum, uMaskData[i*5 +k] );    // Layer Mask Data
            }

        panoWriteINT32( fnum, 0 );        // Layer blending ranges

        panoWriteINT32( fnum, cNames[i] );        // Layer name
        
    }
    // ************** The New Layer ************************************ //
    // ********** Layer Structure ********************************* //  

    //  PrintError("Adding layer : top %d, left %d, bottom %d, right %d, size %d\n",  
    //  theRect.top, theRect.left, theRect.bottom, theRect.right, channelLength);


    panoWriteINT32( fnum, theRect.top );                  // Layer top 
    panoWriteINT32( fnum, theRect.left );                 // Layer left
    panoWriteINT32( fnum, theRect.bottom ) ;              // Layer bottom 
    panoWriteINT32( fnum, theRect.right  ) ;              // Layer right 

    panoWriteSHORT( fnum, psdchannels );                  // The number of channels in the layer.

    // ********** Channel information ***************************** //
        
    panoWriteSHORT( fnum, 0 );                            // red
    panoWriteINT32or64( fnum, channelLength, bBig );      // Length of following channel data.
    panoWriteSHORT( fnum, 1 );                            // green
    panoWriteINT32or64( fnum, channelLength, bBig );      // Length of following channel data.
    panoWriteSHORT( fnum, 2 );                            // blue
    panoWriteINT32or64( fnum, channelLength, bBig );      // Length of following channel data.
    if( hasClipMask )                                     // Mask channel
    {
        panoWriteSHORT( fnum, -1);                        // Shape Mask
        panoWriteINT32or64( fnum, channelLength, bBig );  // Length of following channel data.
        panoWriteSHORT( fnum, -2);                        // Clip Mask
        panoWriteINT32or64( fnum, channelLength, bBig );  // Length of following channel data.
    }


    // ********** End Channel information ***************************** //
    
    // panoWriteINT32( fnum, '8BIM'); // Blend mode signature Always 8BIM.
    panoWriteUCHAR( fnum, '8' );
    panoWriteUCHAR( fnum, 'B' );
    panoWriteUCHAR( fnum, 'I' );
    panoWriteUCHAR( fnum, 'M' );
    
    // the next 4 bytes are the blending mode key
    /* panoWriteINT32( fnum, 'norm'); // Blend mode key */
    assert(PSD_NUMBER_BLENDING_MODES == sizeof(psdBlendingModesNames)/sizeof(char*));
    assert(sizeof(psdBlendingModesNames) == sizeof(psdBlendingModesInternalName));
    blendingModeKey = psdBlendingModesInternalName[sB->psdBlendingMode];
    for (j = 0; j< 4; j++ )
    {
      panoWriteUCHAR( fnum, blendingModeKey[j]);
    }

    panoWriteUCHAR( fnum, sB->psdOpacity);               // 1 byte Opacity 0 = transparent ... 255 = opaque
    panoWriteUCHAR( fnum, 0 );                          // 1 byte Clipping 0 = base, 1 = non–base
    panoWriteUCHAR( fnum, hasShapeMask );               // 1 byte Flags bit 0 = transparency protected bit 1 = visible
    panoWriteUCHAR( fnum, 0 );                          // 1 byte (filler) (zero)

    if(hasClipMask)
    {
        panoWriteINT32( fnum, 32);                      // Extra data size Length of the extra data field. This is the total length of the next five fields.
        panoWriteINT32( fnum, 20 );                     // Yes Layer Mask data
        panoWriteINT32( fnum, theRect.top );            // Layer top 
        panoWriteINT32( fnum, theRect.left );           // Layer left
        panoWriteINT32( fnum, theRect.bottom ) ;        // Layer bottom 
        panoWriteINT32( fnum, theRect.right  ) ;        // Layer right 
        panoWriteUCHAR( fnum, 0 ) ;                     // Default color 0 or 255
        panoWriteUCHAR( fnum, 0 ) ;                     // Flag: bit 0 = position relative to layer bit 1 = layer mask disabled bit 2 = invert layer mask when blending
        panoWriteUCHAR( fnum, 0 ) ;                     // padding
        panoWriteUCHAR( fnum, 0 ) ;                     // padding
    }
    else
    {
        panoWriteINT32( fnum, 12);                      // Extra data size Length of the extra data field. This is the total length of the next five fields.
        panoWriteINT32( fnum, 0 );                      // No Layer Mask data
    }
    panoWriteINT32( fnum, 0 );                          // Layer blending ranges
    
    sprintf(&(sLayerName[1]), "%03.3d", numLayers+1 ); 
    sLayerName[0] = 3;
    count = 4; 
    mywrite( fnum, count, sLayerName ); // Layer Name
    
    
    // ************* End Layer Structure ******************************* //

    // ************* Write Channel Image data ************************** //

    for( i=0; i< numLayers; i++)
    {
        buf = (unsigned char**) mymalloc( chlength[i] );
        
        if( buf == NULL )
        {
            PrintError("Not enough memory, %d bytes needed", chlength[i]);
            result = -1;
            goto _addLayer_exit;
        }
        for( k=0; k< uChannel[i]; k++ )
        {
            fileCopy( src, fnum, chlength[i], *buf );
            //printf("Compression Layer %d Channel %d: %d\n", i,k,(int)*((short*)*buf));
            
            if( chid[i*5 +k] == -1 )    // found an alpha channel
            {
                if( alpha!= NULL )
                {
                    orAlpha( *alpha, &((*buf)[2]), im, &(nRect[i]) );
                }
            }
        }
        myfree( (void**)buf );
    }

    if( 1 == oddSizedOld%2 )    // If an odd number of odd layers then there is a single pad byte
    {
        panoReadUCHAR( src, &ch );
    }
    
    // Write color channels
    for( i=0; i<3; i++)
    {
        if( writeChannelData( im, fnum, i + channels - 3, &theRect ) ) 
        {
            result = -1;
            goto _addLayer_exit;
        }
    }
    
    if( hasShapeMask )  // Alpha channel present
    {       
        if( writeChannelData( im, fnum, 0,  &theRect ) ) 
        {
            result = -1;
            goto _addLayer_exit;
        }
    }
    else
    {
        if( writeTransparentAlpha(im, fnum, &theRect ) )
        {
            result = -1;
            goto _addLayer_exit;
        }
    }

    if( hasClipMask )
    {
        if( sB->seam == _middle && alpha != NULL ) // Create stitching mask
        {
            mergeAlpha( im, *alpha, sB->feather, &theRect );
#if 0
            {
                static nim = 0;
                fullPath fp;
                makePathToHost( &fp );
                sprintf( (char*)fp.name, "Test.%d", nim++);
                PrintError("creating file: %s", fp.name);
                c2pstr((char*)fp.name);
                mycreate( &fp, '8BIM', '8BPS' );
                 writePSD(im, &fp );
            }
#endif
        }
        if( writeChannelData( im, fnum, 0,  &theRect ) ) 
        {
            result = -1;
            goto _addLayer_exit;
        }
    }
    if( oddSized )  // pad byte
    {
        panoWriteUCHAR( fnum, 0 );
    }
        
    // ************* End Write Channel Image data ************************** //
    
    // ************* Global layer mask info ******************************** //
    
    panoReadINT32( src, &var );                 // Length of global layer mask info section.
    panoWriteINT32( fnum, 0 );       
    //panoReadSHORT( src, &svar );    panoWriteSHORT( fnum, 0 );        // 2 bytes Overlay color space
    //panoReadSHORT( src, &svar );    panoWriteSHORT( fnum, 0 );        // 4 * 2 byte color components
    //panoReadSHORT( src, &svar );    panoWriteSHORT( fnum, 0 );        
    //panoReadSHORT( src, &svar );    panoWriteSHORT( fnum, 0 );        
    //panoReadSHORT( src, &svar );    panoWriteSHORT( fnum, 0 );
    //panoReadSHORT( src, &svar );    panoWriteSHORT( fnum, 0 );        // 2 bytes Opacity 0 = transparent, 100 = opaque.
    //panoReadSHORT( src, &svar );    panoWriteUCHAR( fnum, 128 );      // 1 byte Kind 0=Color selected—i.e. inverted; 
                                                // 1=Color protected;128=use value stored per layer.
                                                //  This value is preferred. The others are for back-ward
                                                // compatibility with beta versions.
                        //panoWriteUCHAR( fnum, 0 );
    
    
_addLayer_exit: 
    if( alpha       != NULL )   myfree( (void**)alpha );
    if( chlength    != NULL )   free(chlength); 
    if( uChannel    != NULL )   free(uChannel);
    if( nRect       != NULL )   free(nRect);
    if( cNames      != NULL )   free(cNames);
    if( uExtra      != NULL )   free(uExtra);
    if( uMask       != NULL )   free(uMask);
    if( uMaskData    != NULL )   free(uMaskData);
    if( chid        != NULL )   free(chid);
    if( uFlag       != NULL )   free(uFlag);

    return result;
}


static int fileCopy( file_spec src, file_spec dest, size_t numBytes, unsigned char *buf )
{
    size_t count = numBytes;
    
    myread(  src, count, buf );
    mywrite( dest, count, buf );
    return 0;
}


// Or two alpha channels: one in alpha (same size as image im)
// one in buf comprising the rectangle top,bottom,left,right
// store result in alpha

static void orAlpha( unsigned char* alpha, unsigned char *buf, Image *im, PTRect *theRect )
{
    register int x,y,ay,by,w;
    int BitsPerChannel;
    
    GetBitsPerChannel( im, BitsPerChannel );

    if( im->bitsPerPixel != 32 && im->bitsPerPixel != 64 && im->bitsPerPixel != 128) return;
    
    w = (theRect->right - theRect->left);
    
    if( BitsPerChannel == 8 )
    {
        for(y=theRect->top; y<theRect->bottom; y++)
        {
            ay = y * im->width;
            by = (y - theRect->top) * w;
            {
                for(x=theRect->left; x<theRect->right; x++)
                {
                    if( buf[ by + x - theRect->left ] )
                        alpha[ ay + x ] = 255U;
                }
            }
        }
    }
    if( BitsPerChannel == 16 )
    {
        for(y=theRect->top; y<theRect->bottom; y++)
        {
            ay = y * im->width * 2;
            by = (y - theRect->top) * w * 2;
            {
                for(x=theRect->left; x<theRect->right; x++)
                {
                    if( *((USHORT*)(buf+ by + 2*(x - theRect->left))) )
                        *((USHORT*)(alpha + ay + 2*x) ) = 65535U;
                }
            }
        }
    }
    if( BitsPerChannel == 32 )
    {
        for(y=theRect->top; y<theRect->bottom; y++)
        {
            ay = y * im->width * 2;
            by = (y - theRect->top) * w * 2;
            {
                for(x=theRect->left; x<theRect->right; x++)
                {
                    if( *((float*)(buf+ by + 2*(x - theRect->left))) )
                        *((float*)(alpha + ay + 2*x) ) = 1.0;
                }
            }
        }
    }
}



void FindScript( aPrefs *thePrefs )
{
    FindFile( &(thePrefs->scriptFile) );
}

int LoadOptions( cPrefs * thePrefs )
{   
    fullPath path;
    file_spec fnum;
    struct correct_Prefs loadPrefs;
    size_t  count;
    int result;
    
    if( FindFile( &path ) )
    {
        return -1;
    }
    
    if( myopen( &path, read_bin, fnum ))
    {
        PrintError("Could not open file");
        return -1;
    }
    
    count = sizeof( struct correct_Prefs);
    myread(  fnum, count, &loadPrefs );

    if( count != sizeof( struct correct_Prefs) || loadPrefs.magic != 20 )
    {
        PrintError( "Wrong format!");
        result = -1;
    }
    else
    {
        memcpy((char*) thePrefs, (char*)&loadPrefs, sizeof(struct correct_Prefs));
        result = 0;
    }
    myclose(fnum);

    return result;
}


char* LoadScript( fullPath* scriptFile )
{
    fullPath    sfile;
    int         i;
    file_spec   fnum;
    size_t      count;
    char        *script = NULL, ch;
    
    memset( &sfile, 0, sizeof( fullPath ) );
    if( memcmp( scriptFile, &sfile, sizeof( fullPath ) ) == 0 )
    {
        PrintError("No Scriptfile selected");
        goto _loadError;
    }

    if( myopen( scriptFile, read_text, fnum ))
    {
        PrintError("Error Opening Scriptfile: %s", scriptFile->name);
        goto _loadError;
    }
    
    count = 1; i=0; // Get file length
    
    while( count == 1 )
    {
        myread(  fnum, count, &ch ); 
        if(count==1) i++;
    }
    myclose(fnum);

    count = i;
    script = (char*)malloc( count+1 );
    if( script == NULL )
    {
        PrintError("Not enough memory to load scriptfile");
        goto _loadError;
    }
    if( myopen( scriptFile, read_text, fnum ))
    {
        PrintError("Error Opening Scriptfile: %s", scriptFile->name);
        free(script);
        goto _loadError;
    }
    
    
    myread(fnum,count,script);
    script[count] = 0;
    myclose(fnum);
    return script;
    
_loadError:
    return (char*)NULL;
}


int WriteScript( char* res, fullPath* scriptFile, int launch )
{
    fullPath    sfile;
    file_spec   fnum;
    size_t      count;

    memset( &sfile, 0, sizeof( fullPath ) );
    if( memcmp( scriptFile, &sfile, sizeof( fullPath ) ) == 0 )
    {
        PrintError("No Scriptfile selected");
        goto _writeError;
    }

    memcpy(  &sfile, scriptFile, sizeof (fullPath) );
    mydelete( &sfile );
    mycreate(&sfile,'ttxt','TEXT');

    if( myopen( &sfile, write_text, fnum ) )
    {
        PrintError("Error Opening Scriptfile");
        goto _writeError;
    }
    
    count = strlen( res );
    mywrite( fnum, count, res );    
    myclose (fnum );

    if( launch == 1 )
    {
        showScript( &sfile);
    }
    return 0;
    

_writeError:
    return -1;
}


void SaveOptions( cPrefs * thePrefs )
{
    fullPath    path;
    file_spec   fspec;
    size_t      count;
    
    memset( &path, 0, sizeof( fullPath ));
    if( SaveFileAs( &path, "Save Settings as..", "Params" ) )
        return;
        
    mycreate    (&path,'GKON','TEXT');
    
    if( myopen( &path, write_bin, fspec ) )
        return;
    
    count = sizeof( cPrefs );
    mywrite( fspec, count, thePrefs );
    myclose( fspec );
}



// Temporarily save a 32bit image (including Alpha-channel) using name fname

int SaveBufImage( Image *image, char *fname )
{
    
    fullPath        fspec;

    MakeTempName( &fspec, fname );


    mydelete( &fspec );
    mycreate( &fspec, '8BIM', '8BPS');
    
    return writePSD(image,  &fspec);
}

// Load a saved 32bit image (including Alpha-channel) using name fname
// image must be allocated (not data)
// mode = 0: only load image struct
// mode = 1: also allocate and load data
    
    
int LoadBufImage( Image *image, char *fname, int mode)
{
    fullPath    fspec;

    MakeTempName( &fspec, fname );
    return readPSD(image, &fspec,  mode);
}


// Read Photoshop Multilayer-image
int readPSDMultiLayerImage( MultiLayerImage *mim, fullPath* sfile){
    file_spec       src;

    char            header[128], *h;
    int64_t         chlength;
    size_t          count, iul, kul;
    pt_uint32       var;
    int64_t         var64;
    USHORT          svar;
    int             i, k, result = 0, odd = 0;
    USHORT          uChannel;
    unsigned char **buf = NULL, ch;
    Image           im;
    int             BitsPerSample = 8;
    Boolean         bBig = FALSE;
    
    SetImageDefaults( &im );

    if( myopen( sfile,read_bin, src ) ){    
        PrintError("Error Opening Image File");
        return -1;
    }
        
    // Read psd header

    h   = header;
    count   = PSDHLENGTH;
    myread( src,count,h); 
    if( count != PSDHLENGTH ){
        PrintError("Error Reading Image Header");
        myclose( src );
        return -1;
    }
    
    if( ParsePSDHeader( header, &im, &bBig ) != 0 ){
        PrintError("readPSDMultiLayerImage: Wrong File Format");
        myclose( src );
        return -1;
    }
    
    // Read (and ignore) Color mode data
    panoReadINT32( src, &var ); 
    count = 1;
    for( iul=0; iul<var; iul++ )
    {
        myread(src,count,h);
    }

    // Read (and ingnore) Image resources
    panoReadINT32( src, &var ); 
    count = 1;
    for( iul=0; iul<var; iul++ )
    {
        myread(src,count,h);
    }

    // Read the layers

    // Read layerinfo up to channeldata
    panoReadINT32or64( src, &var64, bBig );               // Length of info section(ignored)
    panoReadINT32or64( src, &var64, bBig );               // Length of layer info (ignored)

    panoReadSHORT( src, &svar );                          // Number of layers
    mim->numLayers = svar;
    
    mim->Layer  = (Image*)  malloc( mim->numLayers * sizeof( Image ) );
    
    if( mim->Layer == NULL )
    {
        PrintError("Not enough memory");
        result = -1;
        goto readPSDMultiLayerImage_exit;
    }
    for(i=0; i<mim->numLayers; i++) 
    {
        SetImageDefaults( &mim->Layer[i] );
        mim->Layer[i].width  = im.width;
        mim->Layer[i].height = im.height;
        panoReadINT32( src, &mim->Layer[i].selection.top )    ;  // Layer top 
        panoReadINT32( src, &mim->Layer[i].selection.left  ); // Layer left
        panoReadINT32( src, &mim->Layer[i].selection.bottom); // Layer bottom 
        panoReadINT32( src, &mim->Layer[i].selection.right ); // Layer right 
        
        panoReadSHORT( src, &uChannel );                              // The number of channels in the layer.
        mim->Layer[i].bitsPerPixel = uChannel * BitsPerSample;

        mim->Layer[i].bytesPerLine = (mim->Layer[i].selection.right - mim->Layer[i].selection.left) *
                        mim->Layer[i].bitsPerPixel/8;
        mim->Layer[i].dataSize = mim->Layer[i].bytesPerLine * 
                     (mim->Layer[i].selection.bottom - mim->Layer[i].selection.top);
        mim->Layer[i].data = (unsigned char**) mymalloc((size_t)(mim->Layer[i].dataSize));
        if( mim->Layer[i].data == NULL )
        {
            PrintError("Not enough memory");
            result = -1;
            goto readPSDMultiLayerImage_exit;
        }
        
        
        // ********** Channel information ***************************** //
        
        panoReadSHORT( src, &svar );                  // red
        panoReadINT32or64( src, &chlength, bBig);     // Length of following channel data.
        panoReadSHORT( src, &svar );                  // green
        panoReadINT32or64( src, &var64, bBig);        // Length of following channel data.
        panoReadSHORT( src, &svar );                  // blue
        panoReadINT32or64( src, &var64, bBig);        // Length of following channel data.
        if( 3 < uChannel )                            // alpha channel
        {
            panoReadSHORT( src, &svar );              // transparency mask
            panoReadINT32or64( src, &var64, bBig);    // Length of following channel data.
        }
        if( 4 < uChannel )                            // alpha channel
        {
            panoReadSHORT( src, &svar );              // transparency mask
            panoReadINT32or64( src, &var64, bBig);    // Length of following channel data.
        }
        // ********** End Channel information ***************************** //
    
        panoReadINT32( src, &var );                   // Blend mode signature Always 8BIM.
        panoReadINT32( src, &var );                   // Blend mode key

        panoReadINT32( src, &var );                   // Four flag bytes

        panoReadINT32( src, &var );                   // Extra data size Length of the extra data field. This is the total length of the next five fields.
        panoReadINT32( src, &var );                   // Layer Mask data
        if(var)                                       // either 0 or 20
        {
            for(kul=0; kul<var; kul++)
            {
                panoReadUCHAR( src, &ch );            // Layer Mask Data
            }
        }
        panoReadINT32( src, &var );                   // Layer blending ranges

        panoReadINT32( src, &var );                   // Layer name
        
        if(bBig)
        {
          var64 = 4*4 + 2 + uChannel * 10 + 4 + 4 + 4 * 1 + 4 + 12 + uChannel * chlength; // length
        }
        else
        {
          var64 = 4*4 + 2 + uChannel * 6 + 4 + 4 + 4 * 1 + 4 + 12 + uChannel * chlength; // length
        }
        if( var64/2 != (var64+1)/2 ) // odd
        {
            odd++; 
        }
    }
    
    
    // ************* End Layer Structure ******************************* //

    // ************* Read Channel Image data ************************** //

    for( i=0; i< mim->numLayers; i++)
    {
        uChannel    = (USHORT)(mim->Layer[i].bitsPerPixel/BitsPerSample);
        chlength    = mim->Layer[i].dataSize/ uChannel;
        buf = (unsigned char**) mymalloc( chlength );
        if( buf == NULL )
        {
            PrintError("Not enough memory");
            result = -1;
            goto readPSDMultiLayerImage_exit;
        }
        for( k=0; k< uChannel; k++ )
        {
            panoReadSHORT( src, &svar );
            if( svar != 0 )
            {
                PrintError("File format error");
                result = -1;
                goto readPSDMultiLayerImage_exit; 
            }
            count = chlength;
            myread(  src, count, *buf );
            {
                register int x,y,cy,by;
                register unsigned char* theData = *(mim->Layer[i].data);
                int offset,bpp;
                int DataWidth = mim->Layer[i].selection.right - mim->Layer[i].selection.left;
                int DataHeight = mim->Layer[i].selection.bottom - mim->Layer[i].selection.top;
                // JMW ToDo Upgrade allow 16 bit.
                offset = (mim->Layer[i].bitsPerPixel == 32?1:0) + k;
                if( k==3 ) offset = 0;
                
                bpp = mim->Layer[i].bitsPerPixel/8;
            
                for(y=0; y<DataHeight; y++)
                {
                    cy = y*mim->Layer[i].bytesPerLine + offset;
                    by = y*DataWidth;
                    for(x=0; x<DataWidth; x++)
                    {
                        theData[cy + bpp*x] = (*buf)[by + x];
                    }
                }
            }
        }
        myfree( (void**)buf );
    }

    if( 1 == odd%2 )    // pad byte
    {
        panoReadUCHAR( src, &ch );
    }
    

readPSDMultiLayerImage_exit:

    myclose( src );
    return result;
}



    
// JMW:  July 2 2004,  Check to see if alpha has any semi transparent pixels.
//    Also check to see if alpha is completely black
//    If completely black or using feathering then alpha
//    channel can not be used as shape mask.
int hasFeather ( Image *im )
{
    register int        y, x, a;
    register unsigned char  *alpha;
    int                 BitsPerChannel;

    GetBitsPerChannel( im, BitsPerChannel );

    if( im->bitsPerPixel != 32 && im->bitsPerPixel != 64) return 1;

    a = 1;
    
    if( BitsPerChannel == 8 )
    {
        for(y=0; y<im->height; y++)
        {
            for(x=0, alpha = *(im->data) + y * im->bytesPerLine; x<im->width;
                x++, alpha += 4)
            {
                if(a && *alpha != 0)// have data
                    a = 0;
                if( *alpha != 0 && *alpha != 255 )
                    return 1;
            }
        }
    }
    else // 16
    {
        for(y=0; y<im->height; y++)
        {
            for(x=0, alpha = *(im->data) + y * im->bytesPerLine; x<im->width;
                x++, alpha += 8)
            {
                if(a && *((USHORT*)alpha) != 0 )// have data
                    a = 0;
                if( *((USHORT*)alpha) != 0xFFFF &&  *((USHORT*)alpha) != 0 )
                    return 1;
            }
        }
    }
    return a;
}

// Create a temporary file
int panoFileMakeTemp(fullPath * path)
{
    file_spec fnum;
    char *dir;
    static int nTry = 0;
    int i;
    char fname[40];


    dir = strrchr(path->name, PATH_SEP);
    if (dir == NULL) {
        dir = path->name;
    }
    else {
        dir++;
    }

    nTry++;

    for (i = 0; i < MAX_TEMP_TRY; nTry++, i++) {
        sprintf(fname, "_PTStitcher_tmp_%06d", nTry);
        if (strlen(fname) + 2 <
            sizeof(path->name) - (strlen(path->name) - strlen(dir))) {
            sprintf(dir, "%s", fname);
			// TODO (dmg -- 060730)
			// THis routine is not perfect. IT does not check if the
			// file is really a file, nor that there is write access to it
            if (myopen(path, read_bin, fnum)) {
				return TRUE;
			}
            myclose(fnum);
        }
        else {
            PrintError("Path too long");
            return FALSE;
        }
    }
	return FALSE;
}

// Read an image
int panoImageRead(Image * im, fullPath * sfile)
{
    char *ext, extension[5];
    int i;

    assert(sfile != NULL);
    assert(im != NULL);
    
    printf("Filename %s\n", sfile->name);
    ext = strrchr(sfile->name, '.');
    if (ext == NULL || strlen(ext) < 4 || strlen(ext) > 5) {
	PrintError("Unsupported file format [%s]: must have extension JPG, PNG, TIF, BMP, PPM, PSD, PSB, or HDR", sfile);
	return 0;
    }
    ext++;
    strcpy(extension, ext);
    // convert to lowercase
    for (i = 0; i < 4; i++)
        extension[i] = tolower(extension[i]);
    
    if (strcmp(extension, "ppm") == 0) {
        return panoPPMRead(im, sfile);
    } 
    else if (strcmp(extension, "jpg") == 0) {
        return panoJPEGRead(im, sfile);
    } 
    else if (strcmp(extension, "tif") == 0 || strcmp(extension, "tiff") == 0) {
        return panoTiffRead(im, sfile->name);
    } 
    else if (strcmp( extension, "bmp" ) == 0 ) {
#ifdef WIN32		
		return panoBMPRead(  im, sfile );
#else
		PrintError("BMP is not a supported format in this operating system");
		return FALSE;
#endif
    } 
    else if( strcmp( extension, "png" ) == 0 ) {
        return panoPNGRead(im, sfile );
    } 
    else if( strcmp( extension, "psd" ) == 0 || strcmp( extension, "psb" ) == 0 ) {
        return readPSD(im, sfile, 1 );
    } 
    else if (strcmp( extension, "hdr" ) == 0 ) {
        return panoHDRRead(im, sfile );
    }
    else {
        PrintError("Unsupported file format [%s]: must have extension JPG, PNG, TIF, BMP, PPM, PSD, PSB, or HDR", sfile);
        return FALSE;
    }
    // it should never get here
    assert(0);
}

// Takes a prefix, and creates a list of unique filenames
int panoFileOutputNamesCreate(fullPath *ptrOutputFiles, int filesCount, char *outputPrefix)
{
    int i;
    char outputFilename[MAX_PATH_LENGTH];

    // I am sure we will never have more than 10000 images in a project!
#define DEFAULT_PREFIX_NUMBER_FORMAT "%04d"

    printf("Output prefix %d %s\n",filesCount,  outputPrefix);

    if (strchr(outputPrefix, '%') == NULL) {
	if ((strlen(outputPrefix) + strlen(DEFAULT_PREFIX_NUMBER_FORMAT)) >= MAX_PATH_LENGTH) {
	    PrintError("Output prefix too long [%s]", outputPrefix);
	    return 0;
	}
      	strcat(outputPrefix, DEFAULT_PREFIX_NUMBER_FORMAT);
    }

    for (i =0; i< filesCount ; i++) {

	sprintf(outputFilename, outputPrefix, i);
   
	// Verify the filename is different from the prefix 
	if (strcmp(outputFilename, outputPrefix) == 0) {
	    PrintError("Invalid output prefix. It does not generate unique filenames.");
	    return -1;
	}

	if (StringtoFullPath(&ptrOutputFiles[i], outputFilename) != 0) { 
	    PrintError("Syntax error: Not a valid pathname");
	    return(-1);
	}
	panoReplaceExt(ptrOutputFiles[i].name, ".tif");
	//    fprintf(stderr, "Output filename [%s]\n", ptrOutputFiles[i].name);
    }
    return 1;
}

// Find the first file that exists in the input array of filenames
char *panoFileExists(fullPath *ptrFiles, int filesCount)
{
    int i;
    FILE *testFile;
    for (i =0; i< filesCount ; i++) {
	if ((testFile = fopen(ptrFiles[i].name, "r"))!= NULL) {
            fclose(testFile);
	    return ptrFiles[i].name;
        }
    }
    return NULL;

}

// Check if a file exists. Returns 1 if it does exist, 0 otherwise
int panoSingleFileExists(char * filename)
{
    FILE *testFile;
	if ((testFile = fopen(filename, "r"))!= NULL) {
        fclose(testFile);
	    return 1;
    }
    return 0;
}




#ifdef DONOTCOMPILE_TOBE_DELETED

I am not sure this function is used at all



int writeImage( Image *im, fullPath *sfile )
{
    char *ext,extension[4];
    int i;
    
    ext = strrchr( sfile->name, '.' );
    ext++;
    strcpy( extension, ext );
    for(i=0; i<3; i++)
        extension[i] = tolower(extension[i]);
    
    
    if( strcmp( extension, "jpg" ) == 0 )
        return writeJPEG(im, sfile, 90, 0 );
    else if( strcmp( extension, "tif" ) == 0 )
        return writeTIFF(im, sfile );
    else if( strcmp( extension, "png" ) == 0 )
        return writePNG(im, sfile );
    else if( strcmp( extension, "hdr" ) == 0 )
        return writeHDR(im, sfile );
    else {
        return writeBMP( im, sfile );
    }
}

#endif

int panoFileDeleteMultiple(fullPath* files, int filesCount)
{
    extern int ptQuietFlag;
    int i;
    assert(files != NULL);
    for (i = 0; i < filesCount; i++) {
	if (!ptQuietFlag) {
	    PrintError("Deleting %-th source file %s", i, files[i].name);
	}
	if(remove(files[i].name) != 0) {
	    PrintError("Unable to remove file %s. Continuing", files[i].name);
	}
    }
    return 1;
}
