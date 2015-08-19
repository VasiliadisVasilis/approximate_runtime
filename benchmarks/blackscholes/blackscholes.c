// Copyright (c) 2007 Intel Corp.

// Black-Scholes
// Analytical method for calculating European Options
//
// 
// Reference Source: Options, Futures, and Other Derivatives, 3rd Edition, Prentice 
// Hall, John C. Hull,

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include "fast_approx.h"
#include <runtime.h>


//double max_otype, min_otype ;
//double max_sptprice, min_sptprice;
//double max_strike, min_strike;
//double max_rate, min_rate ;
//double max_volatility, min_volatility;
//double max_otime, min_otime ;
//double max_out_price, min_out_price;


#define DIVIDE 120.0


//Precision to use for calculations
#define fptype float

#define NUM_RUNS 1000

typedef struct OptionData_ {
	fptype s;          // spot price
	fptype strike;     // strike price
	fptype r;          // risk-free interest rate
	fptype divq;       // dividend rate
	fptype v;          // volatility
	fptype t;          // time to maturity or option expiration in years 
	//     (1yr = 1.0, 6mos = 0.5, 3mos = 0.25, ..., etc)  
	char OptionType;   // Option type.  "P"=PUT, "C"=CALL
	fptype divs;       // dividend vals (not used in this test)
	fptype DGrefval;   // DerivaGem Reference Value
} OptionData;

OptionData *data;
fptype *prices;
int numOptions;

int    * otype;
fptype * sptprice;
fptype * strike;
fptype * rate;
fptype * volatility;
fptype * otime;
int numError = 0;

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
// Cumulative Normal Distribution Function
// See Hull, Section 11.8, P.243-244
#define inv_sqrt_2xPI 0.39894228040143270286

fptype CNDF ( fptype InputX ) 
{
	int sign;

	fptype OutputX;
	fptype xInput;
	fptype xNPrimeofX;
	fptype expValues;
	fptype xK2;
	fptype xK2_2, xK2_3;
	fptype xK2_4, xK2_5;
	fptype xLocal, xLocal_1;
	fptype xLocal_2, xLocal_3;

	// Check for negative value of InputX
	if (InputX < 0.0) {
		InputX = -InputX;
		sign = 1;
	} else 
		sign = 0;

	xInput = InputX;

	// Compute NPrimeX term common to both four & six decimal accuracy calcs
	expValues = exp(-0.5f * InputX * InputX);
	xNPrimeofX = expValues;
	xNPrimeofX = xNPrimeofX * inv_sqrt_2xPI;

	xK2 = 0.2316419 * xInput;
	xK2 = 1.0 + xK2;
	xK2 = 1.0 / xK2;
	xK2_2 = xK2 * xK2;
	xK2_3 = xK2_2 * xK2;
	xK2_4 = xK2_3 * xK2;
	xK2_5 = xK2_4 * xK2;

	xLocal_1 = xK2 * 0.319381530;
	xLocal_2 = xK2_2 * (-0.356563782);
	xLocal_3 = xK2_3 * 1.781477937;
	xLocal_2 = xLocal_2 + xLocal_3;
	xLocal_3 = xK2_4 * (-1.821255978);
	xLocal_2 = xLocal_2 + xLocal_3;
	xLocal_3 = xK2_5 * 1.330274429;
	xLocal_2 = xLocal_2 + xLocal_3;

	xLocal_1 = xLocal_2 + xLocal_1;
	xLocal   = xLocal_1 * xNPrimeofX;

	//printf("# xLocal: %10.10f\n", xLocal);



	xLocal   = 1.0 - xLocal;

	OutputX  = xLocal;

	//printf("# Output: %10.10f\n", OutputX);

	if (sign) {
		OutputX = 1.0 - OutputX;
	}

	return OutputX;
}

