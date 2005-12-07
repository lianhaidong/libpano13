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

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                       *
   This file is a copy of filter.h and panorama.h from Panorama Tools 
   with everything removed that is not neccessary for the tracing functions 
   to work or would break the tracing dll.
 *                                                                       *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */


#ifndef PTFILTER_H
#define PTFILTER_H


#include <math.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <limits.h>

#include "..\panotypes.h"

// MRDL: Replaced BIGENDIAN with PT_BIGENDIAN to eliminate conflict with 
// BIGENDIAN defined in winsock2.h distributed with MingW 2.0

// Determine which machine we are using. Macs are set to PT_BIGENDIAN, all others not

// If you need PT_BIGENDIAN, and don't use MacOS, define it here:
//#define PT_BIGENDIAN          1


// Create a definition if we're on a Windows machine:
#ifndef __Win__
    #if (defined(MSDOS) || defined(WIN32))
        #define __Win__         1
    #endif
#endif


// Create a definition if we're on a Macintosh:
#ifndef __Mac__
    #if (defined(macintosh) || defined(__MC68K__) || defined(__powerc))
        #define __Mac__         1
        #define PT_BIGENDIAN        1
    #endif
#endif



// Use FSSpec on Macs as Path-specifyers, else strings
#define PATH_SEP                            '/'

#ifdef __Mac__
        //#include  <Files.h> // commented by Kekus Digital
    #include <Carbon/Carbon.h> // added by Kekus Digital
    #define         fullPath                            FSSpec
    #undef  PATH_SEP
    #define PATH_SEP                                    ':'

#else // __Mac__, use ANSI-filefunctions

    #ifdef __Win__
        #ifndef __NO_SYSTEM__
            #include <windows.h> // including this causes problems with libjpeg
        #endif
        #define MAX_PATH_LENGTH     260
        // was MAX_PATH
        #undef  PATH_SEP
        #define PATH_SEP                            '\\'
    #else
        #define MAX_PATH_LENGTH     512
    #endif

    typedef struct{char name[MAX_PATH_LENGTH];} fullPath;

#endif


// Enumerates for TrFormStr.tool
enum{                                       // Panorama Tools
    _perspective,
    _correct,
    _remap,
    _adjust,
    _interpolate,
    _sizep,                                 // dummy for size-preferences
    _version,                               // dummy for version
    _panright,                              // Pan Controls
    _panleft,
    _panup,
    _pandown,
    _zoomin,
    _zoomout,
    _apply,
    _getPano,
    _increment
};


// Enumerates for TrFormStr.mode
enum{                                       //  Modes
    _interactive,                           //  display dialogs and do Xform
    _useprefs,                              //  load saved prefs and do Xform/ no dialogs
    _setprefs,                              //  display dialogs and set preferences, no Xform    
    _usedata,                               //  use supplied data in TrFormStr.data, do Xform
    _honor_valid        = 8,                //  Use only pixels with alpha channel set
    _show_progress      = 16,               //  Interpolator displays progress bar
    _hostCanResize      = 32,               //  o-no; 1-yes (Photoshop: no; GraphicConverter: yes)
    _destSupplied       = 64,               //  Destination image allocated by plug-in host
    _wrapX              = 128               //  Wrap image horizontally (if HFOV==360 degrees)
};


// Enumerates for Image.dataformat
enum{
    _RGB,
    _Lab,
    _Grey
};

// Enumerates for TrFormStr.interpolator
enum{                                       // Interpolators
    _poly3      = 0,                        // Third order polynomial fitting 16 nearest pixels
    _spline16   = 1,                        // Cubic Spline fitting 16 nearest pixels
    _spline36   = 2,                        // Cubic Spline fitting 36 nearest pixels
    _sinc256    = 3,                        // Sinc windowed to 8 pixels
    _spline64,                              // Cubic Spline fitting 64 nearest pixels
    _bilinear,                              // Bilinear interpolation
    _nn,                                    // Nearest neighbor
    _sinc1024,
    // Thomas Rauscher: New antialiasing filter. 
    // Plots of the functions are available at http://www.pano2qtvr.com/dll_patch/
    _aabox,                                 // Antialiasing: Box
    _aatriangle,                            // Antialiasing: Bartlett/Triangle Filter
    _aahermite,                             // Antialiasing: Hermite Filter
    _aahanning,                             // Antialiasing: Hanning Filter
    _aahamming,                             // Antialiasing: Hamming Filter
    _aablackman,                            // Antialiasing: Blackmann Filter
    _aagaussian,                            // Antialiasing: Gaussian 1/sqrt(2) Filter (blury)
    _aagaussian2,                           // Antialiasing: Gaussian 1/2 Filter (sharper)
    _aaquadratic,                           // Antialiasing: Quadardic Filter
    _aacubic,                               // Antialiasing: Cubic Filter
    _aacatrom,                              // Antialiasing: Catmull-Rom Filter
    _aamitchell,                            // Antialiasing: Mitchell Filter
    _aalanczos2,                            // Antialiasing: Lanczos2 Filter
    _aalanczos3,                            // Antialiasing: Lanczos3 Filter
    _aablackmanbessel,                      // Antialiasing: Blackman/Bessel Filter
    _aablackmansinc                         // Antialiasing: Blackman/sinc Filter
};

