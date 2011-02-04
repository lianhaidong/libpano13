/*
 *  Clone of PTOptimizer
 *
 *  Based on code found in Helmut Dersch's panorama-tools
 *
 *  Dec 2003
 *
 *  Bruno Postle <bruno at postle.net>
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

// gcc -oPTOptimizer -I/usr/include/pano13 -lpano13 PTOptimizer.c


#include "filter.h"
#include <stdio.h>
#include <getopt.h>

#define PT_OPTIMIZER_VERSION "PTOptimizer Version " VERSION ", written by Helmut Dersch\n"
#define PT_OPTIMIZER_USAGE   "Usage:\n%s [options] <filename>\n\n"\
                             "\t-o <filename> --output=<filename> File to create. Implies --overwrite=0 \n"\
                             "\t-w [0|1]      --overwrite=[0|1]   Overwrite input file, defaults to 1 \n"\
                             "\t-x            --experimental      Use experimental code, if it exists\n"\
                             "\n\n\n"

//static  AlignInfo	*g;
int CheckParams( AlignInfo *g );


int main(int argc,char *argv[])
{
    aPrefs      aP;

    char*       script;
    OptInfo     opt;
    AlignInfo   ainf;

    char option;
    int optionIndex;
    char *outputFileName = NULL;
    char *inputFileName = NULL;

    // Do we overwrite the input file?
    int overwrite = 1;

    // This variable will control the executoin of experimental code.
    // If non-zero, then we try "new" things. Might result in 
    // buggy behaviour

    int experimentalMode = 0;

    // Struct for long options to command line.
    // set getopt(3) for a description of how to use
    // 
    //           struct option {
    //               const char *name;
    //               int         has_arg;
    //               int        *flag;
    //               int         val;
    //           };
    //
    struct option long_options[] = {
        {"overwrite", 2, NULL, 'w'},
        {"output", 2, NULL, 'o'},
        {"experimental", 0, NULL, 'x'},
        {0, 0, 0, 0}
    };

    while ((option = getopt_long(argc, argv, "o:wo::x",
                            long_options, &optionIndex) ) >= 0) {
        switch (option) {
        case 'o':
            outputFileName = optarg;
            break;
        case 'w':
            overwrite = atoi(optarg);
            break;
        case 'x':
            fprintf(stderr, "Experimental mode. It might be buggy, you have been warned\n");
            experimentalMode = 1;
        default:
            fprintf(stderr, PT_OPTIMIZER_USAGE, argv[0]);
            exit(1);
            break;
        }
    }

    // Verify options
    if (outputFileName &&
        overwrite ) {
        fprintf(stderr, "Output filename specified. It will not overwrite input file\n");
        overwrite = 0;
    }
    if (outputFileName == NULL && 
        !overwrite) {
        outputFileName = ""; // the string "" will be statically allocated in the heap
    }

    //fullPath  outfile;
    if(optind == argc)
    {
        printf(PT_OPTIMIZER_VERSION);
        printf(PT_OPTIMIZER_USAGE, argv[0]);
        exit(1);
    }
    inputFileName = argv[optind];
    if (outputFileName == NULL) {
        // Make output file name the same as the input if it has not been set yet
        outputFileName = inputFileName;
    } 
	//fullPath	outfile;

    SetAdjustDefaults(&aP);

    script = LoadScript(inputFileName);
    if( script != NULL )
    {
        if (ParseScript( script, &ainf ) == 0)
        {
            if( CheckParams( &ainf ) == 0 )
            {
                ainf.fcn    = fcnPano;
                
                SetGlobalPtr( &ainf ); 
                
                opt.numVars         = ainf.numParam;
                opt.numData         = ainf.numPts;
                opt.SetVarsToX      = SetLMParams;
                opt.SetXToVars      = SetAlignParams;
                opt.fcn         = ainf.fcn;
                *opt.message        = 0;

                RunLMOptimizer( &opt );
                ainf.data       = opt.message;

                WriteResults( script, outputFileName, &ainf, distSquared, 0);
                exit(0);
            }

      //TODO: if optCreatePano is 1 then should call stitcher  OR  the option removed
      //if (ainf.sP.optCreatePano == 1)
      //{
      //   Stitch();
      //}
            DisposeAlignInfo( &ainf );
        }
        free( script );
    }
    exit(1);
}

