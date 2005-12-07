/* Panorama_Tools   -   Generate, Edit and Convert Panoramic Images
   Copyright (C) 2005 
   
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

#include "ptpano12.h"

#define DETAILS 0
#define TRACECOUNTLIMIT 2

#define CASE(x) case x: strcpy(buf, #x); break

#define SETFUNC(x)\
    x ## _func = (x ## _proc)getAddr(hinstLib, #x);\
    if (x ## _func == NULL) return FALSE

#define EXPORT extern "C" __declspec(dllexport)
typedef void (*dllEntry)();


#if(0) // prototype

THE_FUNCTION_NAME

typedef int (*THE_FUNCTION_NAME_proc)();
THE_FUNCTION_NAME_proc THE_FUNCTION_NAME_func;
EXPORT int THE_FUNCTION_NAME() {
    display(">> THE_FUNCTION_NAME");
    int rc = THE_FUNCTION_NAME_func();
    return rc;
}
        SETFUNC(THE_FUNCTION_NAME);

// ToDo: The following funtions are not implimented yet
DispPrg
InfoPrg
Java_pteditor_CExtract
Java_pteditor_CGetImageHeight
Java_pteditor_CGetImageRow
Java_pteditor_CGetImageWidth
Java_pteditor_CInsert
Java_pteditor_CLoadImage
Java_pteditor_CSaveImage
Java_pteditor_CSetImageHeight
Java_pteditor_CSetImageRow
Java_pteditor_CSetImageWidth
Java_ptutils_CAlignPoint
Java_ptutils_CCallOptimizer
Java_ptutils_CCreateProject
Java_ptutils_CGetCP_1n
Java_ptutils_CGetCP_1t
Java_ptutils_CGetCP_1x
Java_ptutils_CGetCP_1y
Java_ptutils_CGetControlPointCount
Java_ptutils_CGetHfov
Java_ptutils_CGetImageCount
Java_ptutils_CGetImageFormat
Java_ptutils_CGetImageHeight
Java_ptutils_CGetImageName
Java_ptutils_CGetImageRow
Java_ptutils_CGetImageWidth
Java_ptutils_CGetIndex
Java_ptutils_CGetPitch
Java_ptutils_CGetRoll
Java_ptutils_CGetTR_1i
Java_ptutils_CGetTR_1v
Java_ptutils_CGetTriangleCount
Java_ptutils_CGetYaw
Java_ptutils_CLaunchAndSendScript
Java_ptutils_CLoadImage
Java_ptutils_CLoadProject
Java_ptutils_CReduce
Java_ptutils_CSaveProject
Java_ptutils_CSetCP
Java_ptutils_CSetControlPointCount
Java_ptutils_CSetImageName
Java_ptutils_CSetTR
Java_ptutils_CSetTriangleCount
Java_ptutils_CShowScript
Java_ptutils_CTriangulate
SetAdPrefs
SetCPrefs
SetCrOpt
SetCutOpt
SetFrPrefs
SetHorOpt
SetIntp
SetLumOpt
SetPanOpt
SetPerspPrefs
SetRadOpt
SetRem
SetScOpt
SetShOpt
SetSiz
SetVerOpt

#endif // (0)

typedef void (*ConvFileName_proc)(fullPath *fname,char *string);
typedef void (*CopyImageData_proc)(Image *dest, Image *src);
typedef void (*FourToThreeBPP_proc)(Image *im);
typedef void (*PrintError_proc)(char* fmt, ...); 
typedef void (*SetAdjustDefaults_proc)(aPrefs *p);
typedef void (*SetImageDefaults_proc)(Image *im);
typedef void (*SetWindowOwner_proc)(HWND owner);
typedef int (*addLayerToFile_proc)(Image *im, fullPath* sfile, fullPath* dfile, stBuf *sB);
typedef void (*filter_main_proc)(TrformStr *TrPtr, struct size_Prefs *spref);
typedef void (*myfree_proc)(void **hdl);
typedef void** (*mymalloc_proc)(long numBytes);
typedef int (*readPrefs_proc)(char* p, int selector ); 
typedef int (*writePSD_proc)(Image *im, fullPath* fname);
typedef int (*writePSDwithLayer_proc)(Image *im, fullPath *fname);
typedef int (*writeTIFF_proc)(Image *im, fullPath* fname);
typedef void (*ARGBtoRGBA_proc)(UCHAR* buf, int width, int bitsPerPixel);
typedef void (*RGBAtoARGB_proc)(UCHAR* buf, int width, int bitsPerPixel);
typedef void (*CopyPosition_proc)(Image *to, Image *from);
typedef int (*CropImage_proc)(Image *im, PTRect *r);
typedef void (*DisposeAlignInfo_proc)(AlignInfo *g);
typedef int (*GetFullPath_proc)(fullPath *path, char *filename);
typedef int (*IsTextFile_proc)(char* fname);
typedef char* (*LoadScript_proc)(fullPath* scriptFile);
typedef enum {p12FeatureUnknown=0,p12FeatureInt=1,p12FeatureDouble=2,p12FeatureString=3} Tp12FeatureType;
typedef int (*queryFeatureCount_proc)();
typedef void (*queryFeatures_proc)(int index,char** name,Tp12FeatureType* type);
typedef int (*queryFeatureInt_proc)(const char *name, int *result);
typedef int (*queryFeatureDouble_proc)(const char *name, double *result);
typedef int (*queryFeatureString_proc)(const char *name,char *result, const int bufsize);
typedef void (*MakePano_proc)(TrformStr *TrPtr, aPrefs *aP);
typedef void (*MyMakePano_proc)(TrformStr *TrPtr, aPrefs *aP, int imageNum);
typedef int (*ParseScript_proc)(char* script, AlignInfo *gl);
typedef int (*Progress_proc)(int command, char* argument);
typedef int (*infoDlg_proc)(int command, char* argument);
typedef int (*SaveFileAs_proc)(fullPath *fname, char *prompt, char *name);
typedef void (*SetSizeDefaults_proc)(sPrefs *pref);
typedef int (*SetCorrectDefaults_proc)(cPrefs *p);
typedef void (*SetVRPanoOptionsDefaults_proc)(VRPanoOptions *v);
typedef void (*SetOptDefaults_proc)(optVars *opt);
typedef void (*SettMatrixDefaults_proc)(tMatrix *t);
typedef void (*SetStitchDefaults_proc)(stBuf *sbuf);
typedef int (*SetAlignParams_proc)(double *x);
typedef int (*SetLMParams_proc)(double *x);
typedef void (*SetGlobalPtr_proc)(AlignInfo *p);
typedef int (*StringtoFullPath_proc)(fullPath *path, char *filename);
typedef struct tiff TIFF;
typedef unsigned int ttag_t;        /* directory tag */
typedef unsigned short tdir_t;      /* directory index */
typedef unsigned short tsample_t;   /* sample number */
typedef unsigned int tstrip_t;  /* strip number */
typedef unsigned int ttile_t;       /* tile number */
typedef int tsize_t;        /* i/o size in bytes */
typedef void* tdata_t;      /* image data ref */
typedef unsigned int toff_t;        /* file offset */
typedef int (*readTIFF_proc)(Image *im, fullPath *sfile);
typedef int (*readtif_proc)(Image *im, TIFF* tif);
typedef void (*TIFFClose_proc)(TIFF* t);
typedef TIFF* (*TIFFOpen_proc)(const char* a, const char* b);
typedef int (*TIFFGetField_proc)(TIFF* pT, ttag_t tt, ...);
typedef int (*TIFFSetField_proc)(TIFF* pT, ttag_t tt, ...);
typedef int (*TIFFReadDirectory_proc)(TIFF* pT);
typedef int (*TIFFSetDirectory_proc)(TIFF* pT, tdir_t td);
typedef int (*TIFFReadScanline_proc)(TIFF* pT, tdata_t td, unsigned int un, tsample_t ts);
typedef int (*TIFFWriteDirectory_proc)(TIFF * pT);
typedef int (*TIFFWriteScanline_proc)(TIFF* pT, tdata_t td, unsigned int un, tsample_t ts);
typedef tsize_t (*TIFFScanlineSize_proc)(TIFF* pT);
typedef void (*TwoToOneByte_proc)(Image *im);
typedef void (*WriteResults_proc)(char* script, fullPath *sfile, AlignInfo *g, double ds( int i) , int launch);
typedef void (*nextWord_proc)(register char* word, char** ch );
typedef void (*nextLine_proc)(register char* line, char** ch);
typedef int (*numLines_proc)(char* script, char first);
typedef int (*getVRPanoOptions_proc)(VRPanoOptions *v, char *line);
typedef void (*showScript_proc)(fullPath* scriptFile);
typedef int (*makePathToHost_proc)(fullPath *path);
typedef int (*readPSDMultiLayerImage_proc)(MultiLayerImage *mim, fullPath* sfile);
typedef int (*writeImage_proc)(Image *im, fullPath *sfile);
typedef int (*writeJPEG_proc)(Image *im, fullPath *sfile,   int quality, int progressive);
typedef int (*makeTempPath_proc)(fullPath *path);
typedef int (*readPositions_proc)(char* script, transformCoord *tP);
typedef int (*readImage_proc)(Image *im, fullPath *sfile);
typedef int (*writePNG_proc)(Image *im, fullPath *sfile);
typedef aPrefs* (*readAdjustLine_proc)(fullPath *theScript);
typedef void (*pano_sphere_tp_proc)(double x_dest,double  y_dest, double* x_src, double* y_src, void* params);
typedef void (*execute_stack_proc)(double x_dest,double  y_dest, double* x_src, double* y_src, void* params);
typedef int (*normalToTriangle_proc)(CoordInfo *n, CoordInfo *v, triangle *t);
typedef int (*ReduceTriangles_proc)(AlignInfo *g, int nIm);
typedef int (*TriangulatePoints_proc)(AlignInfo *g, int nIm);
typedef void (*fwiener_proc)(TrformStr *TrPtr, Image *nf, Image *psf, double gamma, double frame);
typedef void (*noisefilter_proc)(Image *dest, Image *src);
typedef void (*doCoordinateTransform_proc)(CoordInfo *c, tMatrix *t);
typedef int (*fcnPano_proc)(int m, int n, double x[], double fvec[], int *iflag);
typedef double (*distSquared_proc)(int num);
typedef double (*OverlapRMS_proc)(MultiLayerImage *mim);
typedef void (*RunBROptimizer_proc)(OptInfo   *g, double minStepWidth);
typedef void (*RunLMOptimizer_proc)(OptInfo   *g);
typedef void (*DisposeMultiLayerImage_proc)(MultiLayerImage *mim);
typedef int (*SetDestImage_proc)(TrformStr *TrPtr, int width, int height);
typedef void (*DoColorCorrection_proc)(Image *im1, Image *im2, int mode);
typedef void (*SetInvMakeParams_proc)(struct fDesc *stack, struct MakeParams *mp, Image *im , Image *pn, int color);
typedef void (*SetMakeParams_proc)(struct fDesc *stack, struct MakeParams *mp, Image *im , Image *pn, int color);
typedef int (*InterpolateImageFile_proc)(fullPath *sfile, fullPath *dfile, AlignInfo *g,int nIm);
typedef int (*blendImages_proc)(fullPath *f0,  fullPath *f1, fullPath *result, double s);
typedef int (*MorphImageFile_proc)(fullPath *sfile, fullPath *dfile, AlignInfo *g,int nIm);
typedef int (*PositionCmp_proc)(Image *im1, Image *im2);
typedef int (*cutTheFrame_proc)(Image *dest, Image *src, int width, int height, int showprogress);
typedef int (*SetUpGamma_proc)(double pgamma, unsigned int psize);
typedef void (*ThreeToFourBPP_proc)(Image *im);
typedef int (*HaveEqualSize_proc)(Image *im1, Image *im2);
typedef int (*merge_proc)(Image *dst, Image *src, int feather, int showprogress, int seam);
typedef void (*addAlpha_proc)(Image *im);
typedef int (*FindFile_proc)(fullPath *fname);
typedef int (*readPSD_proc)(Image *im, fullPath* fname, int mode);
typedef int (*SaveBufImage_proc)(Image *image, char *fname);
typedef int (*LoadBufImage_proc)(Image *image, char *fname, int mode);
typedef void (*writePrefs_proc)(char* p, int selector);
typedef int (*CheckParams_proc)(AlignInfo *g);

