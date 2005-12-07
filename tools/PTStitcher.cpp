/*
 *  PTStitcher
 *
 *  Based on code found in Helmut Dersch's panorama-tools
 *  to duplicate the functionality of original program
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
 */

// gcc -oPTStitcher -I/usr/include/pano12 -lpano12 PTStitcher.c

//#include "filter.h"
extern "C" {
#include "filter.h"
};
//#include <tchar.h>

//static  AlignInfo	*g;
//int CheckParams( AlignInfo *g );


/*
Useage
PTStitcher -o outputpano script.txt image1 image2 ....

The outputpano describes the location for the result panorama, ie what 
PTStitcher asks for when run interactively. This file should not exist yet.
The scriptfile is identified via extension 'txt' and need not appear
as second entry, nor has to be named 'script'. 
All input images follow, and are processed in the order they appear 
in the command line.
*/

int WriteOutScript( char* res, fullPath* scriptFile );

//int _tmain(int argc, _TCHAR* argv[])
int main(int argc,char *argv[])
{
	bool		bScript	=	false;
	bool		bPano	=	false;
    char       *Script  =   NULL;
	fullPath	outfile;
	fullPath	scriptfile;
	fullPath	infile;
    file_spec 	fnum;

	int			i;

	aPrefs		aP;
	AlignInfo	ainf;
    aPrefs     *paP     = NULL;;

//	int	 		destwidth, destheight;

    memset( &aP, 0, sizeof(aPrefs) );
    memset( &ainf, 0, sizeof(AlignInfo) );
    memset( &outfile, 0, sizeof(fullPath) );
    memset( &scriptfile, 0, sizeof(fullPath) );
    memset( &infile, 0, sizeof(fullPath) );
	
	// -o for output image
	// script file ends in .txt
	// the remaining are images to process but the images could be listed in the
	//     i line of the script file
	// ToDo: handle images passed in as argv  ordinal
	for(i=1; i<argc; i++)
	{
		if( false == bPano && (0 == strncmp(argv[i], "-o", 2)) && i<argc+1 )
		{
			if( 0 == StringtoFullPath(&outfile, argv[++i]) )
			{
                //ToDo: if file exist -Prompt Overwrite, New, or Cancel
                //  handle cases.
                bPano = true;
			}
		}
		else if( false == bScript && TRUE == IsTextFile(argv[i]) )
		{
			if( 0 == StringtoFullPath(&scriptfile, argv[i]) )
			{
                bScript = true;
			}
		}
	}

	if(false == bScript)
	{
		StringtoFullPath(&scriptfile, "script.txt");
	}

    
	if( myopen( &scriptfile, read_text, fnum ) )
	{
		PrintError("Error Opening Scriptfile (%s)", &scriptfile);
		exit(1);
	}
    myclose(fnum);


	if(false == bPano)
	{
		if( 0 != SaveFileAs(&outfile, "Save pano as", "pano.tif") )
			exit(1);
	}

	// panotools allocates memory for script need to call panotools to dealocate
	Script = LoadScript( &scriptfile );
	
	if( Script != NULL )
	{
        // create a copy of script file
        makeTempPath(&infile);
        if( 0 == WriteOutScript( Script, &infile ) )
        {
		    SetAdjustDefaults(&aP);
		    //StringtoFullPath(&infile, argv[1]);

		    if (ParseScript( Script, &ainf ) == 0)
		    {
                for(i=0; i<ainf.numIm; i++)
                {
                    // Read an Adjust line 'i' then mark it as processed by 
                    // changing it to '!'.  That way the next time we get the 
                    // next 'i' line.
                    paP = readAdjustLine(&infile);
                    if(paP)
                    {
                        /*
                        >> makeTempPath (out.tiff) to (_PTStitcher_tmp_7)
                        >> GetFullPath (p)
                        >> TIFFOpen, (_PTStitcher_tmp_7)(w)
                        >> Progress (Converting Image 0)
                        >> readImage (a1.tif)
                        >> Progress (5)
                        >> TIFFSetField
                        >> TIFFSetField
                        (last message repeated 7 more times, but not printed.)
                        >> 80 = TIFFScanlineSize
                        >> mymalloc (1600)
                        >> CopyPosition
                        >> CopyPosition
                        >> MakePano
                        >> ARGBtoRGBA
                        >> ARGBtoRGBA
                        (last message repeated 18 more times, but not printed.)
                        >> TIFFWriteScanline
                        >> TIFFWriteScanline
                        (last message repeated 18 more times, but not printed.)
                        >> Progress (100)
                        >> TIFFClose
                        >> myfree (00048720)
                        */
                        
                        // ToDo: call PanoTools instead to dealocate memory.
                        //free( paP );
                        paP = NULL;
                    }
                }
		    }
        }
		// ToDo: call PanoTools instead to dealocate memory.
		//free( Script );
		Script = NULL;
	}
	exit(1);
}

int WriteOutScript( char* res, fullPath* scriptFile )
{
	fullPath	sfile;
	file_spec 	fnum;
	size_t		count;

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

	return 0;

_writeError:
	return -1;
}
