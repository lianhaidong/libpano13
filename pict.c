/*#include <Gestalt.h> //commented by Kekus Digital
#include "filter.h"

#include <FixMath.h>
#include <ImageCompression.h>
#include <Movies.h>
#include <QuickTimeComponents.h>
#include <StandardFile.h>
#include <PictUtils.h>
#include <Memory.h>
#include <OSUtils.h>
#include <ToolUtils.h>
#include <Math.h>
#include <Gestalt.h>
#include <ImageCompression.h>*/ // till here
#include "PixMap.h"
#include <carbon/carbon.h> //added by Kekus Digital
#include <QuickTime/QuickTimeComponents.h>
#include <QuickTime/ImageCompression.h> // till here

#include <String.h>
#include "filter.h"
#include <stdio.h>

#include "jpeglib.h"


static void 		FSpWritePicture (const FSSpec *spec, PicHandle pictData); 
static 	PicHandle 	FSpReadPicture 	(const FSSpec *spec) ;
static 	PicHandle 	ImageToPICT		(Image *im) ;
static 	int 		PICTtoImage		( Image *im, PicHandle pic );
static  int 		qtimporter( Image *im, fullPath *sfile );

int writePICT			( Image *im, fullPath *sfile );
int readPICT			( Image *im, fullPath *sfile );
int readJPEG( Image *im, fullPath *sfile );
int readTIFF( Image *im, fullPath *sfile );

					

int writePICT( Image *im, fullPath *sfile )
{
	PicHandle 		pic = NULL;

	TwoToOneByte( im ); // Pict doesn't support 16bit channels
	
	pic = ImageToPICT( im );
	if( pic == NULL )
	{
		PrintError("Could not create Picture");
		return -1;
	}
	FSpWritePicture ( sfile, pic);
	KillPicture(pic);
	
	return 0;
}

int readPICT ( Image *im, fullPath *sfile )
{
	PicHandle 		pic = NULL;
	int result = 0;

	pic = FSpReadPicture (sfile);
	if( pic == NULL )
	{
		PrintError("Could not read Picture");
		return -1;
	}

	if( PICTtoImage	( im, pic ) != 0)
	{ 
		PrintError("Could not convert Picture");
		result = -1;
	}

	KillPicture(pic);
	return result;
}
	


static void FSpWritePicture (const FSSpec *spec, PicHandle pictData) 
{
	short fRefNum;
	long zeroData = 0L, zeroDataLen, pictLength;
	int i;
	
//	FSpCreate(spec, 'GKON', 'PICT', smSystemScript);
	FSpOpenDF(spec, fsWrPerm, &fRefNum);
	
	zeroDataLen = sizeof(long);
	for (i = 0; i < 512 / zeroDataLen; i++) 
		FSWrite(fRefNum, &zeroDataLen, &zeroData);
		
	pictLength = GetHandleSize((Handle)pictData);
	HLock((Handle)pictData);
	FSWrite(fRefNum, &pictLength, *pictData);
	HUnlock((Handle)pictData);
	
	FSClose(fRefNum);
}



static PicHandle FSpReadPicture (const FSSpec *spec) 
{
	PicHandle	pictData;
	short		fRefNum;
	long		fileSize, pictLength;

	FSpOpenDF(spec, fsRdPerm, &fRefNum);
	GetEOF(fRefNum, &fileSize);
	SetFPos(fRefNum, fsFromStart, 512);
	
	pictLength = fileSize - 512;
	pictData = (PicHandle)NewHandle(pictLength);
	if( pictData != nil )
	{
		HLock((Handle)pictData);
		FSRead(fRefNum, &pictLength, *pictData);
		HUnlock((Handle)pictData);
	}
	FSClose(fRefNum);
	
	return(pictData);
}

