/* queryfeature.c
   Feature querying functionality
   See queryfeature.h
*/
#include <stdio.h>
#include <string.h>

#include "version.h"
#include "queryfeature.h"
#include "filter.h"

typedef struct {char* name; int value;}    TIntFeature;
typedef struct {char* name; double value;} TDoubleFeature;
typedef struct {char* name; char* value;}  TStringFeature;

// Fulvio Senore June.2004 changed the check to work with microsoft compiler
#ifdef _MSC_VER
//#ifdef MSVS
#define snprintf _snprintf
#endif

/***************** Feature tables: *************************/
TIntFeature intFeatures[] ={
  {"CPErrorIsDistSphere",1},       // optimizer reports angular control point errors
  {"NumLensTypes",5},              // source lens types 0..4
  {"NumPanoTypes",8},              // pano lens types 0..7
  {"CanCropOutside",1},
  {"CanHaveNegativeCP",1},
  {"AntiAliasingFilter",1},
  {"NumFilter",24},
  {"SetProgressFcn",1}             // setProgressFcn and setInfoDlgFcn are available
};

TDoubleFeature doubleFeatures[] ={
  {"MaxFFOV",MAX_FISHEYE_FOV}
};

TStringFeature stringFeatures[]={
  // Version info:
  {PTVERSION_NAME_FILEVERSION,PTVERSION_FILEVERSION},         // "FileVersion"
  {PTVERSION_NAME_LONG,LONGVERSION},                          // "LongVersion"
  {PTVERSION_NAME_LEGALCOPYRIGHT,PTVERSION_LEGALCOPYRIGHT},   // "LegalCopyright"
  {PTVERSION_NAME_COMMENT,PTVERSION_COMMENT},                 // "Comments"
  // Source lens type names
  // If a lens type is unavailable, set its name to ""
  {"LensType0","Normal (rectilinear)"},
  {"LensType1","Cylindrical"},
  {"LensType2","Circular"},
  {"LensType3","Full Frame"},
  {"LensType4","Equirectangular"},
  // Source lens type crop format (C)ircle or (R)ectangle:
  {"LensMask0","R"},
  {"LensMask1","R"},
  {"LensMask2","C"},
  {"LensMask3","R"},
  {"LensMask4","R"},
  // Pano lens type names 
  // If a lens type is unavailable, set its name to ""
  {"PanoType0","Normal (rectilinear)"},
  {"PanoType1","Cylindrical"},
  {"PanoType2","Equirectangular"},
  {"PanoType3","Full Frame"},
  {"PanoType4","Stereographic"},
  {"PanoType5","Mercator"},
  {"PanoType6","Transverse mercator"},
  {"PanoType7","Sinusoidal"},
  {"PanoType8","Lambert Cylindrical Equal Area"},
  {"PanoType9","Lambert Azimuthal Equal Area"},
  {"PanoType10","Albers Conical Equal Area"},
  // Filter Types
  //   fix: Fixed Windowsize
  //   aa: Antialiasing filter with adaptive filter size
  // Filter Names 
  {"FilterType0","fix"},
  {"FilterName0","Poly3"},
  {"FilterType1","fix"},
  {"FilterName1","Spline16"},
  {"FilterType2","fix"},
  {"FilterName2","Spline36"},
  {"FilterType2","fix"},
  {"FilterName3","Sinc256"},
  {"FilterType4","fix"},
  {"FilterName4","Spline64"},
  {"FilterType5","fix"},
  {"FilterName5","Bilinear"},
  {"FilterType6","fix"},
  {"FilterName6","Nearest neighbor"},
  {"FilterType7","fix"},
  {"FilterName7","Sinc1024"},
  {"FilterType8","aa"},
  {"FilterName8","Box"},
  {"FilterType9","aa"},
  {"FilterName9","Bartlett/Triangle"},
  {"FilterType10","aa"},
  {"FilterName10","Hermite"},
  {"FilterType11","aa"},
  {"FilterName11","Hanning"},
  {"FilterType12","aa"},
  {"FilterName12","Hamming"},
  {"FilterType13","aa"},
  {"FilterName13","Blackmann"},
  {"FilterType14","aa"},
  {"FilterName14","Gaussian 1/sqrt(2)"},
  {"FilterType15","aa"},
  {"FilterName15","Gaussian 1/2"},
  {"FilterType16","aa"},
  {"FilterName16","Quadardic"},
  {"FilterType17","aa"},
  {"FilterName17","Cubic"},
  {"FilterType18","aa"},
  {"FilterName18","Catmull-Rom"},
  {"FilterType19","aa"},
  {"FilterName19","Mitchell"},
  {"FilterType20","aa"},
  {"FilterName20","Lanczos2"},
  {"FilterType21","aa"},
  {"FilterName21","Lanczos3"},
  {"FilterType22","aa"},
  {"FilterName22","Blackman/Bessel"},	
  {"FilterType23","aa"},
  {"FilterName23","Blackman/sinc"},
#if 0
  // WE NO longer support need to list them. It should be enough to list the version

  // Patches that have been applied
  {"Patch200510a", "Rob Platt, Do not process unchanged color channels for CA correction"},
  {"BMPrev", "Jim Watters, correctly open BMP files created with rows in reverse order"},
  {"Tiff32", "Thomas Rauscher, Load and save TIFF 32-bit with IEEE floats, http://www.pano2qtvr.com/dll_patch/"},
  {"AntiAliasing", "Thomas Rauscher, New antialiasing filter for adjust, http://www.pano2qtvr.com/dll_patch/"},
  {"HDRFile", "Thomas Rauscher, Load and save Radiance HDR files, http://www.pano2qtvr.com/dll_patch/"},
  {"Patch200505", "Douglas Wilkins, Correct behaviour when mode = _usedata"},
#ifdef HasJava
  {"Patch200504a", "Douglas Wilkins, Java support enabled"},
#else
  {"Patch200504a", "Douglas Wilkins, Java support disabled"},
#endif
  {"Patch200502a", "Joost Nieuwenhuijse, Crop outside of image, http://www.ptgui.com"},
  {"Patch200410a", "Jim Watters, JPEG optimization, http://photocreations.ca/panotools"},
  {"FastTransform01", "Fulvio Senore, Fast transform, http://www.fsoft.it/panorama/pano12.htm"},
  {"Patch200407a", "Rik Littlefield, Kevin Kratzke, & Jim Watters, Fix multiple bugs - PSD, 16bit"},
  {"MaskFromFocus_001", "Rik Littlefield, added mask-from-focus, http://www.janrik.net/ptools/"},
  {"Patch200405a", "Rik Littlefield, Improved optimizer, http://www.janrik.net/ptools/"},
  {"Patch200403a", "Kevin Kratzke, Radial Shift fix, http://www.kekus.com"},
  {"Patch200312a", "Jim Watters, Updated PSD format, http://photocreations.ca/panotools"},
  {"Patch200309a", "Jim Watters, Allowed linking of y, p, & r values, http://photocreations.ca/panotools"},
  {"Patch200308a", "Jim Watters, Improved Radial Luminance, http://photocreations.ca/panotools"}
#endif
};
/***************** end feature tables **********************/