ConvFileName_proc ConvFileName_func;
CopyImageData_proc CopyImageData_func;
FourToThreeBPP_proc FourToThreeBPP_func;
PrintError_proc PrintError_func;
SetAdjustDefaults_proc SetAdjustDefaults_func;
SetImageDefaults_proc SetImageDefaults_func;
SetWindowOwner_proc SetWindowOwner_func;
addLayerToFile_proc addLayerToFile_func;
filter_main_proc filter_main_func;
myfree_proc myfree_func;
mymalloc_proc mymalloc_func;
readPrefs_proc readPrefs_func;
writePSD_proc writePSD_func;
writePSDwithLayer_proc writePSDwithLayer_func;
writeTIFF_proc writeTIFF_func;
ARGBtoRGBA_proc ARGBtoRGBA_func;
RGBAtoARGB_proc RGBAtoARGB_func;
CopyPosition_proc CopyPosition_func;
CropImage_proc CropImage_func;
DisposeAlignInfo_proc DisposeAlignInfo_func;
GetFullPath_proc GetFullPath_func;
IsTextFile_proc IsTextFile_func;
LoadScript_proc LoadScript_func;
queryFeatureCount_proc queryFeatureCount_func;
queryFeatures_proc queryFeatures_func;
queryFeatureInt_proc queryFeatureInt_func;
queryFeatureDouble_proc queryFeatureDouble_func;
queryFeatureString_proc queryFeatureString_func;
MakePano_proc MakePano_func;
MyMakePano_proc MyMakePano_func;
ParseScript_proc ParseScript_func;
Progress_proc Progress_func;
infoDlg_proc infoDlg_func;
SaveFileAs_proc SaveFileAs_func;
SetSizeDefaults_proc SetSizeDefaults_func;
SetCorrectDefaults_proc SetCorrectDefaults_func;
SetVRPanoOptionsDefaults_proc SetVRPanoOptionsDefaults_func;
SetOptDefaults_proc SetOptDefaults_func;
SettMatrixDefaults_proc SettMatrixDefaults_func;
SetStitchDefaults_proc SetStitchDefaults_func;
SetAlignParams_proc SetAlignParams_func;
SetLMParams_proc SetLMParams_func;
SetGlobalPtr_proc SetGlobalPtr_func;
StringtoFullPath_proc StringtoFullPath_func;
readTIFF_proc readTIFF_func;
readtif_proc readtif_func;
TIFFClose_proc TIFFClose_func;
TIFFOpen_proc TIFFOpen_func;
TIFFGetField_proc TIFFGetField_func;
TIFFSetField_proc TIFFSetField_func;
TIFFReadDirectory_proc TIFFReadDirectory_func;
TIFFSetDirectory_proc TIFFSetDirectory_func;
TIFFReadScanline_proc TIFFReadScanline_func;
TIFFWriteDirectory_proc TIFFWriteDirectory_func;
TIFFWriteScanline_proc TIFFWriteScanline_func;
TIFFScanlineSize_proc TIFFScanlineSize_func;
TwoToOneByte_proc TwoToOneByte_func;
WriteResults_proc WriteResults_func;
nextWord_proc nextWord_func;
nextLine_proc nextLine_func;
numLines_proc numLines_func;
getVRPanoOptions_proc getVRPanoOptions_func;
showScript_proc showScript_func;
makePathToHost_proc makePathToHost_func;
readPSDMultiLayerImage_proc readPSDMultiLayerImage_func;
writeImage_proc writeImage_func;
writeJPEG_proc writeJPEG_func;
makeTempPath_proc makeTempPath_func;
readPositions_proc readPositions_func;
readImage_proc readImage_func;
writePNG_proc writePNG_func;
readAdjustLine_proc readAdjustLine_func;
pano_sphere_tp_proc pano_sphere_tp_func;
execute_stack_proc execute_stack_func;
normalToTriangle_proc normalToTriangle_func;
ReduceTriangles_proc ReduceTriangles_func;
TriangulatePoints_proc TriangulatePoints_func;
fwiener_proc fwiener_func;
noisefilter_proc noisefilter_func;
doCoordinateTransform_proc doCoordinateTransform_func;
fcnPano_proc fcnPano_func;
distSquared_proc distSquared_func;
OverlapRMS_proc OverlapRMS_func;
RunBROptimizer_proc RunBROptimizer_func;
RunLMOptimizer_proc RunLMOptimizer_func;
DisposeMultiLayerImage_proc DisposeMultiLayerImage_func;
SetDestImage_proc SetDestImage_func;
DoColorCorrection_proc DoColorCorrection_func;
SetInvMakeParams_proc SetInvMakeParams_func;
SetMakeParams_proc SetMakeParams_func;
InterpolateImageFile_proc InterpolateImageFile_func;
blendImages_proc blendImages_func;
MorphImageFile_proc MorphImageFile_func;
PositionCmp_proc PositionCmp_func;
cutTheFrame_proc cutTheFrame_func;
SetUpGamma_proc SetUpGamma_func;
ThreeToFourBPP_proc ThreeToFourBPP_func;
HaveEqualSize_proc HaveEqualSize_func;
merge_proc merge_func;
addAlpha_proc addAlpha_func;
FindFile_proc FindFile_func;
readPSD_proc readPSD_func;
SaveBufImage_proc SaveBufImage_func;
LoadBufImage_proc LoadBufImage_func;
writePrefs_proc writePrefs_func;
CheckParams_proc CheckParams_func;

