#include "filter.h"
#include "math.h"
#include "float.h"


lmfunc	fcn; 
static int			AllocateLMStruct( struct LMStruct *LM );
static void			FreeLMStruct	( struct LMStruct *LM );
void 			bracket( struct LMStruct	*LM );
double 			sumSquared( double *a, int n );


// Call Levenberg-Marquard optimizer
#if 1
void  RunLMOptimizer( OptInfo	*o)
{
	struct 	LMStruct	LM;
	int 	iflag;
	char	*warning;
	char 	*infmsg[] = {
				"improper input parameters",
				"the relative error in the sum of squares is at most tol",
				"the relative error between x and the solution is at most tol",
				"conditions for info = 1 and info = 2 both hold",
				"fvec is orthogonal to the columns of the jacobian to machine precision",
				"number of calls to fcn has reached or exceeded 200*(n+1)",
				"tol is too small. no further reduction in the sum of squares is possible",
				"tol too small. no further improvement in approximate solution x possible",
				"Interrupted"
				};
	
	

	LM.n = o->numVars;
	
	if( o->numData < LM.n )
	{
		LM.m 		= LM.n;
		warning		= "Warning: Number of Data Points is smaller than Number of Variables to fit.\n";
	}
	else
	{
		LM.m 		= o->numData;
		warning		= "";
	}

	fcn = o->fcn;
		
	if( AllocateLMStruct( &LM ) != 0 )
	{
		PrintError( "Not enough Memory" );
		return;
	}
	
	// Initialize optimization params

	if( o->SetVarsToX( LM.x ) != 0)
	{
		PrintError("Internal Error");
		return;
	}

	iflag 		= -100; // reset counter and initialize dialog
	fcn(LM.m, LM.n, LM.x, LM.fvec, &iflag);
		
	// infoDlg ( _initProgress, "Optimizing Variables" );

	/* Call lmdif. */
	LM.ldfjac 	= LM.m;
	LM.mode 	= 1;
	LM.nprint 	= 10;
	LM.info 	= 0;
	LM.factor 	= 100.0;

	lmdif(	LM.m,		LM.n,		LM.x,		LM.fvec,	LM.ftol,	LM.xtol,
			LM.gtol,	LM.maxfev,	LM.epsfcn,	LM.diag,	LM.mode,	LM.factor,
			LM.nprint,	&LM.info,	&LM.nfev,	LM.fjac,	LM.ldfjac,	LM.ipvt,
			LM.qtf,		LM.wa1,		LM.wa2,		LM.wa3,		LM.wa4);


	o->SetXToVars( LM.x );

	iflag 		= -99; // reset counter and dispose dialog
	fcn(LM.m, LM.n, LM.x, LM.fvec, &iflag);
	// infoDlg ( _disposeProgress, "" );
	
	// Display solver info
		
	if(LM.info >= 8)
			LM.info = 4;
	if(LM.info < 0)
			LM.info = 8;
	
	sprintf( (char*) o->message, "# %s%d function evalutations\n# %s\n",
								warning, LM.nfev, infmsg[LM.info] );

	FreeLMStruct( &LM );
	
}

#endif

#if 0
void  RunLMOptimizer( OptInfo	*o){
	RunBROptimizer ( o, 1.0e-9);
}
#endif
// Call Bracketing optimizer