fptype CNDF_approx( fptype InputX ) 
{
	int sign;

	fptype OutputX;
	fptype xInput;
	fptype xNPrimeofX;
	fptype expValues;
	fptype xK2;
	fptype xK2_2, xK2_3;
	fptype xK2_4, xK2_5;
	fptype xLocal, xLocal_1;
	fptype xLocal_2, xLocal_3;

	// Check for negative value of InputX
	if (InputX < 0.0) {
		InputX = -InputX;
		sign = 1;
	} else 
		sign = 0;

	xInput = InputX;

	// Compute NPrimeX term common to both four & six decimal accuracy calcs
	expValues = fasterexp(-0.5f * fasterpow2(InputX));
	xNPrimeofX = expValues;
	xNPrimeofX = xNPrimeofX * inv_sqrt_2xPI;

	xK2 = 0.2316419 * xInput;
	xK2 = 1.0 + xK2;
	xK2 = 1.0 / xK2;
	xK2_2 = fasterpow2(xK2);
	xK2_3 = xK2_2 * xK2;
	xK2_4 = xK2_3 * xK2;
	xK2_5 = xK2_4 * xK2;

	xLocal_1 = xK2 * 0.319381530;
	xLocal_2 = xK2_2 * (-0.356563782);
	xLocal_3 = xK2_3 * 1.781477937;
	xLocal_2 = xLocal_2 + xLocal_3;
	xLocal_3 = xK2_4 * (-1.821255978);
	xLocal_2 = xLocal_2 + xLocal_3;
	xLocal_3 = xK2_5 * 1.330274429;
	xLocal_2 = xLocal_2 + xLocal_3;

	xLocal_1 = xLocal_2 + xLocal_1;
	xLocal   = xLocal_1 * xNPrimeofX;

	//printf("# xLocal: %10.10f\n", xLocal);



	xLocal   = 1.0 - xLocal;

	OutputX  = xLocal;

	//printf("# Output: %10.10f\n", OutputX);

	if (sign) {
		OutputX = 1.0 - OutputX;
	}

	return OutputX;
} 

fptype BlkSchlsEqEuroNoDiv_approx( fptype sptprice,
		fptype strike, fptype rate, fptype volatility,
		fptype time, int otype, float timet, fptype* N1, fptype* N2)
{
	fptype OptionPrice;
	fptype xRiskFreeRate;
	fptype xVolatility;
	fptype xTime;
	fptype xSqrtTime;

	fptype logValues;
	fptype xLogTerm;
	fptype xD1; 
	fptype xD2;
	fptype xPowerTerm;
	fptype xDen;
	fptype d1;
	fptype d2;
	fptype FutureValueX;
	fptype NofXd1;
	fptype NofXd2;
	fptype NegNofXd1;
	fptype NegNofXd2;  


	xRiskFreeRate = rate;
	xVolatility = volatility;
	xTime = time;
	xSqrtTime = sqrt(xTime);
	logValues = log( sptprice / strike );
	xLogTerm = logValues;
	xDen = xVolatility * xSqrtTime;
	xPowerTerm = (xVolatility)*xVolatility;
	xPowerTerm = xPowerTerm * 0.5;


	xD1 = xRiskFreeRate + xPowerTerm;
	xD1 = xD1 * xTime;
	xD1 = xD1 + xLogTerm;
	xD1 = xD1 / xDen;
	d1 = xD1;
	NofXd1 = CNDF_approx( d1 );
	*N1 = NofXd1 ;



	xD2 = xD1 - xDen;
	d2 = xD2;
	NofXd2 = CNDF_approx( d2 );
	*N2 = NofXd2 ;


	FutureValueX = strike * ( fasterexp( -(rate*rate) ) );                                                                                   
	if (otype == 0) {                                                                                                                    
		OptionPrice = (sptprice * NofXd1) - (FutureValueX * NofXd2);
	} else { 
		NegNofXd1 = (1.0 - NofXd1);
		NegNofXd2 = (1.0 - NofXd2);
		OptionPrice = (FutureValueX * NegNofXd2) - (sptprice * NegNofXd1);
	}   


	return OptionPrice;
}

