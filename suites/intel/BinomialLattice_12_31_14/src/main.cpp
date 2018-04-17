//==============================================================
//
// SAMPLE SOURCE CODE - SUBJECT TO THE TERMS OF SAMPLE CODE LICENSE AGREEMENT,
// http://software.intel.com/en-us/articles/intel-sample-source-code-license-agreement/
//
// Copyright 2013 Intel Corporation
//
// THIS FILE IS PROVIDED "AS IS" WITH NO WARRANTIES, EXPRESS OR IMPLIED, INCLUDING BUT
// NOT LIMITED TO ANY IMPLIED WARRANTY OF MERCHANTABILITY, FITNESS FOR A PARTICULAR
// PURPOSE, NON-INFRINGEMENT OF INTELLECTUAL PROPERTY RIGHTS.
//
// ===============================================================


// Cox-Ingersoll-Ross binomial options pricing model (BOPM)
// The binomial options pricing model (BOPM) traces the evolution of the option's key underlying variables in discrete-time.
// This is done by means of a binomial lattice (tree), for a number of time steps between the valuation and expiration dates. 
// Each node in the lattice represents a possible price of the underlying at a given point in time.

// This code demonstrates the approach computing the price of an American put option :
// One methodology runs in straight scalar code, one using Intel(R) Cilk(tm) Array Notations
// to allow the code to vectorize, one using cilk_for to include parallelization, and one with both.
// 
// You can optionally compile with GCC* and Microsoft* Visual Studio 2010* or 2012* Compiler, 
// but just the linear, scalar version will compile and it will not have all optimizations

#include <xmmintrin.h>
#include <cstdlib> 
#include <cstdio>
#include <string>
#include <cmath>


#ifdef __INTEL_COMPILER
#include <cilk/cilk.h>
#endif

#include "binomial_lattice.h"
#include "timer.h"

using namespace std;

// Problem size configuration:
int OPT_TIMESTEPS = 1500;			// How many time-steps to consider in the tree
int MAX_NTIMESTEPS = OPT_TIMESTEPS + 1;;
int NUM_OPTIONS = 512;

// Read the the option values for each option from the option_values.txt file
void get_input(OptionData* OptionValues) {
    int filesize = 512;
    for(int j = 0; j < filesize; j++)
        for(int i = 0 ;i < NUM_OPTIONS/filesize; i++)
            OptionValues[j+(filesize)*i]=inOptionValues[j];
}

int main(int argc, char *argv[]) {
	OptionData *OptionValues=(OptionData*)_mm_malloc(NUM_OPTIONS*sizeof(OptionData),64);
	get_input(OptionValues);

	// Display current configuration
	//printf("Num of Options: %d\n", NUM_OPTIONS);
  //printf("Num of Time steps: %d\n", OPT_TIMESTEPS);

	fptype *PriceResult=(fptype*)_mm_malloc(NUM_OPTIONS*sizeof(fptype),64);
	string price_result_base = "price_result";
	string price_result;

	int option = 3;


	// Load up the Intel(R) Cilk(TM) Plus runtime to to get accurate performance numbers
	double g = 2.0;
	cilk_for (int i = 0; i < 100; i++) {
		g /= sin(g);
	}
	CUtilTimer timer;
	double serial_time=0, cilk_time=0, cilk_vec_time=0;
	fptype total_price;

		//printf("Starting cilk_for/scalar sample...\n");
		timer.start();
		total_price = binomial_lattice_cilk(OptionValues, PriceResult);
		timer.stop();
		printf("%.f\n",timer.get_time());
		//printf("Writing results to file...\n");
		writeOutput(price_result.assign(price_result_base).append("_cilk.txt").c_str(),PriceResult);

	_mm_free(OptionValues);
	_mm_free(PriceResult);

#ifdef _WIN32
	system("pause");
#endif

	return 0;
}