// Corrections

struct  correct_Prefs{                      //  Preferences structure for tool correct
    pt_uint32       magic;                  //  File validity check, must be 20
    int             radial;                 //  Radial correction requested?
    double          radial_params[3][5];    //  3 colors x (4 coeffic. for 3rd order polys + correction radius)
    int             vertical;               //  Vertical shift requested ?
    double          vertical_params[3];     //  3 colors x vertical shift value
    int             horizontal;             //  horizontal tilt ( in screenpoints)
    double          horizontal_params[3];   //  3 colours x horizontal shift value
    int             shear;                  //  shear correction requested?
    double          shear_x;                //  horizontal shear values
    double          shear_y;                //  vertical shear values
    int             resize;                 //  scaling requested ?
    pt_int32        width;                  //  new width
    pt_int32        height;                 //  new height
    int             luminance;              //  correct luminance variation?
    double          lum_params[3];          //  parameters for luminance corrections
    int             correction_mode;        //  0 - radial correction;1 - vertical correction;2 - deregistration
    int             cutFrame;               //  remove frame? 0 - no; 1 - yes
    int             fwidth;
    int             fheight;
    int             frame;
    int             fourier;                //  Fourier filtering requested?
    int             fourier_mode;           //  _faddBlurr vs _fremoveBlurr
    fullPath        psf;                    //  Point Spread Function, full path/fsspec to psd-file
    int             fourier_nf;             //  Noise filtering: _nf_internal vs _nf_custom
    fullPath        nff;                    //  noise filtered file: full path/fsspec to psd-file
    double          filterfactor;           //  Hunt factor
    double          fourier_frame;          //  To correct edge errors
};

typedef struct correct_Prefs cPrefs;

enum{
    correction_mode_radial      = 0,
    correction_mode_vertical    = 1,
    correction_mode_deregister  = 2,
    correction_mode_morph       = 4
};


enum{
    _faddBlurr,
    _fremoveBlurr,
    _nf_internal,
    _nf_custom,
    _fresize
};


enum{               // Enumerates for Image.format
    _rectilinear    = 0,
    _panorama       = 1,
    _fisheye_circ   = 2,
    _fisheye_ff     = 3,
    _equirectangular= 4,
    _spherical_cp   = 5,
    _spherical_tp   = 6,
    _mirror         = 7,
    _orthographic   = 8,
    _cubic          = 9
};

// A large rectangle

typedef struct{
    pt_int32        top;
    pt_int32        bottom;
    pt_int32        left;
    pt_int32        right;
} PTRect;



struct Image{
        // Pixel data
    pt_int32        width;
    pt_int32        height;
    pt_int32        bytesPerLine;
    pt_int32        bitsPerPixel;           // Must be 24 or 32
    pt_uint32       dataSize; 
    unsigned char** data;
    pt_int32        dataformat;             // rgb, Lab etc
    pt_int32        format;                 // Projection: rectilinear etc
    double          hfov;
    double          yaw;
    double          pitch;
    double          roll;
    cPrefs          cP;                     // How to correct the image
    char            name[256];
    PTRect          selection;
};

typedef struct Image Image;


struct TrformStr                            // This structure holds all image information
{
    Image          *src;                    // Source image, must be supplied on entry
    Image          *dest;                   // Destination image data, valid if success = 1 
    pt_int32        success;                // 0 - no, 1 - yes 

    pt_int32        tool;                   // Panorama Tool requested
    pt_int32        mode;                   // how to run transformation
    void           *data;                   // data for tool requested.
                                            // Required only if mode = _usedata; then it
                                            // must point to valid preferences structure
                                            // for requested tool (see filter.h).

    pt_int32 interpolator;                  // Select interpolator
    double gamma;                           // Gamma value for internal gamma correction
};

typedef struct TrformStr TrformStr; 


struct PTPoint
{
    double x;
    double y;
};