void  RunBROptimizer ( OptInfo	*o, double minStepWidth)
{
	struct 	LMStruct	LM;
	int 	iflag;
	char 	*infmsg[] = {
				"improper input parameters",
				"the relative error in the sum of squares is at most tol",
				"the relative error between x and the solution is at most tol",
				"conditions for info = 1 and info = 2 both hold",
				"fvec is orthogonal to the columns of the jacobian to machine precision",
				"number of calls to fcn has reached or exceeded 200*(n+1)",
				"tol is too small. no further reduction in the sum of squares is possible",
				"tol too small. no further improvement in approximate solution x possible",
				"Interrupted"
				};

	LM.n = o->numVars;
	
	if( o->numData < LM.n )
	{
		LM.m 		= LM.n;
	}
	else
	{
		LM.m 		= o->numData;
	}

	fcn = o->fcn;
		
	if( AllocateLMStruct( &LM ) != 0 )
	{
		PrintError( "Not enough Memory to allocate Data for BR-solver" );
		return;
	}
				
				
	// Initialize optimization params

	if( o->SetVarsToX( LM.x ) != 0)
	{
		PrintError("Internal Error");
		return;
	}

	iflag 		= -100; // reset counter
	fcn(LM.m, LM.n, LM.x, LM.fvec, &iflag);
		
	//infoDlg ( _initProgress, "Optimizing Params" );

	/* Call lmdif. */
	LM.ldfjac 	= LM.m;
	LM.mode 	= 1;
	LM.nprint 	= 1;
	// Set stepwidth to angle corresponding to one pixel in final pano
	LM.epsfcn	= minStepWidth; // g->pano.hfov / (double)g->pano.width; 
	
	LM.info 	= 0;
	LM.factor 	= 1.0;

#if 0		
	lmdif(	LM.m,		LM.n,		LM.x,		LM.fvec,	LM.ftol,	LM.xtol,
			LM.gtol,	LM.maxfev,	LM.epsfcn,	LM.diag,	LM.mode,	LM.factor,
			LM.nprint,	&LM.info,	&LM.nfev,	LM.fjac,	LM.ldfjac,	LM.ipvt,
			LM.qtf,		LM.wa1,		LM.wa2,		LM.wa3,		LM.wa4);

#endif

	bracket( &LM );

	o->SetXToVars( LM.x );
	iflag 		= -99; // 
	fcn(LM.m, LM.n, LM.x, LM.fvec, &iflag);
	//infoDlg ( _disposeProgress, "" );
	

	FreeLMStruct( &LM );
	
}



// Allocate Memory and set default values. n must be set!

int	AllocateLMStruct( struct LMStruct *LM )
{
	int i,k, result = 0 ;
	

	if( LM->n <= 0 || LM->m <= 0 || LM->n > LM->m )
		return -1;
		
	LM->ftol 	= 	DBL_EPSILON;//1.0e-14;
	LM->xtol 	= 	DBL_EPSILON;//1.0e-14;
	LM->gtol 	= 	DBL_EPSILON;//1.0e-14;
	LM->epsfcn 	=  	DBL_EPSILON * 10.0;//1.0e-15;
	LM->maxfev 	=  	100 * (LM->n+1) * 100; 
	
	LM->ipvt = NULL;
	LM->x = LM->fvec = LM->diag = LM->qtf = LM->wa1 = LM->wa2 = LM->wa3 = LM->wa4 = LM->fjac = NULL;

	LM->ipvt 	= (int*) 	malloc(  LM->n * sizeof( int ));		
	LM->x 		= (double*) malloc(  LM->n * sizeof( double )); 		
	LM->fvec 	= (double*) malloc(  LM->m * sizeof( double )); 		
	LM->diag 	= (double*) malloc(  LM->n * sizeof( double )); 		
	LM->qtf 	= (double*) malloc(  LM->n * sizeof( double )); 		
	LM->wa1 	= (double*) malloc(  LM->n * sizeof( double )); 		
	LM->wa2 	= (double*) malloc(  LM->n * sizeof( double )); 		
	LM->wa3 	= (double*) malloc(  LM->n * sizeof( double )); 		
	LM->wa4 	= (double*) malloc(  LM->m * sizeof( double )); 		
	LM->fjac 	= (double*) malloc(  LM->m  * LM->n * sizeof( double ));

	if( LM->ipvt == NULL ||  LM->x    == NULL 	|| LM->fvec == NULL || LM->diag == NULL || 
		LM->qtf  == NULL ||  LM->wa1  == NULL 	|| LM->wa2  == NULL || LM->wa3  == NULL || 
		LM->wa4  == NULL ||  LM->fjac == NULL )
	{
		FreeLMStruct( LM );
		return -1;
	}


	// Initialize to zero

	for(i=0; i<LM->n; i++)
	{
		LM->x[i] = LM->diag[i] = LM->qtf[i] = LM->wa1[i] = LM->wa2[i] = LM->wa3[i] =  0.0;
		LM->ipvt[i] = 0;
	}

	for(i=0; i<LM->m; i++)
	{
		LM->fvec[i] = LM->wa4[i] = 0.0;
	}

	k = LM->m * LM->n;
	for( i=0; i<k; i++)
			LM->fjac[i] = 0.0;
	
	return 0;
}
		



	
void FreeLMStruct( struct LMStruct *LM )
{
	if(LM->x 	!= NULL) 	free( LM->x );
	if(LM->fvec != NULL) 	free( LM->fvec );
	if(LM->qtf 	!= NULL) 	free( LM->qtf );
	if(LM->wa1 	!= NULL) 	free( LM->wa1 );
	if(LM->wa2 	!= NULL) 	free( LM->wa2 );
	if(LM->wa3 	!= NULL) 	free( LM->wa3 );
	if(LM->wa4 	!= NULL) 	free( LM->wa4 );
	if(LM->fjac != NULL) 	free( LM->fjac );
	if(LM->ipvt != NULL) 	free( LM->ipvt );
}