static PicHandle ImageToPICT(Image *im) 
{
	GrafPtr saved;
	//CGrafPort port;//commented by Kekus Digital
	CGrafPtr port;//added by Kekus Digital
	Rect bounds;
	PixMapHandle 	pix = NULL;
	PicHandle 	pic = NULL;
	
	long row_bytes;
	Ptr raster_data;
	long bytecount, response;
	CTabHandle colours;
	PixMapPtr pixptr;
	unsigned char *rowptr;
	unsigned long *pixelptr;
	short y, x;

		/* is it possible? */
	if (Gestalt(gestaltQuickdrawVersion, &response) != noErr) response = 0;
	if (response < gestalt32BitQD) return NULL;

		/* set up initial values for locals */
	row_bytes = ((32 * ((long) im->width) + 31) >> 5) << 2;
	if( row_bytes != im->bytesPerLine )
	{
		PrintError("Alignment problem: Rowbytes != bytesPerLine");
		return NULL;
	}
	bytecount 	= row_bytes * ((long) im->height);
	colours 	= NULL;
	raster_data = NULL;
	
		/* check for valid rowbytes */
	if (row_bytes > 0x00003FFF) goto bail;

		/* allocate the raster data */
	raster_data = (Ptr)*(im->data);
	rowptr = (unsigned char *) raster_data;
	for (y=0; y< im->height; y++, rowptr += row_bytes)
		for (pixelptr = (unsigned long *) rowptr, x=0; x<im->width; x++)
			*pixelptr++ &= 0x00FFFFFF;

		/* dummy colour table */
	colours = (CTabHandle) NewHandleClear(8);
	if (colours == NULL) goto bail;
	(*colours)->ctSeed = 24;
	(*colours)->ctSize = -1;

		/* create the pixmap handle */
	pix = (PixMapHandle) NewHandleClear(sizeof(PixMap));
	if (pix == NULL) goto bail;
	
		/* set up the pixmap fields */
	HLock((Handle) pix);
	pixptr = *pix;
	pixptr->baseAddr = raster_data;
	pixptr->rowBytes = (row_bytes | 0x8000);
	pixptr->bounds.right = im->width;
	pixptr->bounds.bottom = im->height;
	pixptr->hRes = 0x00480000; /* resolution = 72 dots per inch */
	pixptr->vRes = 0x00480000;
	pixptr->pixelSize = 32;
	pixptr->pixelType = RGBDirect;
	pixptr->cmpCount = 3;
	pixptr->cmpSize = 8;
	pixptr->pmTable = colours;

	bounds = (*pix)->bounds;
	GetPort(&saved);
	//OpenCPort(&port);//commented by Kekus Digital
        port = CreateNewPort();//added by Kekus Digital
	pic = OpenPicture(&bounds);
	ClipRect(&bounds);
	PlotPixMap(pix, 0, 0, srcCopy);
	ClosePicture();
	SetPort(saved);
	//ClosePort((GrafPtr) &port);//commented by Kekus Digital
        DisposePort(port);//added by Kekus Digital
	if (GetHandleSize((Handle) pic) == sizeof(Picture)) {
		KillPicture(pic);
		pic = NULL;
	}
bail:
	if (pix != NULL) DisposeHandle((Handle) pix);
	if (colours != NULL) DisposeHandle((Handle) colours);
	return pic;
}