typedef struct PTPoint PTPoint;

#define CopyPTPoint( to, from )       memcpy( &to, &from, sizeof( PTPoint ))
#define SamePTPoint( p, s )           ((p).x == (s).x && (p).y == (s).y)

struct PTLine
{
    PTPoint         v[2];
};

typedef struct PTLine PTLine;


struct PTTriangle
{
    PTPoint         v[3];
};

typedef struct PTTriangle PTTriangle;


struct remap_Prefs{                         //  Preferences Structure for remap
    pt_int32        magic;                  //  File validity check, must be 30
    int             from;                   //  Image format source image
    int             to;                     //  Image format destination image
    double          hfov;                   //  horizontal field of view /in degrees
    double          vfov;                   //  vertical field of view (usually ignored)
};

typedef struct remap_Prefs rPrefs;

struct perspective_Prefs{                   //  Preferences structure for tool perspective
    pt_int32        magic;                  //  File validity check, must be 40
    int             format;                 //  rectilinear or fisheye?
    double          hfov;                   //  Horizontal field of view (in degree)
    double          x_alpha;                //  New viewing direction (x coordinate or angle)
    double          y_beta;                 //  New viewing direction (y coordinate or angle)
    double          gamma;                  //  Angle of rotation
    int             unit_is_cart;           //  true, if viewing direction is specified in coordinates
    int             width;                  //  new width
    int             height;                 //  new height
};

typedef struct perspective_Prefs pPrefs;


struct optVars{                             //  Indicate to optimizer which variables to optimize
    int             hfov;                   //  optimize hfov? 0-no 1-yes , etc
    int             yaw;
    int             pitch;
    int             roll;
    int             a;
    int             b;
    int             c;
    int             d;
    int             e;
    int             shear_x;
    int             shear_y;
};

typedef struct optVars optVars;


enum{                                       // Enumerates for stBuf.seam
    _middle,                                // seam is placed in the middle of the overlap
    _dest                                   // seam is places at the edge of the image to be inserted
};

enum{                                       // Enumerates for colcorrect
    _colCorrectImage    = 1,
    _colCorrectBuffer   = 2,
    _colCorrectBoth     = 3,
};

struct stitchBuffer{                        //  Used describe how images should be merged
    char            srcName[256];           //  Buffer should be merged to image; 0 if not.
    char            destName[256];          //  Converted image (ie pano) should be saved to buffer; 0 if not
    int             feather;                //  Width of feather
    int             colcorrect;             //  Should the images be color corrected?
    int             seam;                   //  Where to put the seam (see above)
};

typedef struct stitchBuffer stBuf;

struct panControls{                         //  Structure for realtime Panoeditor
    double          panAngle;               //  The amount by which yaw/pitch are changed per click
    double          zoomFactor;             //  The percentage for zoom in/out
};

typedef struct panControls panControls;


enum{                                       // Enumerates for aPrefs.mode
    _readControlPoints,
    _runOptimizer,
    _insert,
    _extract,
    _useScript      = 8,                     // else use options
};

struct adjust_Prefs{                        //  Preferences structure for tool adjust
    pt_int32        magic;                  //  File validity check, must be 50
    pt_int32        mode;                   //  What to do: create Panorama etc?
    Image           im;                     //  Image to be inserted/extracted
    Image           pano;                   //  Panorama to be created/ used for extraction

    stBuf           sBuf;
    fullPath        scriptFile;             // On Mac: Cast to FSSpec; else: full path to scriptFile
    int             nt;                     // morphing triangles
    PTTriangle     *ts;                     // Source triangles
    PTTriangle     *td;                     // Destination triangles

    int interpolator;
    double gamma;
};

typedef struct adjust_Prefs aPrefs;


union panoPrefs{
    cPrefs          cP;
    pPrefs          pP;
    rPrefs          rP;
    aPrefs          aP;
    panControls     pc;
};

typedef union panoPrefs panoPrefs;


struct size_Prefs{                          //  Preferences structure for 'pref' dialog
    pt_int32        magic;                  //  File validity check; must be 70
    int             displayPart;            //  Display cropped/framed image ?
    int             saveFile;               //  Save to tempfile? 0-no, 1-yes
    fullPath        sFile;                  //  Full path to file (short name)
    int             launchApp;              //  Open sFile ?
    fullPath        lApp;                   //  the Application to launch
    int             interpolator;           //  Which interpolator to use 
    double          gamma;                  //  Gamma correction value
    int             noAlpha;                //  If new file is created: Don't save mask (Photoshop LE)
    int             optCreatePano;          //  Optimizer creates panos? 0  no/ 1 yes
};