void display(const char *fmt, ...) {
    static char LastBuf[100];
    static int  nCount   = 0;
    bool        bPrint   = true;

    char        buf[100];
    va_list     ap;
    FILE       *fp       = fopen("c:\\trace.txt", "a+");

    va_start(ap, fmt);
    vsprintf(buf, fmt, ap);
    lstrcat(buf, "\n");

    if( 0 == strcmp(LastBuf, buf) )
    {
        nCount++;
    }
    else
    {
        if( nCount == TRACECOUNTLIMIT+1 )
        {
            fwrite(LastBuf, 1, strlen(LastBuf), fp);
        }
        else if( nCount > TRACECOUNTLIMIT )
        {
            char  buf2[100];

            sprintf(buf2, "(last message repeated %d more times, but not printed.)\n", nCount - TRACECOUNTLIMIT);
            fwrite(buf2, 1, strlen(buf2), fp);
        }
        nCount = 1;
    }

    if(nCount <= TRACECOUNTLIMIT)
    {
        fwrite(buf, 1, strlen(buf), fp);
        
        if(nCount == 1)
        {
            strcpy(LastBuf, buf);
        }
    }

    fclose(fp);

    va_end(ap);
}

void display_PTRect(char *msg, PTRect *selection) {
  #if DETAILS
    display("PTRect (%s):", msg);
    display("  top:    %d", selection->top);
    display("  bottom: %d", selection->bottom);
    display("  left:   %d", selection->left);
    display("  right:  %d", selection->right);
  #endif
}