static int PICTtoImage( Image *im, PicHandle pic )
{
	Rect bounds;
	PixMapHandle pix = NULL;
	PixMapPort *pxmp;
	PictInfo pInfo;

	long row_bytes;
	Ptr raster_data;
	Handle hdl_raster_data = NULL;
	long bytecount, response;
	CTabHandle colours;
	PixMapPtr pixptr;
	unsigned char *rowptr;
	unsigned long *pixelptr;
	short y, x;


	if( GetPictInfo( pic, &pInfo, recordComments, (short) 0,systemMethod,(short) 0 ) != noErr	)
	{
		PrintError("Could not get Pict-info");
		return -1;
	}

	bounds = pInfo.sourceRect; //(*pic)->picFrame;
	OffsetRect(&bounds, -bounds.left, -bounds.top);

		/* is it possible? */
	if (Gestalt(gestaltQuickdrawVersion, &response) != noErr) response = 0;
	if (response < gestalt32BitQD) return -1;

		/* set up initial values for locals */
	row_bytes = ((32 * ((long) bounds.right) + 31) >> 5) << 2;
	bytecount = row_bytes * ((long) bounds.bottom);
	colours 	= NULL;
	raster_data = NULL;
	
		/* check for valid rowbytes */
	if (row_bytes > 0x00003FFF) return -1;

		/* allocate the raster data */
	hdl_raster_data=NewHandle(bytecount);
	if (hdl_raster_data == NULL)
	{
		PrintError("Not enough memory to load picture");
		return -1;
	}
	HLock(hdl_raster_data);

	raster_data = *hdl_raster_data;
	rowptr = (unsigned char *) raster_data;
	for (y=0; y< bounds.bottom; y++, rowptr += row_bytes)
		for (pixelptr = (unsigned long *) rowptr, x=0; x<bounds.right; x++)
			*pixelptr++ = 0x00FFFFFF;

		/* dummy colour table */
	colours = (CTabHandle) NewHandleClear(8);
	if( colours == NULL) return -1;

	(*colours)->ctSeed = 24;
	(*colours)->ctSize = -1;

		/* create the pixmap handle */
	pix = (PixMapHandle) NewHandleClear(sizeof(PixMap));
	if (pix == NULL) return -1;
	
		/* set up the pixmap fields */
	HLock((Handle)pix);
	pixptr = *pix;
	pixptr->baseAddr = raster_data;
	pixptr->rowBytes = (row_bytes | 0x8000);
	pixptr->bounds.right = bounds.right;
	pixptr->bounds.bottom = bounds.bottom;
	pixptr->hRes = pInfo.hRes; // 0x00480000; /* resolution = 72 dots per inch */
	pixptr->vRes = pInfo.vRes; // 0x00480000;
	pixptr->pixelSize = 32;
	pixptr->pixelType = RGBDirect;
	pixptr->cmpCount = 3;
	pixptr->cmpSize = 8;
	pixptr->pmTable = colours;


	WithPixMap(pix, pxmp) 
	{
			DrawPicture(pic, &bounds);
	}


	SetImageDefaults( im );

	
	im->width 	= (*pix)->bounds.right;
	im->height 	= (*pix)->bounds.bottom;
	im->bitsPerPixel = 32;
	im->bytesPerLine = ((32 * ((long) im->width) + 31) >> 5) << 2;
	im->dataSize = im->bytesPerLine * im->height;
	im->data = (unsigned char**) hdl_raster_data;
	
	// Set alpha channel
	
	LOOP_IMAGE( im, *idata = UCHAR_MAX );
	
	if ((*pix)->pmTable != NULL) 	DisposeHandle((Handle) (*pix)->pmTable);
	HUnlock( (Handle)pix );
	if (pix != NULL) 				DisposeHandle((Handle) pix);
	
	if( pInfo.thePalette != NULL ) 		DisposeHandle((Handle)  pInfo.thePalette);
	if( pInfo.theColorTable != NULL ) 	DisposeHandle((Handle)  pInfo.theColorTable);
	if( pInfo.commentHandle != NULL ) 	DisposeHandle((Handle)  pInfo.commentHandle);
	if( pInfo.fontHandle != NULL ) 		DisposeHandle((Handle)  pInfo.fontHandle);
	if( pInfo.fontNamesHandle != NULL ) DisposeHandle((Handle)  pInfo.fontNamesHandle);
	
	return 0;
}