typedef struct size_Prefs sPrefs;


struct controlPoint{                        //  Control Points to adjust images
    int             num[2];                 //  Indices of Images 
    double          x[2];                   //  x - Coordinates 
    double          y[2];                   //  y - Coordinates 
    int             type;                   //  What to optimize: 0-r, 1-x, 2-y
};

typedef struct controlPoint controlPoint;

struct CoordInfo{                           //  Real World 3D coordinates
    int             num;                    //  auxilliary index
    double          x[3];
    int             set[3];
};

typedef struct CoordInfo CoordInfo;

struct transformCoord{                      // 
    int             nump;                   //  Number of p-coordinates
    CoordInfo      *p;                      //  Coordinates "as is"
    int             numr;                   //  Number of r-coordinates
    CoordInfo      *r;                      //  Requested values for coordinates
};

typedef struct transformCoord transformCoord;

struct  tMatrix{
    double          alpha;
    double          beta;
    double          gamma;
    double          x_shift[3];
    double          scale;
};

typedef struct tMatrix tMatrix;


struct MakeParams{                          //  Actual parameters used by Xform functions for pano-creation
    double          scale[2];               //  scaling factors for resize;
    double          shear[2];               //  shear values
    double          rot[2];                 //  horizontal rotation params
    void           *perspect[2];            //  Parameters for perspective control functions
    double          rad[6];                 //  coefficients for polynomial correction (0,...3) and source width/2 (4) and correction radius (5) 
    double          mt[3][3];               //  Matrix
    double          distance;
    double          horizontal;
    double          vertical;
};

struct LMStruct{                            // Parameters used by the Levenberg Marquardt-Solver
    int             m;
    int             n;
    double         *x;
    double         *fvec;
    double          ftol;
    double          xtol;
    double          gtol;
    int             maxfev;
    double          epsfcn;
    double         *diag;
    int             mode;
    double          factor;
    int             nprint;
    int             info;
    int             nfev;
    double         *fjac;
    int             ldfjac;
    int            *ipvt;
    double         *qtf;
    double         *wa1;
    double         *wa2;
    double         *wa3;
    double         *wa4;
};

// function to minimize in Levenberg-Marquardt solver
typedef     int (*lmfunc)();

struct triangle
{
    int             vert[3];                //  Three vertices from list
    int             nIm;                    //  number of image for texture mapping
};

typedef struct triangle triangle;


struct AlignInfo{                           // Global data structure used by alignment optimization
    Image          *im;                     // Array of Pointers to Image Structs
    optVars        *opt;                    // Mark variables to optimize
    int             numIm;                  // Number of images 
    controlPoint   *cpt;                    // List of Control points
    triangle       *t;                      // List of triangular faces
    int             nt;                     // Number of triangular faces
    int             numPts;                 // Number of Control Points
    int             numParam;               // Number of parameters to optimize
    Image           pano;                   // Panoramic Image decription
    stBuf           st;                     // Info on how to stitch the panorama
    void           *data;
    lmfunc          fcn;
    sPrefs          sP; 
    CoordInfo      *cim;                    // Real World coordinates
};

typedef struct AlignInfo AlignInfo;

struct OptInfo{
    int             numVars;                //  Number of variables to fit
    int             numData;                //  Number of data to fit to
    int (*SetVarsToX)(double *x);           //  Translate variables to x-values
    int (*SetXToVars)(double *x);           //  and reverse
    lmfunc          fcn;                    //  Levenberg Marquardt function measuring quality
    char            message[256];           //  info returned by LM-optimizer
};

typedef struct OptInfo OptInfo;


struct VRPanoOptions{
    int             width;
    int             height;
    double          pan;
    double          tilt;
    double          fov;
    int             codec;
    int             cquality;
    int             progressive;
};

typedef struct VRPanoOptions VRPanoOptions;


struct MultiLayerImage{
    int             numLayers;
    Image          *Layer;
    PTRect          selection;
};

typedef struct MultiLayerImage MultiLayerImage;

// Transformation function type (we have only one...)
typedef     void (*trfn)( double x_dest,double  y_dest, double* x_src, double* y_src, void* params );

// Function descriptor to be executed by exec_function
struct fDesc {
    trfn            func;                   //  The function to be called
    void           *param;                  //  The parameters to be used
};

typedef struct fDesc fDesc;


// Gamma Correction
struct PTGamma{
    double         *DeGamma;
    unsigned short *Gamma;
    int             ChannelSize;
    int             ChannelStretch;
    int             GammaSize;
};

typedef struct PTGamma PTGamma;

#endif