void display_cPrefs(char *msg, panoPrefs *prefs) {
  #if DETAILS
    display("cPrefs (%s):", msg);

    cPrefs *cP = &prefs->cP;

    display("  magic: %d", cP->magic);
    display("  radial:     %d", cP->radial);
    if (cP->radial)
        for (int i = 0; i < 3; i++)
            display("    %g\t%g\t%g\t%g\t%g",
                cP->radial_params[i][0], 
                cP->radial_params[i][1], 
                cP->radial_params[i][2], 
                cP->radial_params[i][3], 
                cP->radial_params[i][4]);

    display("  vertical:   %d", cP->vertical);
    if (cP->vertical)
        display("    %g\t%g\t%g",
            cP->vertical_params[0],
            cP->vertical_params[1],  
            cP->vertical_params[2]);

    display("  horizontal: %d", cP->horizontal);
    if (cP->horizontal)
        display("    %g\t%g\t%g",
            cP->horizontal_params[0],
            cP->horizontal_params[1],  
            cP->horizontal_params[2]);

    display("  shear:      %d", cP->shear);
    display("  resize:     %d", cP->resize);
    display("  width:      %d", cP->width);
    display("  height:     %d", cP->height);

    display("  luminance:  %d", cP->luminance);
    if (cP->luminance)
        display("    %g\t%g\t%g",
            cP->lum_params[0],
            cP->lum_params[1],  
            cP->lum_params[2]);

    display("  correction_mode: %d", cP->correction_mode);
    display("  cutFrame:   %d", cP->cutFrame);
    display("  fwidth:     %d", cP->fwidth);
    display("  fheight:    %d", cP->fheight);
    display("  frame:      %d", cP->frame);
    display("  fourier:    %d", cP->fourier);
  #endif
}

void display_Image(char *msg, Image *im) {
  #if DETAILS
    display("Image (%s):", msg);
    display("  width:        %d", im->width);
    display("  height:       %d", im->height);
    display("  bytesPerLine: %d", im->bytesPerLine);
    display("  bitsPerPixel: %d", im->bitsPerPixel);
    display("  dataSize:     %d", im->dataSize);
    display("  dataformat:   %d", im->dataformat);
    display("  format:       %d", im->format);
    display("  hfov:         %f", im->hfov);
    display("  yaw:          %f", im->yaw);
    display("  pitch:        %f", im->pitch);
    display("  roll:         %f", im->roll);
    display("  data:         %p", im->data);
//  display_cPrefs("Image.cP", &im->cP);
    display_PTRect("Image.selection", &im->selection);
  #endif
}

void display_stBuf(char *msg, stBuf *sB) {
  #if DETAILS

  #endif
}

char *mode(char *buf, int mode) {
    *buf = 0;
    if (mode > _wrapX || mode < 0) {
        strcat(buf, "invalid");
        return buf;
    }

    switch(mode & 7) {
    CASE(_interactive);
    CASE(_useprefs);
    CASE(_setprefs);
    CASE(_usedata);
    }

    if (mode & _honor_valid) strcat(buf, " _honor_valid");
    if (mode & _show_progress) strcat(buf, " _show_progress");
    if (mode & _hostCanResize) strcat(buf, " _hostCanResize");
    if (mode & _destSupplied) strcat(buf, " _destSupplied");
    if (mode & _wrapX) strcat(buf, " _wrapX");
    return buf;
}


char *interpolator(char *buf, int interpolator) {
    switch(interpolator) {
    CASE(_poly3);
    CASE(_spline16);
    CASE(_spline36);
    CASE(_sinc256);
    CASE(_spline64);
    CASE(_bilinear);
    CASE(_nn);
    CASE(_sinc1024);
    default: strcpy(buf, "invalid");
    }
    return buf;
}

char *tool(char *buf, int tool) {
    switch(tool) {
    CASE(_perspective);                   
    CASE(_correct);
    CASE(_remap);
    CASE(_adjust);
    CASE(_interpolate);
    CASE(_sizep);
    CASE(_version);
    CASE(_panright);
    CASE(_panleft);
    CASE(_panup);
    CASE(_pandown);
    CASE(_zoomin);
    CASE(_zoomout);
    CASE(_apply);
    CASE(_getPano);
    CASE(_increment);
    default: strcpy(buf, "invalid");
    }
    return buf;
}

void display_TrformStr(char *msg, TrformStr *tr) {
  #if DETAILS
    char buf[100];
    display("TrformStr (%s):", msg);
    display("  success:      %d", tr->success);
    display("  tool:         %s", tool(buf, tr->tool));
    display("  mode:         %s", mode(buf, tr->mode));
    display("  interpolator: %s", interpolator(buf, tr->interpolator));
    display("  gamma:        %g", tr->gamma);
    display("  data:         %p", tr->data);
    display_Image("Tr src", tr->src);
    display_Image("Tr dest", tr->dest);
    switch(tr->tool) {
    case _correct:
        display_cPrefs("TR cprefs", (panoPrefs *)tr->data);
        break;
    case _adjust:
        break;
    }
  #endif
}

void display_size_Prefs(char *msg, size_Prefs *sprefs) {
  #if DETAILS
    char buf[100];
    display("size_Prefs (%s):", msg);
    if (!sprefs) return;
    display("  saveFile:     %d", sprefs->saveFile);
    display("  launchApp:    %d", sprefs->launchApp);
    display("  interpolator: %s", interpolator(buf, sprefs->interpolator));
    display("  gamma:        %g", sprefs->gamma);
    display("  noAlpha:      %d", sprefs->noAlpha);
    display("  optCreatePano %d", sprefs->optCreatePano);
  #endif
}

void display_prefs(char *msg, int tool, panoPrefs *prefs) {
  #if DETAILS
    display("prefs (%s):", msg);
    switch(tool) {
    case _adjust:
        display("  mode:      %d", prefs->aP.mode);
        display_Image("aP.image", &prefs->aP.im);
        display_Image("aP.pano", &prefs->aP.pano);
        break;
    case _correct:
        break;
    }
  #endif
}


