//==============================================================
//
// SAMPLE SOURCE CODE - SUBJECT TO THE TERMS OF SAMPLE CODE LICENSE AGREEMENT,
// http://software.intel.com/en-us/articles/intel-sample-source-code-license-agreement/
//
// Copyright 2010-2013 Intel Corporation
//
// THIS FILE IS PROVIDED "AS IS" WITH NO WARRANTIES, EXPRESS OR IMPLIED, INCLUDING BUT
// NOT LIMITED TO ANY IMPLIED WARRANTY OF MERCHANTABILITY, FITNESS FOR A PARTICULAR
// PURPOSE, NON-INFRINGEMENT OF INTELLECTUAL PROPERTY RIGHTS.
//
// ===============================================================

//Based on an original code by Paul Glasserman and Xiaoliang Zhao (Columbia University, 1999-2000) 
//with subsequent modifications by Mike Giles (Oxford University, 2005-8)"

#ifndef __MONTE_CARLO_INCLUDED_H
#define __MONTE_CARLO_INCLUDED_H

#include <xmmintrin.h>
#include <cstdlib> 
#include <cstdio> 
#include <cmath>
#include <cassert>

#ifdef __INTEL_COMPILER
#include <cilk/cilk.h>
#include <cilk/reducer_opadd.h>
#endif

#ifdef IS_USING_MKL
#include <mkl_vsl.h>
#endif

#include "timer.h"

//GCC uses __attribute__(noinline) at end of function declaration
//This #defines Microsoft's/Intel's __declspec(noinline) to nothing
#ifdef __GNUC__
#define __declspec(noinline) 
#endif

//Determines whether float is single precision (float) or double precision (double)
//This should be defined in the preprocessor as either DOUBLE or SINGLE
//If not set in the preprocessor, SINGLE is chosen, but setting it is preferred
//Set c_simd_vector_length according to how many doubles/floats will fit in your processor's vector register
//#define DOUBLE
#ifdef DOUBLE
typedef double float_point_precision;

const int c_simd_vector_length = 4;
const double c_zero = 0.0;
const double c_one = 1.0;
const float c_one_half = 0.5;
const double c_hundred = 100.0;

#else //either SINGLE is defined, or SINGLE and DOUBLE not specified.
#define SINGLE
typedef float float_point_precision;

const int c_simd_vector_length = 8;
const float c_one = 1.0f;
const float c_zero = 0.0f;
const float c_one_half = 0.5f;
const float c_hundred = 100.0f;
#define exp expf

#endif // SINGLE or DOUBLE floating point precision

//constants

//Number of simulations that Monte Carlo runs
//Must be a multiple of c_simd_vector_length
const int c_num_simulations = 96000;
//The interval at which the future LIBOR rate is recalculated
//Otherwise known as the LIBOR interval
const float_point_precision c_reset_interval = 0.25;

//Number of possible swaptions in the portfolio
//There are 3 possible swaptions with 5 possible maturities (lengths) per swaption
const int c_num_options = 15;
//Possible lengths of swaptions. There are 5 total
const int c_lengths[] = {4,4,4,8,8,8,20,20,20,28,28,28,40,40,40};
//possible strike prices for the 3 swaptions
const float_point_precision c_strike_prices[]  = {.045,.05,.055,.045,.05,.055,.045,.05,
                                                  .055,.045,.05,.055,.045,.05,.055 };

//number of different possible forward rates calculated for the portfolio
const int c_num_forward_rates=80;
//Number of time steps each possible forward rate goes through
//Each step uses the seed of a random number from a normal distrubition
const int c_time_steps = 40;

//Amount price varies over time
//Typically determined as function of time to maturity
//In this example, however, it remains constant
const float_point_precision c_volatility_val = 0.2;
//current known value of the LIBOR rate
const float_point_precision c_initial_LIBOR_rate = 0.051;

//Used for creating a Gaussian distribution using MKL
const float_point_precision c_normal_dist_mean = 0.0;
const float_point_precision c_normal_dist_std_dev = 1.0;

//function prototypes

//kernels.cpp

//Predicts the valuation of a portfolio of swaptions
//initial LIBOR rates and volatility are constant
//Function is seeded by random numbers from a normal distribution
//Answer is stored in the discounted_swaption_payoffs array
//Uses strictly scalar methods to calculate valuation
__declspec(noinline)
void calculate_path_for_swaption_kernel_scalar(
    float_point_precision *initial_LIBOR_rate,
    float_point_precision *volatility,
	float_point_precision *normal_distribution_rand,
    float_point_precision *discounted_swaption_payoffs
)
#ifdef __GNUC__
__attribute__ ((noinline))
#endif
;
#ifdef __INTEL_COMPILER
//Predicts the valuation of a portfolio of swaptions
//initial LIBOR rates and volatility are constant
//Function is seeded by random numbers from a normal distribution
//Answers are stored in the discounted_swaption_payoffs array
//Uses array notation to calculate multiple valuations simultaneously
__declspec(noinline)
void calculate_path_for_swaption_kernel_array(
    float_point_precision *initial_LIBOR_rate,
    float_point_precision *volatility,
	float_point_precision *normal_distribution_rand,
    float_point_precision discounted_swaption_payoffs[c_simd_vector_length]
);
#endif

//simulations.cpp
//Runs simulations using strictly linear, scalar methods
//Calls calculate_path_for_swaption_kernel_array using a for loop
//returns the average of all simulations
__declspec(noinline)
float_point_precision calculate_monte_carlo_paths_scalar(
    float_point_precision *__restrict initial_LIBOR_rate,
    float_point_precision *__restrict volatility,
	float_point_precision *__restrict normal_distribution_rand,
    float_point_precision *__restrict discounted_swaption_payoffs
);
#ifdef __INTEL_COMPILER
//Runs simulations using array notation for vectorized work
//Calls calculate_path_for_swaption_kernel_array using a for loop
//returns the average of all simulations
__declspec(noinline)
float_point_precision calculate_monte_carlo_paths_AN(
    float_point_precision *__restrict initial_LIBOR_rate,
    float_point_precision *__restrict volatility,
	float_point_precision *__restrict normal_distribution_rand,
    float_point_precision *__restrict discounted_swaption_payoffs
);
//Runs simulations using cilk_for for parallelized work
//Calls calculate_path_for_swaption_kernel_scalar using a cilk_for loop
//returns the average of all simulations
__declspec(noinline)
float_point_precision calculate_monte_carlo_paths_cilk(
    float_point_precision *__restrict initial_LIBOR_rate,
    float_point_precision *__restrict volatility,
	float_point_precision *__restrict normal_distribution_rand,
    float_point_precision *__restrict discounted_swaption_payoffs
);
//Runs simulations using array notation and cilk_for for parallelized and vectorized work
//Calls calculate_path_for_swaption_kernel_array using a cilk_for loop
//returns the average of all simulations
__declspec(noinline)
float_point_precision calculate_monte_carlo_paths_cilk_AN(
    float_point_precision *__restrict initial_LIBOR_rate,
    float_point_precision *__restrict volatility,
	float_point_precision *__restrict normal_distribution_rand,
    float_point_precision *__restrict discounted_swaption_payoffs
);
#endif //__INTEL_COMPILER

//mc01.cpp

//Creates and returns a random array of numbers from a normal distribution
//If MKL is not being used simply initializes all values to 0.3
//In this case, pass 0 for mean and std_dev
float_point_precision * initialize_normal_dist(float_point_precision mean, float_point_precision std_dev);

#endif //MC01_INCLUDED_H
