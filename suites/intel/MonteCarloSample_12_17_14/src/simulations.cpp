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

// Based on an original code by Paul Glasserman and Xiaoliang Zhao (Columbia University, 1999-2000) 
// with subsequent modifications by Mike Giles (Oxford University, 2005-8)"

// These four functions will call the kernel functions located in kernels.cpp in different ways. 
// Monte Carlo runs many simulations that calculate the swaption paths (kernels). 
// One calls scalar code using a for loop, one calls array notation using a for loop,
// one calls scalar code using cilk_for, and one calls array notation using cilk_for.

#include "monte_carlo.h"

// Description:
// Calls scalar kernel using linear for loop
// [in]: initial_LIBOR_rate, volatility, normal_distribution_rand, discounted_swaption_payoffs
// [out]: average discounted payoff
__declspec(noinline)
float_point_precision calculate_monte_carlo_paths_scalar(
    float_point_precision *__restrict initial_LIBOR_rate,
    float_point_precision *__restrict volatility,
	float_point_precision *__restrict normal_distribution_rand,
    float_point_precision *__restrict discounted_swaption_payoffs
)
{
    for (int path=0; path<c_num_simulations; ++path) {
        calculate_path_for_swaption_kernel_scalar(initial_LIBOR_rate, volatility, 
			normal_distribution_rand+(path*c_time_steps), discounted_swaption_payoffs+path);
    }
	float_point_precision total_payoff = c_zero;
	for(int i=0; i<c_num_simulations; ++i) {
		total_payoff += discounted_swaption_payoffs[i];
	}
    return total_payoff/c_num_simulations;
}

#ifdef __INTEL_COMPILER
// Description:
// Calls array notation kernel using linear for loop
// [in]: initial_LIBOR_rate, volatility, normal_distribution_rand, discounted_swaption_payoffs
// [out]: average discounted payoff
__declspec(noinline)
float_point_precision calculate_monte_carlo_paths_AN(
    float_point_precision *__restrict initial_LIBOR_rate,
    float_point_precision *__restrict volatility,
	float_point_precision *__restrict normal_distribution_rand,
    float_point_precision *__restrict discounted_swaption_payoffs
)
{
	float_point_precision total_payoff = c_zero;
    for (int path=0; path<c_num_simulations; path+=c_simd_vector_length) {
		calculate_path_for_swaption_kernel_array(initial_LIBOR_rate, volatility, 
			normal_distribution_rand+(path*c_time_steps), discounted_swaption_payoffs+path);
    }
	for(int i=0; i<c_num_simulations; ++i) {
		total_payoff += discounted_swaption_payoffs[i];
	}
    return total_payoff/c_num_simulations;
}

// Description:
// Calls scalar kernel using cilk_for
// [in]: initial_LIBOR_rate, volatility, normal_distribution_rand, discounted_swaption_payoffs
// [out]: average discounted payoff
__declspec(noinline)
float_point_precision calculate_monte_carlo_paths_cilk(
    float_point_precision *__restrict initial_LIBOR_rate,
    float_point_precision *__restrict volatility,
	float_point_precision *__restrict normal_distribution_rand,
    float_point_precision *__restrict discounted_swaption_payoffs
)
{
    cilk_for (int path=0; path<c_num_simulations; ++path) {
        calculate_path_for_swaption_kernel_scalar(initial_LIBOR_rate, volatility, 
			normal_distribution_rand+(path*c_time_steps), discounted_swaption_payoffs+path);
    }        
	float_point_precision total_payoff = c_zero;
	for(int i=0; i<c_num_simulations; ++i) {
		total_payoff+=discounted_swaption_payoffs[i];
	}
    return total_payoff/c_num_simulations;
}

// Description:
// Calls array notation kernel using cilk_for
// [in]: initial_LIBOR_rate, volatility, normal_distribution_rand, discounted_swaption_payoffs
// [out]: average discounted payoff
__declspec(noinline)
float_point_precision calculate_monte_carlo_paths_cilk_AN(
    float_point_precision *__restrict initial_LIBOR_rate,
    float_point_precision *__restrict volatility,
	float_point_precision *__restrict normal_distribution_rand,
    float_point_precision *__restrict discounted_swaption_payoffs
)
{
    cilk_for (int path=0; path<c_num_simulations; path+=c_simd_vector_length) {
		calculate_path_for_swaption_kernel_array(initial_LIBOR_rate, volatility, 
			normal_distribution_rand+(path*c_time_steps), discounted_swaption_payoffs+path);
    }
	float_point_precision total_payoff = c_zero;
	for(int i=0; i<c_num_simulations; ++i) {
		total_payoff+=discounted_swaption_payoffs[i];
	}
    return total_payoff/c_num_simulations;
}
#endif // __INTEL_COMPILER