EXPORT void ConvFileName(fullPath *fname, char *string) {
    display(">> ConvFileName (%s)", string);
    ConvFileName_func(fname, string);
}


EXPORT void CopyImageData(Image *dest, Image *src) {
    display(">> CopyImageData");
    display_Image("  src", src);
    display_Image("  dest", dest);
    CopyImageData_func(dest, src);
}

EXPORT void FourToThreeBPP(Image *im) {
    display(">> FourToThreeBPP");
    display_Image("  before", im);
    FourToThreeBPP_func(im);
    display_Image("  after", im);
}

EXPORT void PrintError(char* fmt, ...) {
    char buf[100];
    va_list ap;

    va_start(ap, fmt);
    vsprintf(buf, fmt, ap);
    MessageBox(NULL, buf, "Info", MB_OK);
    va_end(ap);
} 

EXPORT void SetAdjustDefaults(aPrefs *p) {
    display(">> SetAdjustDefaults");
    SetAdjustDefaults_func(p);
}

EXPORT void SetImageDefaults(Image *im) {
    display(">> SetImageDefaults");
    SetImageDefaults_func(im);
}

EXPORT void SetWindowOwner(HWND owner) {
    display(">> SetWindowOwner");
    SetWindowOwner_func(owner);
}

EXPORT int addLayerToFile(Image *im, fullPath* sfile, fullPath* dfile, stBuf *sB) {
    display(">> addLayerToFile");
    display("  sfile=%d", sfile);
    display("  dfile=%d", dfile);
    display_Image("", im);
    display_stBuf("", sB);
    int rc = addLayerToFile_func(im, sfile, dfile, sB);
    return rc;
}

EXPORT void filter_main(TrformStr *TrPtr, struct size_Prefs *spref) {
    display(">> filter_main");
    display_TrformStr("before filter_main", TrPtr);
    display_size_Prefs("before filter_main", spref);
    filter_main_func(TrPtr, spref);
    display_TrformStr("after filter_main", TrPtr);
    display_size_Prefs("after filter_main", spref);
}

EXPORT void myfree(void **hdl) {
    display(">> myfree (%p)", hdl);
    myfree_func(hdl);
}

EXPORT void** mymalloc(long numBytes) {
    display(">> mymalloc (%d)", numBytes);
    void **rc = mymalloc_func(numBytes);
    return rc;
}

EXPORT int readPrefs(char* p, int selector) {
    char buf[100];
    display(">> readPrefs (%s)", tool(buf, selector));
    int rc = readPrefs_func(p, selector);
    return rc;
} 

EXPORT int writePSD(Image *im, fullPath* fname) {
    display(">> writePSD (%s)", fname->name);
    display_Image("  ", im);
    int rc = writePSD_func(im, fname);
    return rc;
}

EXPORT int writePSDwithLayer(Image *im, fullPath *fname) {
    display(">> writePSDwithLayer (%s)", fname->name);
    int rc = writePSDwithLayer_func(im, fname);
    return rc;
}

EXPORT int writeTIFF(Image *im, fullPath* fname) {
    display(">> writeTIFF (%s)", fname->name);
    int rc = writeTIFF_func(im, fname);
    return rc;
}

EXPORT void ARGBtoRGBA(UCHAR* buf, int width, int bitsPerPixel) {
    display(">> ARGBtoRGBA");
    ARGBtoRGBA_func(buf, width, bitsPerPixel);
}

EXPORT void RGBAtoARGB(UCHAR* buf, int width, int bitsPerPixel) {
    display(">> RGBAtoARGB");
    RGBAtoARGB_func(buf, width, bitsPerPixel);
}

EXPORT void CopyPosition(Image *to, Image *from) {
    display(">> CopyPosition");
  #if DETAILS
    display_Image("  to", to);
    display_Image("  from", from);
  #endif
    CopyPosition_func(to, from);
}

EXPORT int CropImage(Image *im, PTRect *r) {
    display(">> CropImage");
    display_Image("  ", im);
    int rc = CropImage_func(im, r);
    return rc;
}

EXPORT void DisposeAlignInfo(AlignInfo *g) {
    display(">> DisposeAlignInfo");
    DisposeAlignInfo_func(g);
}

EXPORT int GetFullPath(fullPath *path, char *filename) {
    display(">> GetFullPath (%s)", filename);
    int rc = GetFullPath_func(path, filename);
    return rc;
}

EXPORT int IsTextFile(char* fname) {
    display(">> IsTextFile (%s)", fname);
    int rc = IsTextFile_func(fname);
    return rc;
}

EXPORT char* LoadScript(fullPath* scriptFile) {
    display(">> LoadScript (%s)", scriptFile->name);
    char* rc = LoadScript_func(scriptFile);
    return rc;
}


EXPORT int queryFeatureCount() {
    display(">> queryFeatureCount");
    int rc = queryFeatureCount_func();
    return rc;
}

EXPORT void queryFeatures(int index,char** name,Tp12FeatureType* type) {
    display(">> queryFeatures");
    queryFeatures_func(index, name, type);
}

EXPORT int queryFeatureInt(const char *name, int *result) {
    display(">> queryFeatureInt");
    int rc = queryFeatureInt_func(name, result);
    return rc;
}

EXPORT int queryFeatureDouble(const char *name, double *result) {
    display(">> queryFeatureDouble");
    int rc = queryFeatureDouble_func(name, result);
    return rc;
}

EXPORT int queryFeatureString(const char *name,char *result, const int bufsize) {
    display(">> queryFeatureString");
    int rc = queryFeatureString_func(name, result, bufsize);
    return rc;
}

EXPORT void MyMakePano(TrformStr *TrPtr, aPrefs *aP, int imageNum) {
    display(">> MyMakePano");
    MyMakePano_func(TrPtr, aP, imageNum);
}

EXPORT void MakePano(TrformStr *TrPtr, aPrefs *aP) {
    display(">> MakePano");
    MakePano_func(TrPtr , aP);
}

EXPORT int ParseScript(char* script, AlignInfo *gl) {
    display(">> ParseScript");
    int rc = ParseScript_func(script, gl);
    return rc;
}

EXPORT int Progress(int command, char* argument) {
    display(">> Progress (%s)", argument);
    int rc = Progress_func(command, argument);
    return rc;
}

