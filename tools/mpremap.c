/* mpremap - remap motion pictures
   Copyright (C) 2007 - Helmut Dersch  der@fh-furtwangen.de
   
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

// Updated to libPano13 by Jim Watters

#include "filter.h"
#include "getopt.h"
#include "ppmstream.h"
#include "string2.h"

#include <fcntl.h>


#define MPREMAP_VERSION "0.2a"

void stitchImages( char** files, int nim, char *pano, char *script, int stitch);
void extractImages( char** files, int nim, char *pano, char *script);
void addAlphato( Image *im );
int preprocess(const char* s, const char* d);
Image * cutImage(Image* im, PTRect* s);


#define PrintError(x) {fprintf(stderr,x);fprintf(stderr,"\n");fflush(stderr);}



void usage(){
  fprintf(stderr, "mpremap version %s\n", MPREMAP_VERSION );
  fprintf(stderr, "Usage:\n");
  fprintf(stderr, "mpremap -f scriptfile -o output  file1 file2 ...\n");
  fprintf(stderr, "To process piped PPM-stream:\n");
  fprintf(stderr, "mpremap -f scriptfile -o - -\n");
  fprintf(stderr, "Options:\n");
  fprintf(stderr, "-f Scriptfile\n");
  fprintf(stderr, "-o Output, - stdout\n");
  fprintf(stderr, "-s stitch\n");
  fprintf(stderr, "-e extract\n");
  fprintf(stderr, "-b num blur using num images (only extract mode)\n");
  fprintf(stderr, "file1,.. input, - stdin\n");
}


int quality=80;
int nblur=1;
int idx_in = 0;


int main( int argc, char **argv ){

  char** files=NULL;            // The list of files to stitch
  char*  pano=NULL;             // The output
  char*  script="script.txt";   // Scriptfile
  int    nim = 0,c;             // Number of images
  int    stitch=FALSE;
  int    extract=FALSE;

  while( (c = getopt (argc, argv, "eso:f:F:q:b:h")) != EOF ){
    switch (c){
      case 'o': 
        pano = optarg;
        break;
      case 'f':
        script=optarg;
        break;
      case 'F':
        script=tmpnam(NULL);
        sfileout( sreplace(optarg, "_:", " \n"), script );
        break;
      case 's':
        stitch=TRUE;
        break;
      case 'e':
        extract=TRUE;
        break;
      case 'b':
        nblur = atoi(optarg);
        break;
      case 'q':
        quality=atoi(optarg);
        if(quality<0 || quality>100)
          quality=80;
        break;
      case 'h': 
        usage();
        exit(0);
    }
  }

  while (optind < argc){
    files = (char**) realloc(files, (++nim)*sizeof(char*));
    files[nim-1]=argv[optind++];
  }     
      
  // Check Scriptfile
  if( pano==NULL || script==NULL || files==NULL ){
    usage();
    exit(0);
  }

#ifdef __Win__
  setmode(fileno(stdin), O_BINARY);
  setmode(fileno(stdout), O_BINARY);
#endif

  if(extract)
    extractImages( files, nim, pano, script );
  else
    stitchImages( files, nim, pano, script, stitch );
  return 0;
}



// Save image to file
// name "-" ppm,stdout
// extension ".jpg" file jpeg
// default ppm
int saveImage(char* fname, Image* im){
  static int idx=0;
  
  if(strcmp(fname, "-")){
    char* c=(char*)malloc(strlen(fname)+10);
    sprintf(c,fname,idx++);
    if(sendswith(c,".jpg", TRUE)){
      writeJPEG(im,(fullPath *)c,quality,0);
    }else{
      FILE *out = fopen(c,"wb");
      if(out==NULL || putImageToPPMStream( out, im )){
        PrintError( "Could not save result");
        return -1;
      }
      fclose(out);
    }
    free(c);
  }else{
    if(putImageToPPMStream( stdout, im )){
      PrintError( "Could not save result");
      return -1;
    }
  }
  return 0;
}
  

Image *extractImage(aPrefs* a, Image* pano){
  TrformStr   Tr;
  int         destheight;
  int         destwidth;
  
  memset(&Tr, 0, sizeof(TrformStr));
  Tr.dest = (Image*)malloc(sizeof(Image));
  SetImageDefaults( Tr.dest );    
        
  Tr.src      = pano;
  Tr.mode     = _honor_valid; 
  Tr.success  = 1;

  destheight  = a->im.height;
  destwidth   = a->im.width;

  if( SetDestImage( &Tr, destwidth, destheight) != 0){
    // fprintf(stderr,"Could not allocate %ld bytes",Tr.dest->dataSize );
    return NULL;
  }
  CopyPosition( Tr.dest, &(a->im) );

  Tr.mode         |= _honor_valid;
  if( a->pano.hfov == 360.0 )
    Tr.mode       |= _wrapX;
      
  Tr.interpolator = a->interpolator;
  Tr.gamma      = a->gamma;
  Tr.fastStep   = a->fastStep;

  ExtractStill( &Tr,  a );
  if(Tr.success)
    return Tr.dest;
  else{
    myfree((void**)Tr.dest->data);
    free(Tr.dest);
    return NULL;
  }
}

INLINE int  hasmoved( Image *a, Image *b ){
  return  a->hfov != b->hfov || 
      a->yaw  != b->yaw  || 
      a->pitch != b->pitch || 
      a->roll != b->roll;
} 


aPrefs* interpolate_position(aPrefs *a, aPrefs* b, int n){
  aPrefs* ap = (aPrefs*)malloc(n*sizeof(aPrefs));
  int i;
  Image *im;
  for(i=0; i<n; i++){ 
    memcpy(&ap[i], a, sizeof(aPrefs));
    im = &ap[i].im;
    im->yaw   = a->im.yaw   + i*(b->im.yaw-a->im.yaw) /(double)n;
    im->pitch = a->im.pitch + i*(b->im.pitch-a->im.pitch)/(double)n;
    im->roll  = a->im.roll  + i*(b->im.roll-a->im.roll) /(double)n;
    im->hfov  = a->im.hfov  + i*(b->im.hfov-a->im.hfov) /(double)n;
  }
  return ap;
}


Image* imaverage(Image** dim, int n){
  Image *im = (Image*)malloc(sizeof(Image));
  int x,y,i,fs=dim[0]->bitsPerPixel==32 || dim[0]->bitsPerPixel==64;
  unsigned char** d=(unsigned char**)malloc(n*sizeof(unsigned char*)), *dst;
  memcpy(im, dim[0], sizeof(Image));
  im->data=(unsigned char**)mymalloc(im->dataSize);
  
  for(y=0; y<im->height; y++){
    for(i=0; i<n; i++){
      d[i] = (unsigned char*)*(dim[i]->data) + y*dim[i]->bytesPerLine;
    }
    dst = (unsigned char*)*im->data + y*im->bytesPerLine;;
    for(x=0; x<im->width; x++){
      double r=0,g=0,b=0;
      for(i=0; i<n; i++){
        if(fs)d[i]++;
        r += *(d[i]++);
        g += *(d[i]++);
        b += *(d[i]++);
      }
      r /= n; g /= n; b /= n;
      if(fs)dst++;
      DBL_TO_UC( *dst, r ); dst++;
      DBL_TO_UC( *dst, g ); dst++;
      DBL_TO_UC( *dst, b ); dst++;
    }
  }
  free(d);
  return im;
}


void extractImages( char** files, int nim, char *pano, char *script){
  int     i   =0;
  int     nr  =0;
  Image   source;
  Image  *dest;
  aPrefs *aPtr = NULL;
  aPrefs  aPref_h;
  
  // make temporary copy of scriptfile
  char *tscript = tmpnam(NULL);
  preprocess(script, tscript);
  
  SetImageDefaults( &source );
  source.data = NULL;

  while(TRUE){
    if(aPtr==NULL) // Not null if continue case (see below)
      aPtr = readAdjustLine((fullPath*)tscript );
    if(aPtr == NULL){
      preprocess(script, tscript);
      aPtr = readAdjustLine( (fullPath*)tscript );
      if(aPtr == NULL){
        PrintError("Could not read Scriptfile");
        goto _extract_exit;
      }
    }else{
      if(i==0)
        memcpy(&aPref_h, aPtr, sizeof(aPrefs));
    }
    i++;

    // Load new file unless image name == "same"
    if(strcmp(aPtr->im.name, "same") || i==1){ // Not same: load new image
      if(source.data != NULL){
        myfree((void**)source.data);
        source.data = NULL;
      }   

      if( strcmp(files[0], "-") ){  // Filenames given
        if( strchr( files[0], '%' ) != NULL){ // Numbered Images
          char* c=(char*)malloc(strlen(files[0])+10);
          sprintf(c,files[0],idx_in++);
          if( panoImageRead( &source, (fullPath*)c  )==0 ){ // No more images, return
            if(idx_in == 1){ // No images: error
              fprintf(stderr,"Could not read image %s",c);
            }
            free(c);
            goto _extract_exit;
          }
          free(c);
        }else{          
          if( nr >= nim ) break;    // No more input images
          if( panoImageRead( &source, (fullPath*)(files[nr++]) )==0 ){
            fprintf(stderr,"Could not read image %s",files[nr-1]);
            goto _extract_exit;
          }
        }
      }else{ // read file from stdin
        int rh = getImageFromPPMStream( stdin, &source );
        if(rh<0){ // Error
          PrintError("Error reading from stream.");
          goto _extract_exit;
        }else if(rh>0){ // No more Images in stream
          break;
        }
      }   
    }
    
    // Skip image if image name == "skip"
    if(strcmp(aPtr->im.name, "skip")==0){
      if(aPtr != NULL){
        free(aPtr);
        aPtr=NULL;
      }
      continue;
    } 

    if( aPtr->im.width == 0 ){
        aPtr->im.width = 320 ;
    }
    if(  aPtr->im.height == 0 ){
        aPtr->im.height = aPtr->im.width * 3 / 4;
    }
        
    // Set pano-params to src-image irrespective of prefs
    aPtr->pano.width  = source.width;         //  width of panorama
    aPtr->pano.height = source.height;        //  height of panorama
      
    CopyPosition( &source, &(aPtr->pano) );
    addAlphato( &source ); 

    if( nblur > 1 && hasmoved( &aPtr->im, &aPref_h.im ) ){
      int k;
      aPrefs *aps = interpolate_position(aPtr, &aPref_h, nblur);
      Image **dim = (Image**)malloc(nblur*sizeof(Image*));
      for(k=0; k<nblur; k++){
        fprintf(stderr, "mpremap: converting image %d blur %d\n",i,k);
        fflush(stderr);
        dim[k] = extractImage(&aps[k],&source);
      }
      fprintf(stderr, "mpremap: blurring image %d\n",i);
      fflush(stderr);
      dest = imaverage(dim, nblur);
      for(k=0; k<nblur; k++){
        myfree((void**)dim[k]->data);
        free(dim[k]);
      }
      free(dim);
      free(aps);
    }else{    
      fprintf(stderr, "mpremap: converting image %d\n",i);
      fflush(stderr);
      dest =  extractImage(aPtr,&source);
    }

    if(dest==NULL){
      PrintError("Error converting image.");
      goto _extract_exit;
    }

    // Output result here
    // Crop Image if required
    if( aPtr->im.cP.cutFrame ){
      Image* out = cutImage(dest, &aPtr->im.selection);
      if(out==NULL){
        PrintError("Error cutting rectangle.");
        goto _extract_exit;
      }
      myfree((void**)dest->data);
      free(dest);
      dest = out;
    }

    // Output Image
    if( saveImage(pano, dest) ){
      PrintError( "Error saving result");
      goto _extract_exit;
    }
          
    myfree((void**)dest->data);
    free(dest);
    if(aPtr != NULL){ 
      memcpy(&aPref_h, aPtr, sizeof(aPrefs));     
      free(aPtr);
      aPtr=NULL;
    }
  } // Tr.success 


  // Clean up; delete tempfiles
_extract_exit:
  mydelete((fullPath*)tscript);
}


Image * cutImage(Image* im, PTRect* s){
  int y;
  Image *output = (Image*)malloc(sizeof(Image));
  memcpy(output, im, sizeof(Image));          
  output->width         = s->right-s->left;
  output->height        = s->bottom-s->top;
  output->bytesPerLine  = output->width*output->bitsPerPixel/8;
  output->dataSize      = output->bytesPerLine*output->height;
  output->data          = (unsigned char**)mymalloc( output->dataSize );
  if(output->data == NULL){
    PrintError("Not enough memory");
    return NULL;
  }

  for(y=0; y<output->height; y++){
    memcpy( (*output->data)+y*output->bytesPerLine,
        (*im->data)+(y+s->top)*im->bytesPerLine+s->left*im->bitsPerPixel/8,
        output->bytesPerLine);
  }
  return output;
}


Image *insertImage(aPrefs* a, Image* im){
  TrformStr   Tr;
  
  memset(&Tr, 0, sizeof(TrformStr));
  Tr.dest = (Image*)malloc(sizeof(Image));
  SetImageDefaults( Tr.dest );    
        
  Tr.src      = im;
  Tr.mode     = _honor_valid; 
  Tr.success  = 1;

  Tr.dest->height = a->pano.height;
  Tr.dest->width  = a->pano.width;

  Tr.dest->bitsPerPixel = im->bitsPerPixel;
  Tr.dest->bytesPerLine = Tr.dest->width * Tr.dest->bitsPerPixel/8;
  Tr.dest->dataSize     = Tr.dest->bytesPerLine * Tr.dest->height;
  Tr.dest->data         = (unsigned char**)mymalloc( Tr.dest->dataSize );
  if(    Tr.dest->data == NULL){
    PrintError("Not enough memory");
    exit(0);
  }

  Tr.dest->selection.left   = 0;
  Tr.dest->selection.right  = Tr.dest->width;
  Tr.dest->selection.top    = 0;
  Tr.dest->selection.bottom = Tr.dest->height;
  CopyPosition( Tr.dest, &(a->pano) );
  a->pano.width  = Tr.dest->width;
  a->pano.height = Tr.dest->height;

  Tr.interpolator = a->interpolator;
  Tr.gamma        = a->gamma;
  Tr.fastStep     = a->fastStep;
  Tr.mode         |= _honor_valid;
  if( a->pano.hfov == 360.0 )
    Tr.mode       |= _wrapX;
      
  MakePano( &Tr,  a );

  if(Tr.success)
    return Tr.dest;
  else{
    myfree((void**)Tr.dest->data);
    free(Tr.dest);
    return NULL;
  }
}
  
void stitchImages( char** files, int nim, char *pano, char *script, int stitch){
  int     i  = 0;
  int     nr = 0;
  Image   source;
  Image  *dest = NULL;
  aPrefs *aPtr = NULL;
  aPrefs  aPref_h;

  // make temporary copy of scriptfile
  char *tscript = tmpnam(NULL);
  preprocess(script, tscript);
  
  SetImageDefaults( &source );
  source.data = NULL;

  while(TRUE){
    if(aPtr==NULL) // Not null if continue case (see below)
      aPtr = readAdjustLine((fullPath*)tscript );
    if(aPtr == NULL){
      preprocess(script, tscript);
      aPtr = readAdjustLine( (fullPath*)tscript );
      if(aPtr == NULL){
        PrintError("Could not read Scriptfile");
        return;
      }
    }else{
      if(i==0)
        memcpy(&aPref_h, aPtr, sizeof(aPrefs));
    }
    i++;

    // Load new file unless image name == "same"
    if(strcmp(aPtr->im.name, "same") || i==1){ // Not same: load new image

      if(source.data != NULL){
        myfree((void**)source.data);
        source.data = NULL;
      }   


      if( strcmp(files[0], "-") ){  // Filenames given
        if( strchr( files[0], '%' ) != NULL){ // Numbered Images
          char* c=(char*)malloc(strlen(files[0])+10);
          sprintf(c,files[0],idx_in++);
          if( panoImageRead( &source, (fullPath*)c )==0 ){ // No more images, return
            if(idx_in == 1){ // No images: error
              fprintf(stderr,"Could not read image %s",c);
            }
            free(c);
            return;
          }
          free(c);
        }else{
          if( nr >= nim ) break;    // No more input images
          if( panoImageRead( &source, (fullPath*)(files[nr++]) )==0 ){
            fprintf(stderr,"Could not read image %s",files[nr-1]);
            return;
          }
        }
      }else{ // read file from stdin
        int rh = getImageFromPPMStream( stdin, &source );
        if(rh<0){ // Error
          PrintError("Error reading from stream.");
          return;
        }else if(rh>0){ // No more Images in stream
          break;
        }
      }   
    }

    
    // Skip image if image name == "skip"
    if(strcmp(aPtr->im.name, "skip")==0){
      if(aPtr != NULL){
        free(aPtr);
        aPtr=NULL;
      }
      continue;
    } 

    
/* NOT implimented at this time
  //int     colcorrect = 0, brcorrect = 0, cabcorrect = 0; // Color correction requested?
    colcorrect = aPtr->sBuf.colcorrect;
    if( aPtr->pano.cP.radial )
      brcorrect  = (int)aPtr->pano.cP.radial_params[0][2] + 1;
    if( aPtr->pano.cP.horizontal )
      cabcorrect = (int)aPtr->pano.cP.horizontal_params[0] + 1;
*/

    
    // Crop Image if required
    if( aPtr->im.cP.cutFrame ){
      if( CropImage( &source, &aPtr->im.selection ) == 0 ){ // Cropping 
        aPtr->im.selection.top = aPtr->im.selection.bottom =
        aPtr->im.selection.right = aPtr->im.selection.left = 0;
        aPtr->im.cP.cutFrame = FALSE;
      }
    } 

    aPtr->im.width  = source.width;
    aPtr->im.height = source.height;

    if( aPtr->pano.width == 0 && aPtr->im.hfov != 0.0){
      aPtr->pano.width = aPtr->im.width * aPtr->pano.hfov / aPtr->im.hfov;
      aPtr->pano.width/=10; aPtr->pano.width*=10;
    }
    if( aPtr->pano.height == 0 )
      aPtr->pano.height = aPtr->pano.width/2;

    CopyPosition( &source,  &(aPtr->im) );
    memcpy( &source.selection, &aPtr->im.selection, sizeof(PTRect) );

    addAlphato( &source ); 

    if(dest!=NULL){
      myfree((void**)dest->data);
      free(dest);
    }

    fprintf(stderr, "mpremap: converting image %d\n",i); 
    fflush(stderr);
    dest = insertImage(aPtr, &source);
    
    
    if( dest == NULL ) {
      PrintError("Error converting image");
      return;
    }
    
    
    // Stitch images; Proceed only if panoramic image valid

    if( *(aPtr->sBuf.srcName) != 0 ){ // We have to merge in one images
    // Load the bufferimage
      if( LoadBufImage( &aPtr->pano, aPtr->sBuf.srcName, 1 ) != 0 ){
         goto _insert_exit;
      }

      if( HaveEqualSize( &aPtr->pano, dest )){
        // At this point we have two valid, equally sized images            
        // Do Colour Correction on one or both  images
        DoColorCorrection( dest, &aPtr->pano, aPtr->sBuf.colcorrect & 3);
        if( merge( dest , &aPtr->pano, aPtr->sBuf.feather, 0, aPtr->sBuf.seam ) != 0 ){
          PrintError( "Error merging images. Keeping Source" );
        }
      }       
      myfree( (void**)aPtr->pano.data );
    } // src != 0
          
    if( *(aPtr->sBuf.destName) != 0 ){ // save buffer image
      if( SaveBufImage( dest, aPtr->sBuf.destName )  )
        PrintError( "Could not save to Buffer. Most likely your disk is full");
    }
    
    if(!stitch){ // Output result here
      // Crop Image if required
      if( aPtr->pano.cP.cutFrame ){
        Image* output = cutImage(dest, &aPtr->pano.selection);
        if(output==NULL){
          PrintError("Error cutting rectangle.");
          goto _insert_exit;
        }

        myfree((void**)dest->data);
        free(dest);
        dest = output;
      }
      
      // Output Image
      if( saveImage(pano, dest) ){
        PrintError( "Error saving result");
        return;
      }
      
    }
    if(aPtr != NULL){
      free(aPtr);
      aPtr=NULL;
    }
    
  } // Tr.success 

  if(stitch){ // Output result here
    if( saveImage(pano, dest) ){
      PrintError( "Error saving result");
      return;
    }
  }

