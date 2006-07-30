/*
 *  file.h
 *
 *  Many of the routines are based on the program PTStitcher by Helmut
 *  Dersch.
 * 
 *  Copyright Helmut Dersch and Daniel M. German
 *  
 *  July 2006
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

#ifndef __FILE_H__

// these are defined in file.c

int panoImageRead(Image * im, fullPath * sfile);
int panoFileMakeTemp(fullPath * path);

// and these are defined in bmp.c, jpeg.c, hdrfile.c, png.c, and ppm.c
// but there is no point in creating a file for each one of them

int panoBMPRead( Image *im, fullPath *sfile );
int panoJPEGRead(Image * im, fullPath * sfile);
int panoHDRRead(Image *im, fullPath *sfile );
int panoPNGRead(Image *im, fullPath *sfile );
int panoPPMRead(Image * im, fullPath * sfile);


#endif