int readImage( Image *im, fullPath *sfile )
{
	FileInfo fInfo;
	char ext[5], *ex;
	int i;

	// Get file info	
	FSpGetFInfo	(sfile, (FInfo*)&fInfo );
	
	// Get extension
	ext[0] = 0;
	p2cstr(sfile->name);
	ex = strrchr( (char*)sfile->name, '.' );
	if( ex != NULL && strlen(ex) < 6 )
		strcpy( ext, ex+1 );
	c2pstr((char*)sfile->name);
	for(i=0; i<strlen(ext); i++)
		ext[i] = tolower(ext[i]);

	// read image depending on type

	if		( fInfo.fileType == 'PICT' || strcmp( ext, "pict" ) == 0 )
		return readPICT	( im, sfile );
	else if	( fInfo.fileType == 'JPEG' || strcmp( ext, "jpg" ) == 0 )
		 return readJPEG	( im, sfile );
	else if	( fInfo.fileType == 'PNGf' || strcmp( ext, "png" ) == 0 )
		 return readPNG	( im, sfile );
	else if	( fInfo.fileType == 'TIFF' || strcmp( ext, "tif" ) == 0 || strcmp( ext, "tiff" ) == 0)
		 return readTIFF( im, sfile );
	else
		return qtimporter	( im, sfile );

	return -1;
}

static int qtimporter( Image *im, fullPath *sfile )
{
	GraphicsImportComponent		gImporter  = NULL;
	OSErr						myErr 	   = noErr;
	Rect						myRect;
	GWorldPtr					myGWorld;
	PixMapHandle				myPixMap;
	
	myErr = GetGraphicsImporterForFile( sfile, &gImporter);
	
	if(myErr != noErr)
	{
		PrintError("Could not get GraphicImporter for this file");
		return -1;
	}

	myErr = GraphicsImportGetNaturalBounds(gImporter, &myRect);
	if(myErr != noErr)
	{
		PrintError("Could not get GraphicImporter for this file");
		return -1;
	}

	myErr = NewGWorld(&myGWorld, 0, &myRect, NULL, NULL, 0);
	if (myErr != noErr) {
		PrintError("Not enough Memory to read image");
		return -1;
	}
	myErr = GraphicsImportSetGWorld(gImporter, (CGrafPtr)myGWorld, NULL);
	if( myErr != noErr){
		PrintError("Error setting GWorld");
		return -1;
	}
	myErr = GraphicsImportDraw(gImporter);
	if( myErr != noErr){
		PrintError("Error Drawing to GWorld");
		return -1;
	}
		
	OffsetRect(&myRect, -myRect.left, -myRect.top);

	SetImageDefaults( im );
	
	myPixMap = GetGWorldPixMap(myGWorld);
	
	LockPixels(myPixMap);
	
	if( (*myPixMap)->pixelSize != 32 ||
		(*myPixMap)->pixelType != RGBDirect )
	{
		PrintError("Only 24bit rgb formats supported");
		return -1;
	}
	
	im->width 			= (*myPixMap)->bounds.right;
	im->height 			= (*myPixMap)->bounds.bottom;
	im->bitsPerPixel 	= 32;
	
	im->bytesPerLine 	= (*myPixMap)->rowBytes & 0x7FFF;
	im->dataSize 		= im->bytesPerLine * im->height;
	im->data 			= (unsigned char**)mymalloc( im->dataSize );
	if( im->data 		== NULL )
	{
		PrintError("Not enough memory to load image");
		return -1;
	}
	memcpy( *im->data, (*myPixMap)->baseAddr, im->dataSize );		
	UnlockPixels( myPixMap);

	if (gImporter != NULL)
		CloseComponent(gImporter);
		
	DisposeGWorld( myGWorld );
	
	// Set alpha channel
	
	//LOOP_IMAGE( im, *idata = 0xff )
	
	return 0;
}



int writeImage( Image *im, fullPath *sfile )
{
	return writePICT( im, sfile );
}


int makeTempPath( fullPath *path )
{
	file_spec fnum;
	static int try = 0;
	int i;
	
#define MAX_TEMP_TRY	1000
	
	try++;
	
	for(i=0; i < MAX_TEMP_TRY; try++,i++)
	{
		
		sprintf( (char*)path->name, "_PTStitcher_tmp_%d", try );
		c2pstr( (char*)path->name );
		if( myopen( path, read_bin, fnum ))
			break;
		myclose( fnum );
	}
	if( try < MAX_TEMP_TRY )
		return 0;
	else
		return -1;
	
}