_insert_exit:
  // Clean up; delete tempfiles
  mydelete((fullPath*)"pano12.buf");
  mydelete((fullPath*)tscript);
}


// Add an alpha channel to the image, assuming rectangular or circular shape
// subtract frame 
void addAlphato( Image *im ){
  int     x,y,xc,yc,rc,rcs,BPP,delx,dely;
  UCHAR   *dx, *dy;
  PTRect    s;

  memcpy(&s, &im->selection, sizeof(PTRect));
  if( s.bottom == 0 ) s.bottom = im->height;
  if( s.right == 0 ) s.right = im->width;
  
  if( im->bitsPerPixel == 32 ){
#define PIXEL_TYPE UCHAR
    BPP = 4;
    if( im->format != _fisheye_circ ){ // Rectangle valid
      for(y=0,dy=*im->data; y<s.top; y++,dy+=im->bytesPerLine)
        for(x=0,dx=dy; x<im->width; x++,dx+=BPP)
          *(PIXEL_TYPE*)dx = 0;
      for(y=s.bottom,dy=*im->data+s.bottom*im->bytesPerLine; y<im->height; y++,dy+=im->bytesPerLine)
        for(x=0,dx=dy; x<im->width; x++,dx+=BPP)
          *(PIXEL_TYPE*)dx = 0;
      for(y=0,dy=*im->data; y<im->height; y++,dy+=im->bytesPerLine)
        for(x=0,dx=dy; x<s.left; x++,dx+=BPP)
          *(PIXEL_TYPE*)dx = 0;
      for(y=0,dy=*im->data; y<im->height; y++,dy+=im->bytesPerLine)
        for(x=s.right,dx=dy+s.right*BPP; x<im->width; x++,dx+=BPP)
          *(PIXEL_TYPE*)dx = 0;       
    }else{ //Circle inside selection
      xc = (s.right+s.left)/2;
      yc = (s.bottom+s.top)/2;
      rc = (s.left-s.right)/2;
      rcs = rc*rc;
      for(y=0,dy=*im->data; y<im->height; y++,dy+=im->bytesPerLine)
        for(x=0,dx=dy; x<im->width; x++,dx+=BPP){
          dely = y - yc; delx = x - xc;
          if( (dely*dely + delx*delx) > rcs )
            *(PIXEL_TYPE*)dx = 0;
        }
    }
  }else if( im->bitsPerPixel == 64 ){
#undef PIXEL_TYPE
#define PIXEL_TYPE USHORT
    BPP = 8;
    if( im->format != _fisheye_circ ){ // Rectangle valid
      for(y=0,dy=*im->data; y<s.top; y++,dy+=im->bytesPerLine)
        for(x=0,dx=dy; x<im->width; x++,dx+=BPP)
          *(PIXEL_TYPE*)dx = 0;
      for(y=s.bottom,dy=*im->data+s.bottom*im->bytesPerLine; y<im->height; y++,dy+=im->bytesPerLine)
        for(x=0,dx=dy; x<im->width; x++,dx+=BPP)
          *(PIXEL_TYPE*)dx = 0;
      for(y=0,dy=*im->data; y<im->height; y++,dy+=im->bytesPerLine)
        for(x=0,dx=dy; x<s.left; x++,dx+=BPP)
          *(PIXEL_TYPE*)dx = 0;
      for(y=0,dy=*im->data; y<im->height; y++,dy+=im->bytesPerLine)
        for(x=s.right,dx=dy+s.right*BPP; x<im->width; x++,dx+=BPP)
          *(PIXEL_TYPE*)dx = 0;       
    }else{ //Circle inside selection
      xc = (s.right+s.left)/2;
      yc = (s.bottom+s.top)/2;
      rc = (s.left-s.right)/2;
      rcs = rc*rc;
      for(y=0,dy=*im->data; y<im->height; y++,dy+=im->bytesPerLine)
        for(x=0,dx=dy; x<im->width; x++,dx+=BPP){
          dely = y - yc; delx = x - xc;
          if( (dely*dely + delx*delx) > rcs )
            *(PIXEL_TYPE*)dx = 0;
        }
    }
  }
}


//////////////////////// Some modifications to adjust.c in pano12 ////////////////////////////////
static int    CheckMakeParams( aPrefs *aP)
{

  if( (aP->pano.format == _rectilinear) && (aP->pano.hfov >= 180.0) )
  {
    PrintError("Rectilinear Panorama can not have 180 or more degrees field of view.");
    return -1;
  }
  if( (aP->im.format == _rectilinear) && (aP->im.hfov >= 180.0) )
  {
    PrintError("Rectilinear Image can not have 180 or more degrees field of view.");
    return -1;
  }
  if( (aP->mode & 7) == _insert ){
    if( (aP->im.format == _fisheye_circ ||  aP->im.format == _fisheye_ff) &&
        (aP->im.hfov > MAX_FISHEYE_FOV) ){
        fprintf(stderr,"Fisheye lens processing limited to fov <= %lg\n", MAX_FISHEYE_FOV);
        return -1;
    }
  }

  return 0;

}


////////////////////////////////////////////////////////////////////////////////////////////////////////