//////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////
fptype BlkSchlsEqEuroNoDiv( fptype sptprice,
		fptype strike, fptype rate, fptype volatility,
		fptype time, int otype, float timet, fptype* N1, fptype* N2)
{
	fptype OptionPrice;
	fptype xRiskFreeRate;
	fptype xVolatility;
	fptype xTime;
	fptype xSqrtTime;

	fptype logValues;
	fptype xLogTerm;
	fptype xD1; 
	fptype xD2;
	fptype xPowerTerm;
	fptype xDen;
	fptype d1;
	fptype d2;
	fptype FutureValueX;
	fptype NofXd1;
	fptype NofXd2;
	fptype NegNofXd1;
	fptype NegNofXd2;  


	xRiskFreeRate = rate;
	xVolatility = volatility;
	xTime = time;
	xSqrtTime = sqrt(xTime);
	logValues = log( sptprice / strike );
	xLogTerm = logValues;
	xDen = xVolatility * xSqrtTime;
	xPowerTerm = xVolatility * xVolatility;
	xPowerTerm = xPowerTerm * 0.5;


	xD1 = xRiskFreeRate + xPowerTerm;
	xD1 = xD1 * xTime;
	xD1 = xD1 + xLogTerm;
	xD1 = xD1 / xDen;
	d1 = xD1;
	NofXd1 = CNDF( d1 );
	*N1 = NofXd1 ;



	xD2 = xD1 - xDen;
	d2 = xD2;
	NofXd2 = CNDF( d2 );
	*N2 = NofXd2 ;


	FutureValueX = strike * ( exp( -(rate)*(time) ) );                                                                                   
	if (otype == 0) {                                                                                                                    
		OptionPrice = (sptprice * NofXd1) - (FutureValueX * NofXd2);
	} else { 
		NegNofXd1 = (1.0 - NofXd1);
		NegNofXd2 = (1.0 - NofXd2);
		OptionPrice = (FutureValueX * NegNofXd2) - (sptprice * NegNofXd1);
	}   


	return OptionPrice;
}


double normalize(double in, double min, double max, double min_new, double max_new)
{
	return (((in - min) / (max - min)) * (max_new - min_new)) + min_new ;
}

void task_black_approx(void *tid_ptr) {
	int i;
	int tid = *(int *)tid_ptr;
	int start = tid * (numOptions);
	int end = start + (numOptions);
	fptype price_orig;

	for (i=start; i<end; i++) {
		fptype price;
		fptype N1, N2;

		price_orig = BlkSchlsEqEuroNoDiv_approx( sptprice[i], strike[i],
				rate[i], volatility[i], otime[i],
				otype[i], 0, &N1, &N2);
		prices[i] = price_orig;
	}
}

void task_black(void *tid_ptr) {
	int i;
	int tid = *(int *)tid_ptr;
	int start = tid * (numOptions);
	int end = start + (numOptions);
	fptype price_orig;

	for (i=start; i<end; i++) {
		fptype price;
		fptype N1, N2;

		price_orig = BlkSchlsEqEuroNoDiv( sptprice[i], strike[i],
				rate[i], volatility[i], otime[i],
				otype[i], 0, &N1, &N2);
		prices[i] = price_orig;
	}
}