EXPORT int infoDlg(int command, char* argument) {
    display(">> infoDlg (%s)", argument);
    int rc = infoDlg_func(command, argument);
    return rc;
}

EXPORT int SaveFileAs(fullPath *fname, char *prompt, char *name) {
    display(">> SaveFileAs (%s)", name);
    int rc = SaveFileAs_func(fname, prompt, name);
    return rc;
}

EXPORT void SetSizeDefaults(sPrefs *pref) {
    display(">> SetSizeDefaults");
    SetSizeDefaults_func(pref);
}

EXPORT int SetCorrectDefaults(cPrefs *p) {
    display(">> SetCorrectDefaults");
    int rc = SetCorrectDefaults_func(p);
    return rc;
}

EXPORT void SetVRPanoOptionsDefaults(VRPanoOptions *v) {
    display(">> SetVRPanoOptionsDefaults");
    SetVRPanoOptionsDefaults_func(v);
}

EXPORT void SetOptDefaults(optVars *opt) {
    display(">> SetOptDefaults");
    SetOptDefaults_func(opt);
}

EXPORT void SettMatrixDefaults(tMatrix *t) {
    display(">> SettMatrixDefaults");
    SettMatrixDefaults_func(t);
}

EXPORT void SetStitchDefaults(stBuf *sbuf) {
    display(">> SetStitchDefaults");
    SetStitchDefaults_func(sbuf);
}

EXPORT int SetAlignParams(double *x) {
    display(">> SetAlignParams");
    int rc = SetAlignParams_func(x);
    return rc;
}

EXPORT int SetLMParams(double *x) {
    display(">> SetLMParams");
    int rc = SetLMParams_func(x);
    return rc;
}

EXPORT void SetGlobalPtr(AlignInfo *p) {
    display(">> SetGlobalPtr");
    SetGlobalPtr_func(p);
}

EXPORT int StringtoFullPath(fullPath *path, char *filename) {
    display(">> StringtoFullPath (%s)", filename);
    int rc = StringtoFullPath_func(path, filename);
    return rc;
}

EXPORT int readTIFF(Image *im, fullPath *sfile) {
    display(">> readTIFF");
    int rc = readTIFF_func(im, sfile);
    return rc;
}

EXPORT int readtif(Image *im, TIFF* tif) {
    display(">> readtif");
    int rc = readtif_func(im, tif);
    return rc;
}

EXPORT void TIFFClose(TIFF* t) {
    display(">> TIFFClose");
    TIFFClose_func(t);
}

EXPORT TIFF* TIFFOpen(const char* a, const char* b) {
    display(">> TIFFOpen, (%s)(%s)", a, b);
    TIFF* rc = TIFFOpen_func(a, b);
    return rc;
}

// Not sure if this one is done right
// from  ...  to  va_list
EXPORT int TIFFGetField(TIFF* pT, ttag_t tt, va_list v) {
    display(">> TIFFGetField");
    int rc = TIFFGetField_func(pT, tt, v);
    return rc;
}

EXPORT int TIFFSetField(TIFF* pT, ttag_t tt, va_list v) {
    display(">> TIFFSetField");
    int rc = TIFFSetField_func(pT, tt, v);
    return rc;
}

EXPORT int TIFFReadDirectory(TIFF* pT) {
    display(">> TIFFReadDirectory");
    int rc = TIFFReadDirectory_func(pT);
    return rc;
}

EXPORT int TIFFSetDirectory(TIFF* pT, tdir_t td) {
    display(">> TIFFSetDirectory");
    int rc = TIFFSetDirectory_func(pT, td);
    return rc;
}

EXPORT int TIFFReadScanline(TIFF* pT, tdata_t td, unsigned int un, tsample_t ts) {
    display(">> TIFFReadScanline");
    int rc = TIFFReadScanline_func(pT, td, un, ts);
    return rc;
}

EXPORT int TIFFWriteDirectory(TIFF * pT) {
    display(">> TIFFWriteDirectory");
    int rc = TIFFWriteDirectory_func(pT);
    return rc;
}

EXPORT int TIFFWriteScanline(TIFF* pT, tdata_t td, unsigned int un, tsample_t ts) {
    display(">> TIFFWriteScanline");
    int rc = TIFFWriteScanline_func(pT, td, un, ts);
    return rc;
}

EXPORT tsize_t TIFFScanlineSize(TIFF* pT) {
    tsize_t rc = TIFFScanlineSize_func(pT);
    display(">> %d = TIFFScanlineSize", rc);
    return rc;
}

EXPORT void TwoToOneByte(Image *im) {
    display(">> TwoToOneByte");
    TwoToOneByte_func(im);
}

EXPORT void WriteResults(char* script, fullPath *sfile, AlignInfo *g, double ds( int i) , int launch) {
    display(">> WriteResults");
    WriteResults_func(script, sfile, g, ds,launch);
}

EXPORT void nextWord(register char* word, char** ch ) {
    display(">> nextWord");
    nextWord_func(word, ch );
}

EXPORT void nextLine(register char* line, char** ch) {
    display(">> nextLine");
    nextLine_func(line, ch);
}

EXPORT int numLines(char* script, char first) {
    display(">> numLines");
    int rc = numLines_func(script, first);
    return rc;
}

EXPORT int getVRPanoOptions(VRPanoOptions *v, char *line) {
    display(">> getVRPanoOptions");
    int rc = getVRPanoOptions_func(v, line);
    return rc;
}

EXPORT void showScript(fullPath* scriptFile) {
    display(">> showScript");
    showScript_func(scriptFile);
}

EXPORT int makePathToHost(fullPath *path) {
    display(">> makePathToHost");
    int rc = makePathToHost_func(path);
    return rc;
}

EXPORT int readPSDMultiLayerImage(MultiLayerImage *mim, fullPath* sfile) {
    display(">> readPSDMultiLayerImage");
    int rc = readPSDMultiLayerImage_func(mim, sfile);
    return rc;
}

EXPORT int writeImage(Image *im, fullPath *sfile) {
    display(">> writeImage");
    int rc = writeImage_func(im, sfile);
    return rc;
}

EXPORT int writeJPEG(Image *im, fullPath *sfile,   int quality, int progressive) {
    display(">> writeJPEG");
    int rc = writeJPEG_func(im, sfile, quality, progressive);
    return rc;
}

