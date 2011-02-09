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

void ZCombSetDisabled(void);
void ZCombSetEnabled(void);
void ZCombSetMaskType(int mt);
void ZCombSetFocusWindowHalfwidth(int fwh);
void ZCombSetSmoothingWindowHalfwidth(int swh);
void ZCombSetMaskFromFocusData(Image *im);
void ZCombSetGreenTo255(Image *im);
int  ZCombSeeImage(Image *im, char *filename);
