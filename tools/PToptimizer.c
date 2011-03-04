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


/* The default behaviour of PToptimizer is to overwrite the input file.

   If the option --output=<filename> (-o) then write to such file. To
   avoid confusion this option always requires the output file name.

   Option --stdout (-s) output to standard output. Incompatible with
   --output.

   Option --overwrite=[0|1] (-o [0|1]. Because by default PToptimizer
   overwrites the output file, the option requires a parameter
   value. This way we know exactly what the user is trying to do.

   There is also a --experimental (-x) option to test the new
   execution paths (i.e. parser replacement).

*/
#define PT_OPTIMIZER_VERSION "PTOptimizer Version " VERSION ", written by Helmut Dersch\n"
#define PT_OPTIMIZER_USAGE   "Usage:\n%s [options] <filename>\n\n"\
    "\tWarning: if no options are specified, input file is overwritten with output.\n" \
    "\t-o <filename> --output=<filename> Write output to <filename>.\n" \
    "\t-s            --stdout            Write result to standard output.\n"\
    "\t-w [0|1]      --overwrite=[0|1]   Overwrite input file. \n"      \
    "\t-x            --experimental      Use experimental code, if it exists\n" \
    "\n"

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

    // output option indicated?
    int outputFileFlag = 0;

    // Do we overwrite the input file?
    int overwrite = 0;

    // do we get the stdout option?
    int toStdout = 0;

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
    // has_arg
    //   is: no_argument (or 0) if the option does not take an argument;
    //   required_argument (or 1) if the option requires an argument; or
    //   optional_argument (or 2) if
    //   the option takes an optional argument.
    //
    struct option long_options[] = {
        {"output", 1, NULL, 'o'},
        {"stdout", 0, NULL, 's'},
        {"overwrite", 1, NULL, 'w'},
        {"experimental", 0, NULL, 'x'},
        {0, 0, 0, 0}
    };

    while ((option = getopt_long(argc, argv, "o:sw:x",
                            long_options, &optionIndex) ) >= 0) {
        switch (option) {
        case 's':
            if (outputFileName != NULL) {
                fprintf(stderr, "--stdout (-s) option is incompatible with --output");
                fprintf(stderr, PT_OPTIMIZER_USAGE, argv[0]);
                exit(1);
            }
            toStdout = 1;
            break;
        case 'o':
            if (toStdout) {
                fprintf(stderr, "--stdout (-s) option is incompatible with --output");
                fprintf(stderr, PT_OPTIMIZER_USAGE, argv[0]);
                exit(1);
            }
            outputFileName = optarg;
            outputFileFlag = 1;
            break;
        case 'w':
            // we check that the option is 0 or 1
            if (strcmp(optarg, "0") != 0 &&
                strcmp(optarg, "1") != 0 ) {
                fprintf(stderr, "Invalid value for option --overwrite (-w) [%s]\n", optarg);
                fprintf(stderr, PT_OPTIMIZER_USAGE, argv[0]);
                exit(1);
            }
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
    //fullPath  outfile;
    if(optind == argc)
    {
        printf(PT_OPTIMIZER_VERSION);
        printf(PT_OPTIMIZER_USAGE, argv[0]);
        exit(1);
    }

    // we only take one filename as input
    inputFileName = argv[optind];

    // if no output file specified, then we force overwrite 
    // to be on (default behaviour of the old PToptimizer
    if (outputFileName == NULL && overwrite == 0) {
        overwrite = 1; 
    }

    // Make output file name the same as the input if it has not been set yet
    if (outputFileName == NULL && !toStdout) {
        outputFileName = inputFileName;
    } 

    /////////////////////// end of options processing///////////////////////////////


    // if we are not to overwrite, check if the output file exists

    if (outputFileName != NULL && 
        !overwrite) {
        // does the file exist? the easiest way is to open it (read only mode)
        FILE *file = fopen(outputFileName, "r");
        if (file != NULL) {
            fclose(file);
            fprintf(stderr, "Will not overwrite. Output file [%s] exists\n",
                    outputFileName);
            exit(1);
                    
        } 
        // at this point, the output file does not exist
    }

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