int main (int argc, char **argv)
{
	FILE *file;
	int i;
	int loopnum;
	fptype * buffer;
	int * buffer2;
	int rv, threads;
	double ratio;

	threads = atoi(argv[1]);
	ratio = atof(argv[2]);

	char *inputFile = "input";
	char *outputFile = "output";
	char *correct = "golden.data";

	//Read input data from file
	file = fopen(inputFile, "r");
	if(file == NULL) {
		printf("ERROR: Unable to open file `%s'.\n", inputFile);
		exit(1);
	}
	rv = fscanf(file, "%i", &numOptions);
	if(rv != 1) {
		printf("ERROR: Unable to read from file `%s'.\n", inputFile);
		fclose(file);
		exit(1);
	}

	numOptions/=(threads*10);
	numOptions*=(threads*10);

	// alloc spaces for the option data
	data = (OptionData*)malloc(numOptions*sizeof(OptionData));
	prices = (fptype*)malloc(numOptions*sizeof(fptype));
	for ( loopnum = 0; loopnum < numOptions; ++ loopnum )
	{
		rv = fscanf(file, "%f %f %f %f %f %f %c %f %f", &data[loopnum].s, &data[loopnum].strike, &data[loopnum].r, &data[loopnum].divq, &data[loopnum].v, &data[loopnum].t, &data[loopnum].OptionType, &data[loopnum].divs, &data[loopnum].DGrefval);
		if(rv != 9) {
			printf("ERROR: Unable to read from file `%s'.\n", inputFile);
			fclose(file);
			exit(1);
		}
	}
	rv = fclose(file);
	if(rv != 0) {
		printf("ERROR: Unable to close file `%s'.\n", inputFile);
		exit(1);
	}

#define PAD 256
#define LINESIZE 64

	buffer = (fptype *) malloc(5 * numOptions * sizeof(fptype) + PAD);
	sptprice = (fptype *) (((unsigned long long)buffer + PAD) & ~(LINESIZE - 1));
	strike = sptprice + numOptions;
	rate = strike + numOptions;
	volatility = rate + numOptions;
	otime = volatility + numOptions;

	buffer2 = (int *) malloc(numOptions * sizeof(fptype) + PAD);
	otype = (int *) (((unsigned long long)buffer2 + PAD) & ~(LINESIZE - 1));

	for (i=0; i<numOptions; i++) {
		otype[i]      = (data[i].OptionType == 'P') ? 1 : 0;
		sptprice[i]   = data[i].s / DIVIDE;
		strike[i]     = data[i].strike / DIVIDE;
		rate[i]       = data[i].r;
		volatility[i] = data[i].v;    
		otime[i]      = data[i].t;
	}
	int total_numOptions = numOptions;
	char group[1024];
	int tid=0;
	task_t *task;

	numOptions/=(threads*10);

	init_system(threads);
	threads*=10;
	for (i=0; i<NUM_RUNS; ++i )
	{
		sprintf(group, "run%d", i);
		for (tid=0; tid<threads; ++tid)
		{
			task = new_task(task_black, &tid, sizeof(tid), task_black_approx, NON_SIGNIFICANT+1);
			push_task(task, group);
		}
		wait_group(group, NULL, NULL, SYNC_RATIO, 0, 0, ratio, 0);
	}
	shutdown_system();

	numOptions = total_numOptions;


	//Write prices to output file
	#if 0
	file = fopen(outputFile, "w");
	if(file == NULL) {
		printf("ERROR: Unable to open file `%s'.\n", outputFile);
		exit(1);
	}
	//rv = fprintf(file, "%i\n", numOptions);
	if(rv < 0) {
		printf("ERROR: Unable to write to file `%s'.\n", outputFile);
		fclose(file);
		exit(1);
	}
	for(i=0; i<numOptions; i++) {
		rv = fprintf(file, "%.18f\n", prices[i]);
		if(rv < 0) {
			printf("ERROR: Unable to write to file `%s'.\n", outputFile);
			fclose(file);
			exit(1);
		}
	}
	rv = fclose(file);
	if(rv != 0) {
		printf("ERROR: Unable to close file `%s'.\n", outputFile);
		exit(1);
	}
	#endif

	free(data);
	
	file = fopen(correct, "rt");
	
	fptype t_val;
	double err = 0.0;
	double t;
	double total=0.0;

	if ( file != NULL )
	{
		for ( i=0; i<numOptions; ++i )
		{
			if ( fscanf(file, "%f", &t_val) != 1 )
			{
				printf("PSOFOS\n");
				return -1;
			}
			t = t_val - prices[i];
			total += fabs(t_val);
			t = fabs(t)/(double)numOptions;
			err += t;
			total+=fabs(t_val);
		}
		err /= total;
	}
	else
	{
		printf("No golden file!\n");
	}
	free(prices);
		
	printf("ERROR=%.32lg\n", err);

	return 0;
}