int queryFeatureInt(const char *name, int *result)
{
  int i;
  int count = sizeof( intFeatures ) / sizeof( intFeatures[0] );
  for(i=0; i < count; i++)
  {
    if(strcmp(name,intFeatures[i].name)==0)
    {
      *result=intFeatures[i].value;
      return 1;
    }
  }
  return 0;
}

int queryFeatureDouble(const char *name, double *result)
{
  int i;
  int count = sizeof( doubleFeatures ) / sizeof( doubleFeatures[0] );
  for(i=0; i < count; i++)
  {
    if(strcmp(name,doubleFeatures[i].name)==0)
    {
      *result=doubleFeatures[i].value;
      return 1;
    }
  }
  return 0;
}

int queryFeatureString(const char *name,char *result, const int bufsize)
{
  int intvalue;
  double doublevalue;
  size_t i, length=0, count = (sizeof( stringFeatures ) / sizeof( stringFeatures[0] ));
  const size_t tmp_len=200;
  
  // Fulvio Senore, August 2004
  // allocates a dummy buffer for the calls to snprintf
  // the original code passed NULL to snprintf() but it caused problems (asserts) when compiling 
  // with the microsoft compiler that links with the debug libraries to avoid crashes
  char *cpTmp = malloc( tmp_len + 1 );
  cpTmp[tmp_len] = '\0';

  for(i=0; i < count; i++)
  {
    if(strcmp(name,stringFeatures[i].name)==0)
    {
      if(result != NULL)
      {
        strncpy(result, stringFeatures[i].value, (size_t)bufsize);
      }
      length=strlen(stringFeatures[i].value);
      break;
    }
  }
  if(length <= 0)
  {
    // there's no string value with the specified name
    // Let's check the int values too:
    for(i=0; i < sizeof( intFeatures ) / sizeof( intFeatures[0] ); i++)
    {
      if(queryFeatureInt(name, &intvalue))
      {
        // length=snprintf(NULL,0,"%d",intvalue);
        length=snprintf(cpTmp,tmp_len,"%d",intvalue);
        if(result != NULL)
        {
          snprintf(result,(size_t)bufsize,"%d",intvalue);
        }
        break;
      }
    }
  }
  if(length <= 0)
  {
    // Let's check the double values too:
    for(i=0; i < sizeof( doubleFeatures ) / sizeof( doubleFeatures[0] ); i++)
    {
      if(queryFeatureDouble(name, &doublevalue))
      {
//        length=snprintf(NULL,0,"%0.f",doublevalue);
        length=snprintf(cpTmp,tmp_len,"%0.f",doublevalue);
        if(result != NULL)
        {
          snprintf(result,(size_t)bufsize,"%0.f",doublevalue);
        }
        break;
      }
    }
  }
  // make sure that the copied string always is NULL terminated, even if truncated
  // (except if the buffer holds only zero bytes):
  if( result && ((int)length >= bufsize) && (bufsize > 0) )
  {
    result[bufsize-1]=0;
  }
  free( cpTmp );
  return length;
}

int queryFeatureCount()
{
  return sizeof( intFeatures ) / sizeof( intFeatures[0] )+ 
	     sizeof( doubleFeatures ) / sizeof( doubleFeatures[0] ) + 
		 sizeof( stringFeatures ) / sizeof( stringFeatures[0] );
}

void queryFeatures(int index,char** name,Tp12FeatureType* type)
{
  if(index < (sizeof( intFeatures ) / sizeof( intFeatures[0] )))
  {
    if(name) *name=intFeatures[index].name;
    if(type) *type=p12FeatureInt;
  }
  else
  {
    index -= sizeof( intFeatures ) / sizeof( intFeatures[0] );
    if(index < sizeof( doubleFeatures ) / sizeof( doubleFeatures[0] ))
    {
      if(name) *name=doubleFeatures[index].name;
      if(type) *type=p12FeatureDouble;
    }
    else
    {
      index -= sizeof( doubleFeatures ) / sizeof( doubleFeatures[0] );
      if(index < sizeof( stringFeatures ) / sizeof( stringFeatures[0] ))
      {
        if(name) *name=stringFeatures[index].name;
        if(type) *type=p12FeatureString;
      }
      else
      {
        if(type) *type=p12FeatureUnknown;
      }
    }
  }
}

