// ==============================================================
// 
//  SAMPLE SOURCE CODE - SUBJECT TO THE TERMS OF SAMPLE CODE LICENSE AGREEMENT,
//  http:// software.intel.com/en-us/articles/intel-sample-source-code-license-agreement/
// 
//  Copyright 2010-2013 Intel Corporation
// 
//  THIS FILE IS PROVIDED "AS IS" WITH NO WARRANTIES, EXPRESS OR IMPLIED, INCLUDING BUT
//  NOT LIMITED TO ANY IMPLIED WARRANTY OF MERCHANTABILITY, FITNESS FOR A PARTICULAR
//  PURPOSE, NON-INFRINGEMENT OF INTELLECTUAL PROPERTY RIGHTS.
// 
//  ===============================================================

// Based on an original code by Paul Glasserman and Xiaoliang Zhao (Columbia University, 1999-2000) 
// with subsequent modifications by Mike Giles (Oxford University, 2005-8)"



// This code calculates an estimation of the valuation of a portfolio of European LIBOR-based 
// swaptions using a Monte Carlo simulation.
// One methodology runs in straight scalar code, one using Array Notations to allow the code 
// to vectorize, one using Intel(R) Cilk(TM) Plus to include parallelization, and one with both.
// 
// You can optionally compile with GCC and MSC, but just the linear, scalar version will compile
// and it will not have all optimizations

#include "monte_carlo.h"

int main(int argc, const char **argv)
{
	// sanity check to make sure the number of simulations is a multiple of c_simd_vector_length
    assert((c_num_simulations%c_simd_vector_length)==0);

    float_point_precision *volatility = (float_point_precision *)_mm_malloc(sizeof(float_point_precision)*c_num_forward_rates, 64);
    float_point_precision *initial_LIBOR_rate = (float_point_precision *)_mm_malloc(sizeof(float_point_precision)*c_num_forward_rates, 64);
    float_point_precision *discounted_swaption_payoffs = (float_point_precision *)_mm_malloc(sizeof(float_point_precision)*c_num_simulations, 64);

	// Initialize current LIBOR rate and volatility table
	for(int i=0; i<c_num_forward_rates; i++) {
		initial_LIBOR_rate[i] = c_initial_LIBOR_rate;
		volatility[i] = c_volatility_val;
	}

	// create normal distribution either using MKL, or setting all values to 0.3
#ifdef IS_USING_MKL
	float_point_precision *normal_distribution_rand=initialize_normal_dist(c_normal_dist_mean, c_normal_dist_std_dev);
#else
	float_point_precision *normal_distribution_rand=initialize_normal_dist(0,0);
#endif

#ifndef __INTEL_COMPILER
#ifdef PERF_NUM
	double avg_time = 0;
	for(int i=0; i<5; ++i) {
#endif
	CUtilTimer timer;
	printf("Starting serial, scalar Monte Carlo...\n");
	timer.start();
	float_point_precision payoff = calculate_monte_carlo_paths_scalar(initial_LIBOR_rate, volatility, normal_distribution_rand, discounted_swaption_payoffs);
	timer.stop();
	printf("Calculation finished. Average discounted payoff is %.6f. Time taken is %.0fms\n", payoff, timer.get_time()*1000.0);
#ifdef PERF_NUM
	avg_time += time;
	}
	printf("avg time: %.0fms\n", avg_time*1000.0/5);
#endif
#else
	int option = 3; /*
#ifndef PERF_NUM
	// Checks to see if option was given at command line
	if(argc>1) {
		// Prints out instructions and quits
		if(argv[1][0] == 'h') {
		    printf("This example will use Monte Carlo to determine an average valuation of a swaption based on the LIBOR market. Pick which parallel method you would like to use.\n");
		    printf("[0] all tests\n[1] serial/scalar\n[2] serial/array notation\n[3] cilk_for/scalar\n[4] cilk_for/array notation\n");
#ifdef _WIN32
			system("PAUSE");
#endif
			return 0;
		}
		else {			
		option = atoi(argv[1]);
		}
	}
	// If no options are given, prompt user to choose an option
	else {
		printf("This example will use Monte Carlo to determine an average valuation of a swaption based on the LIBOR market. Pick which parallel method you would like to use.\n");
		printf("[0] all tests\n[1] serial/scalar\n[2] serial/array notation\n[3] cilk_for/scalar\n[4] cilk_for/array notation\n  > ");
		scanf("%i", &option);
	}
#endif // !PERF_NUM
  */

	CUtilTimer timer;
	double serial_time, vec_time, cilk_time, cilk_vec_time;

	// Run this once to initialize Intel(R) Cilk(TM) Plus runtime
	calculate_monte_carlo_paths_cilk_AN(initial_LIBOR_rate, volatility, normal_distribution_rand, discounted_swaption_payoffs);
	float_point_precision payoff;
	switch (option) {
	case 0:
#ifdef PERF_NUM
		double avg_time[4];
		avg_time[:] = 0.0;
		for(int i=0; i<5; ++i) {
#endif
        printf("\nRunning all tests\n");

    	printf("Starting serial, scalar Monte Carlo...\n");
		timer.start();
		payoff = calculate_monte_carlo_paths_scalar(initial_LIBOR_rate, volatility, normal_distribution_rand, discounted_swaption_payoffs);
		timer.stop();
		serial_time = timer.get_time();
		printf("Calculation finished. Average discounted payoff is %.6f. Time taken is %.0fms\n", payoff, serial_time*1000.0);

        printf("\nStarting array notations Monte Carlo...\n");
		timer.start();
		payoff = calculate_monte_carlo_paths_AN(initial_LIBOR_rate, volatility, normal_distribution_rand, discounted_swaption_payoffs);
		timer.stop();
		vec_time = timer.get_time();
		printf("Calculation finished. Average discounted payoff is %.6f. Time taken is %.0fms\n", payoff,	vec_time*1000.0);

        printf("\nStarting cilk_for Monte Carlo...\n");
		timer.start();
		payoff = calculate_monte_carlo_paths_cilk(initial_LIBOR_rate, volatility, normal_distribution_rand, discounted_swaption_payoffs);
		timer.stop();
		cilk_time = timer.get_time();
		printf("Calculation finished. Average discounted payoff is %.6f. Time taken is %.0fms\n", payoff, cilk_time*1000.0);

        printf("\nStarting cilk_for + array notation Monte Carlo...\n");
		timer.start();
		payoff = calculate_monte_carlo_paths_cilk_AN(initial_LIBOR_rate, volatility, normal_distribution_rand, discounted_swaption_payoffs);
		timer.stop();
		cilk_vec_time = timer.get_time();
		printf("Calculation finished. Average discounted payoff is %.6f. Time taken is %.0fms\n", payoff,	cilk_vec_time*1000.0);
#ifdef PERF_NUM
		avg_time[0] += serial_time;
		avg_time[1] += vec_time;
		avg_time[2] += cilk_time;
		avg_time[3] += cilk_vec_time;
		}
		printf("avg time: %.0f\n", avg_time[:]*1000.0/5);
#endif
		break;

	case 1:
		printf("Starting serial, scalar Monte Carlo...\n");
		timer.start();
		payoff = calculate_monte_carlo_paths_scalar(initial_LIBOR_rate, volatility, normal_distribution_rand, discounted_swaption_payoffs);
		timer.stop();
		printf("Calculation finished. Average discounted payoff is %.6f. Time taken is %.0fms\n", payoff, timer.get_time()*1000.0);
		break;

	case 2:
        printf("\nStarting array notations Monte Carlo...\n");
		timer.start();
		payoff = calculate_monte_carlo_paths_AN(initial_LIBOR_rate, volatility, normal_distribution_rand, discounted_swaption_payoffs);
		timer.stop();
		printf("Calculation finished. Average discounted payoff is %.6f. Time taken is %.0fms\n", payoff, timer.get_time()*1000.0);
		break;

	case 3:
    //    printf("\nStarting cilk_for Monte Carlo...\n");
		timer.start();
		payoff = calculate_monte_carlo_paths_cilk(initial_LIBOR_rate, volatility, normal_distribution_rand, discounted_swaption_payoffs);
		timer.stop();
		printf("%.0f\n", timer.get_time()*1000.0);
		break;

	case 4:
		printf("\nStarting cilk_for + array notation Monte Carlo...\n");
		timer.start();
		payoff = calculate_monte_carlo_paths_cilk_AN(initial_LIBOR_rate, volatility, normal_distribution_rand, discounted_swaption_payoffs);
		timer.stop();
		printf("Calculation finished. Average discounted payoff is %.6f. Time taken is %.0fms\n", payoff, timer.get_time()*1000.0);
		break;

	default:
        printf("Please pick a valid option\n");
		break;
	}
#endif

#ifdef _WIN32
    system("PAUSE");
#endif
    return 0; 
}