EXPORT int makeTempPath(fullPath *path) {
    fullPath fp;
    memcpy( &fp, path, sizeof(fullPath) );
    int rc = makeTempPath_func(path);
    display(">> makeTempPath (%s) to (%s)",&fp, path);
    return rc;
}

EXPORT int readPositions(char* script, transformCoord *tP) {
    display(">> readPositions");
    int rc = readPositions_func(script, tP);
    return rc;
}

EXPORT int readImage(Image *im, fullPath *sfile) {
    display(">> readImage (%s)", sfile);
    int rc = readImage_func(im, sfile);
    return rc;
}

EXPORT int writePNG(Image *im, fullPath *sfile) {
    display(">> writePNG");
    int rc = writePNG_func(im, sfile);
    return rc;
}

EXPORT aPrefs* readAdjustLine(fullPath *theScript) {
    display(">> readAdjustLine (%s)", theScript );
    aPrefs* rc = readAdjustLine_func(theScript);
    return rc;
}

EXPORT void pano_sphere_tp(double x_dest,double  y_dest, double* x_src, double* y_src, void* params) {
    display(">> pano_sphere_tp");
    pano_sphere_tp_func(x_dest, y_dest, x_src, y_src, params);
    return;
}

EXPORT void execute_stack(double x_dest,double  y_dest, double* x_src, double* y_src, void* params) {
    display(">> execute_stack");
    execute_stack_func(x_dest, y_dest, x_src, y_src, params);
    return;
}
EXPORT int normalToTriangle(CoordInfo *n, CoordInfo *v, triangle *t) {
    display(">> normalToTriangle");
    int rc = normalToTriangle_func(n, v, t);
    return rc;
}

EXPORT int ReduceTriangles(AlignInfo *g, int nIm) {
    display(">> ReduceTriangles");
    int rc = ReduceTriangles_func(g, nIm);
    return rc;
}

EXPORT int TriangulatePoints(AlignInfo *g, int nIm) {
    display(">> TriangulatePoints");
    int rc = TriangulatePoints_func(g, nIm);
    return rc;
}

EXPORT void fwiener(TrformStr *TrPtr, Image *nf, Image *psf, double gamma, double frame) {
    display(">> fwiener");
    fwiener_func(TrPtr, nf, psf, gamma, frame);
    return;
}

EXPORT void noisefilter(Image *dest, Image *src) {
    display(">> noisefilter");
    noisefilter_func(dest, src);
    return;
}

EXPORT void doCoordinateTransform(CoordInfo *c, tMatrix *t) {
    display(">> doCoordinateTransform");
    doCoordinateTransform_func(c, t);
    return;
}

EXPORT int fcnPano(int m, int n, double x[], double fvec[], int *iflag) {
    display(">> fcnPano");
    int rc = fcnPano_func(m, n, x, fvec, iflag);
    return rc;
}

EXPORT double distSquared(int num) {
    display(">> distSquared");
    double rc = distSquared_func(num);
    return rc;
}

EXPORT double OverlapRMS(MultiLayerImage *mim) {
    display(">> OverlapRMS");
    double rc = OverlapRMS_func(mim);
    return rc;
}

EXPORT void RunBROptimizer(OptInfo   *g, double minStepWidth) {
    display(">> RunBROptimizer");
    RunBROptimizer_func(g, minStepWidth);
    return;
}

EXPORT void RunLMOptimizer(OptInfo   *g) {
    display(">> RunLMOptimizer");
    RunLMOptimizer_func(g);
    return;
}

EXPORT void DisposeMultiLayerImage(MultiLayerImage *mim) {
    display(">> DisposeMultiLayerImage");
    DisposeMultiLayerImage_func(mim);
    return;
}

EXPORT int SetDestImage(TrformStr *TrPtr, int width, int height) {
    display(">> SetDestImage");
    int rc = SetDestImage_func(TrPtr, width, height);
    return rc;
}

EXPORT void DoColorCorrection(Image *im1, Image *im2, int mode) {
    display(">> DoColorCorrection");
    DoColorCorrection_func(im1, im2, mode);
    return;
}

EXPORT void SetInvMakeParams(struct fDesc *stack, struct MakeParams *mp, Image *im , Image *pn, int color) {
    display(">> SetInvMakeParams");
    SetInvMakeParams_func(stack, mp, im , pn, color);
    return;
}

EXPORT void SetMakeParams(struct fDesc *stack, struct MakeParams *mp, Image *im , Image *pn, int color) {
    display(">> SetMakeParams");
    SetMakeParams_func(stack, mp, im , pn, color);
    return;
}

EXPORT int InterpolateImageFile(fullPath *sfile, fullPath *dfile, AlignInfo *g,int nIm) {
    display(">> InterpolateImageFile");
    int rc = InterpolateImageFile_func(sfile, dfile, g, nIm);
    return rc;
}

EXPORT int blendImages(fullPath *f0,  fullPath *f1, fullPath *result, double s) {
    display(">> blendImages");
    int rc = blendImages_func(f0, f1, result, s);
    return rc;
}

EXPORT int MorphImageFile(fullPath *sfile, fullPath *dfile, AlignInfo *g,int nIm) {
    display(">> MorphImageFile");
    int rc = MorphImageFile_func(sfile, dfile, g, nIm);
    return rc;
}

EXPORT int PositionCmp(Image *im1, Image *im2) {
    display(">> PositionCmp");
    int rc = PositionCmp_func(im1, im2);
    return rc;
}

EXPORT int cutTheFrame(Image *dest, Image *src, int width, int height, int showprogress) {
    display(">> cutTheFrame");
    int rc = cutTheFrame_func(dest, src, width, height, showprogress);
    return rc;
}

EXPORT int SetUpGamma(double pgamma, unsigned int psize) {
    display(">> SetUpGamma");
    int rc = SetUpGamma_func(pgamma, psize);
    return rc;
}

EXPORT void ThreeToFourBPP(Image *im) {
    display(">> ThreeToFourBPP");
    ThreeToFourBPP_func(im);
    return;
}

EXPORT int HaveEqualSize(Image *im1, Image *im2) {
    display(">> HaveEqualSize");
    int rc = HaveEqualSize_func(im1, im2);
    return rc;
}

EXPORT int merge(Image *dst, Image *src, int feather, int showprogress, int seam) {
    display(">> merge");
    int rc = merge_func(dst, src, feather, showprogress, seam);
    return rc;
}

EXPORT void addAlpha(Image *im) {
    display(">> addAlpha");
    addAlpha_func(im);
    return;
}

