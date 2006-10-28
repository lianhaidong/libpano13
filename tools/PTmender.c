/*
 *  PTmender
 *
 *  Based on the program PTStitcher by Helmut Dersch.
 *
 *  Dec 2005
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


// TODO
//    Create_Panorama requires some floating point assembly to be interpreted

#define __DEBUG__

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <dirent.h>
#include <unistd.h>

#include "tiffio.h"
#include "filter.h"
#include "panorama.h"

#include "PTmender.h"
#include "PTcommon.h"
#include "file.h"
#include "ColourBrightness.h"


// Global variables for the program


int ptDebug = 0;

#define PT_MENDER_VERSION  "PTmender Version " VERSION ", originally written by Helmut Dersch, rewritten by Daniel German\n"

int sorting_function(const void *, const void *);
int hasPathInfo(char *aName);


int main(int argc,char *argv[])
{

  printf(PT_MENDER_VERSION);
  fprintf(stderr, "This program is deprecated. Please use PTremapper instead");
  exit(-1);

}