// Returns an array of random numbers pulled from a normal distribution
// If MKL is enabled, a Gaussian distribution is used
// if MKL is not enabled, 0.3 is used for all "random" values (pass 0 for mean and std_dev)
float_point_precision *initialize_normal_dist(float_point_precision mean, float_point_precision std_dev)
{
#if _MSC_VER && !__INTEL_COMPILER
	float_point_precision *zloc = (float_point_precision *)_aligned_malloc(sizeof(float_point_precision)*c_num_simulations*c_time_steps, 64);
#else
	float_point_precision *zloc = (float_point_precision *)_mm_malloc(sizeof(float_point_precision)*c_num_simulations*c_time_steps, 64);
#endif

#ifdef IS_USING_MKL
	VSLStreamStatePtr vslstream;
	// To vary the random distribution, change the seed (seed is currently 1234)
    vslNewStream( &vslstream, VSL_BRNG_MRG32K3A, 1234);
#ifdef DOUBLE 
    vdRngGaussian( VSL_METHOD_DGAUSSIAN_ICDF, vslstream, c_num_simulations*c_time_steps, zloc,  mean, std_dev);
#else
    vsRngGaussian( VSL_METHOD_DGAUSSIAN_ICDF, vslstream, c_num_simulations*c_time_steps, zloc, mean, std_dev);  
#endif
    vslDeleteStream( &vslstream);
#else
	for(int i=0; i<c_num_simulations*c_time_steps; ++i) {
		zloc[i] = 0.3;
	}
#endif // IS_USING_MKL

    return zloc;
}