EXPORT int FindFile(fullPath *fname) {
    display(">> FindFile");
    int rc = FindFile_func(fname);
    return rc;
}

EXPORT int readPSD(Image *im, fullPath* fname, int mode) {
    display(">> readPSD");
    int rc = readPSD_func(im, fname, mode);
    return rc;
}

EXPORT int SaveBufImage(Image *image, char *fname) {
    display(">> SaveBufImage");
    int rc = SaveBufImage_func(image, fname);
    return rc;
}

EXPORT int LoadBufImage(Image *image, char *fname, int mode) {
    display(">> LoadBufImage");
    int rc = LoadBufImage_func(image, fname, mode);
    return rc;
}

EXPORT void writePrefs(char* p, int selector) {
    display(">> writePrefs");
    writePrefs_func(p, selector);
    return;
}

EXPORT int CheckParams(AlignInfo *g) {
    display(">> CheckParams");
    int rc = CheckParams_func(g);
    return rc;
}
dllEntry getAddr(HINSTANCE hinstLib, char *func) {
    dllEntry addr = (dllEntry) GetProcAddress(hinstLib, func); 
    if (addr == NULL) {
        FreeLibrary(hinstLib);
        display("Pano12.dll is invalid (%s).", func);
    }
    return addr;
}

int WINAPI DllMain (HINSTANCE hInstance, DWORD fdwReason, PVOID pvReserved) {
    static HINSTANCE hinstLib;

    if (fdwReason == DLL_PROCESS_ATTACH) {
        // Get a handle to the DLL module.
        hinstLib = LoadLibrary("pano13.dll");
        if (hinstLib == NULL) {
            display("Missing C:\\WINDOWS\\system\\pano12.dll");
            return FALSE;
        }

        // initialize all function pointers
        SETFUNC(ConvFileName);
        SETFUNC(CopyImageData);
        SETFUNC(FourToThreeBPP);
        SETFUNC(PrintError);
        SETFUNC(SetAdjustDefaults);
        SETFUNC(SetImageDefaults);
        SETFUNC(SetWindowOwner);
        SETFUNC(addLayerToFile);
        SETFUNC(filter_main);
        SETFUNC(myfree);
        SETFUNC(mymalloc);
        SETFUNC(readPrefs);
        SETFUNC(writePSD);
        SETFUNC(writePSDwithLayer);
        SETFUNC(writeTIFF);
        SETFUNC(ARGBtoRGBA);
        SETFUNC(RGBAtoARGB);
        SETFUNC(CopyPosition);
        SETFUNC(CropImage);
        SETFUNC(DisposeAlignInfo);
        SETFUNC(GetFullPath);
        SETFUNC(IsTextFile);
        SETFUNC(LoadScript);
        SETFUNC(queryFeatureCount);
        SETFUNC(queryFeatures);
        SETFUNC(queryFeatureInt);
        SETFUNC(queryFeatureDouble);
        SETFUNC(queryFeatureString);
        SETFUNC(MakePano);
        SETFUNC(MyMakePano);
        SETFUNC(ParseScript);
        SETFUNC(Progress);
        SETFUNC(infoDlg);
        SETFUNC(SaveFileAs);
        SETFUNC(SetSizeDefaults);
        SETFUNC(SetCorrectDefaults);
        SETFUNC(SetVRPanoOptionsDefaults);
        SETFUNC(SetOptDefaults);
        SETFUNC(SettMatrixDefaults);
        SETFUNC(SetStitchDefaults);
        SETFUNC(SetAlignParams);
        SETFUNC(SetLMParams);
        SETFUNC(SetGlobalPtr);
        SETFUNC(StringtoFullPath);
        SETFUNC(readTIFF);
        SETFUNC(readtif);
        SETFUNC(TIFFClose);
        SETFUNC(TIFFOpen);
        SETFUNC(TIFFGetField);
        SETFUNC(TIFFSetField);
        SETFUNC(TIFFReadDirectory);
        SETFUNC(TIFFSetDirectory);
        SETFUNC(TIFFReadScanline);
        SETFUNC(TIFFWriteDirectory);
        SETFUNC(TIFFWriteScanline);
        SETFUNC(TIFFScanlineSize);
        SETFUNC(TwoToOneByte);
        SETFUNC(WriteResults);
        SETFUNC(nextWord);
        SETFUNC(nextLine);
        SETFUNC(numLines);
        SETFUNC(getVRPanoOptions);
        SETFUNC(showScript);
        SETFUNC(makePathToHost);
        SETFUNC(readPSDMultiLayerImage);
        SETFUNC(writeImage);
        SETFUNC(writeJPEG);
        SETFUNC(makeTempPath);
        SETFUNC(readPositions);
        SETFUNC(readImage);
        SETFUNC(writePNG);
        SETFUNC(readAdjustLine);
        SETFUNC(pano_sphere_tp);
        SETFUNC(execute_stack);
        SETFUNC(normalToTriangle);
        SETFUNC(ReduceTriangles);
        SETFUNC(TriangulatePoints);
        SETFUNC(fwiener);
        SETFUNC(noisefilter);
        SETFUNC(doCoordinateTransform);
        SETFUNC(fcnPano);
        SETFUNC(distSquared);
        SETFUNC(OverlapRMS);
        SETFUNC(RunBROptimizer);
        SETFUNC(RunLMOptimizer);
        SETFUNC(DisposeMultiLayerImage);
        SETFUNC(SetDestImage);
        SETFUNC(DoColorCorrection);
        SETFUNC(SetInvMakeParams);
        SETFUNC(SetMakeParams);
        SETFUNC(InterpolateImageFile);
        SETFUNC(blendImages);
        SETFUNC(MorphImageFile);
        SETFUNC(PositionCmp);
        SETFUNC(cutTheFrame);
        SETFUNC(SetUpGamma);
        SETFUNC(ThreeToFourBPP);
        SETFUNC(HaveEqualSize);
        SETFUNC(merge);
        SETFUNC(addAlpha);
        SETFUNC(FindFile);
        SETFUNC(readPSD);
        SETFUNC(SaveBufImage);
        SETFUNC(LoadBufImage);
        SETFUNC(writePrefs);
        SETFUNC(CheckParams);
    } else if (fdwReason == DLL_PROCESS_DETACH) {
        FreeLibrary(hinstLib);
    }

    return TRUE;
}