void bracket( struct LMStruct	*LM )
{
	int iflag = 1,i;
	double eps, delta, delta_max;
	int changed, c = 1;
	
	
	fcn(LM->m, LM->n, LM->x, LM->fvec, &iflag);
	if( iflag < 0 ) return;

	// and do a print
	iflag = 0;
	fcn(LM->m, LM->n, LM->x, LM->fvec, &iflag);
	if( iflag < 0 ) return;
	iflag = 1;
	
	eps = sumSquared( LM->fvec, LM->m );
	
	// Choose delta_max to be between 1 and 2 degrees
	
	if( LM->epsfcn <= 0.0 ) return; // This is an error
	
	for( delta_max = LM->epsfcn; delta_max < 1.0; delta_max *= 2.0){}
	
	for( delta = delta_max; 
		 delta >= LM->epsfcn; 
		 delta /= 2.0  )
	{
		c = 1;
		
		// PrintError("delta = %lf", delta);
		while( c )
		{
			c = 0;
		
			for( i = 0; i < LM->n; i++ )
			{
				changed = 0;
				LM->x[i] += delta;
				fcn(LM->m, LM->n, LM->x, LM->fvec, &iflag); if( iflag < 0 ) return;
				
				if( delta == delta_max ) // search everywhere
				{
					while(  sumSquared( LM->fvec, LM->m ) < eps )
					{
						changed = 1;
						eps = sumSquared( LM->fvec, LM->m );
						LM->x[i] += delta;
						fcn(LM->m, LM->n, LM->x, LM->fvec, &iflag);if( iflag < 0 ) return;
					}
					LM->x[i] -= delta;
				}
				else // do just this one step
				{
					if( sumSquared( LM->fvec, LM->m ) < eps )
					{
						eps = sumSquared( LM->fvec, LM->m );
						changed = 1;
					}
					else
						LM->x[i] -= delta;
				}
		
				if( !changed )	// Try other direction
				{
					LM->x[i] -= delta;
					fcn(LM->m, LM->n, LM->x, LM->fvec, &iflag);if( iflag < 0 ) return;
					
					if( delta == delta_max ) // search everywhere
					{
						while(  sumSquared( LM->fvec, LM->m ) < eps )
						{
							changed = 1;
							eps = sumSquared( LM->fvec, LM->m );
							LM->x[i] -= delta;
							fcn(LM->m, LM->n, LM->x, LM->fvec, &iflag);if( iflag < 0 ) return;
						}
						LM->x[i] += delta;
					}
					else // do just this one step
					{
						if( sumSquared( LM->fvec, LM->m ) < eps )
						{
							eps = sumSquared( LM->fvec, LM->m );
							changed = 1;
						}
						else
							LM->x[i] += delta;
					}
				}
				
				if( changed ) c = 1;
		
				if( 0 ) //changed )
				{
					// This is an improvement now: print
					c=1;
					iflag = 0;
					LM->fvec[0] = sqrt(eps/LM->m);
					fcn(LM->m, LM->n, LM->x, LM->fvec, &iflag);
					if( iflag < 0 ) return;
					iflag = 1;
				}
				
			}
		}
		// PrintError("%lf %ld %lf", delta, c, eps);
					iflag = 0;
					LM->fvec[0] = sqrt(eps/LM->m);
					fcn(LM->m, LM->n, LM->x, LM->fvec, &iflag);
					if( iflag < 0 ) return;
					iflag = 1;

	}
	
}
	
	
double sumSquared( double *a, int n )
{
	double result = 0.0;
	int i;
	
	for( i=0; i<n; i++ )
		result += a[i] * a[i];
		
	return result;
}
		




