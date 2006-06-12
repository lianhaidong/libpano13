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

#ifndef VERS1
#define VERS1 0x2
#endif

#ifndef VERS2
#define VERS2 0x00
#endif

//version of preferences file, used to verify data
#ifndef VERSION
#define VERSION "2.8.4 "
#endif

#ifndef PTVERSION_NAME_LONG
#define PTVERSION_NAME_LONG "LongVersion"
#endif
#ifndef LONGVERSION
#define LONGVERSION VERSION ", Copyright (c) 1998-2001, H. Dersch, der@fh-furtwangen.de"
#endif

#ifndef PTVERSION_FILEVERSIONNUMBER
#define PTVERSION_FILEVERSIONNUMBER 2,8,4,0
#endif

#ifndef PTVERSION_NAME_FILEVERSION
#define PTVERSION_NAME_FILEVERSION "FileVersion"
#endif
#ifndef PTVERSION_FILEVERSION
#define PTVERSION_FILEVERSION VERSION "\0"
#endif

#ifndef PROGRESS_VERSION
#define PROGRESS_VERSION VERSION "\0"
#endif

#ifndef PTVERSION_NAME_LEGALCOPYRIGHT 
#define PTVERSION_NAME_LEGALCOPYRIGHT "LegalCopyright"
#endif
#ifndef PTVERSION_LEGALCOPYRIGHT 
#define PTVERSION_LEGALCOPYRIGHT "Copyright © 1999, 2000, 2001 Helmut Dersch\0"
#endif

#ifndef PTVERSION_NAME_COMMENT
#define PTVERSION_NAME_COMMENT "Comments"
#endif
#ifndef PTVERSION_COMMENT
#define PTVERSION_COMMENT "MinGW\0"
#endif
