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

// There are two kernels: scalar kernel and array notation kernel. These kernels calculate
// a single possible path (in the case of scalar) or multiple paths simultaneously (array notation)
// of the swaption based on random numbers from the normal distribution.


#include "monte_carlo.h"

// Description:
// 1) calculates the forward LIBOR rates over the time interval
// 2) calculates the estimated strike price for the swaption
// 3) discounts the forward LIBOR rate
//
// Everything is done linearly and scalar, which makes this the simpler kernel to understand
// since no optimizations are mixed with the logic of the code
//
// [in]:  initial_LIBOR_rate, volatility, normal_distribution_rand
// [out]: discounted_swaption_payoffs
__declspec(noinline)
void calculate_path_for_swaption_kernel_scalar(
    float_point_precision *initial_LIBOR_rate,
    float_point_precision *volatility,
	float_point_precision *normal_distribution_rand,
    float_point_precision *discounted_swaption_payoffs
)
{
    __declspec(align(64)) float_point_precision forward_LIBOR_rates[c_num_forward_rates];
    // initial_LIBOR_rate holds constant values, but could be filled with real, varied values
	for(int i=0;i<c_num_forward_rates;++i) {
        forward_LIBOR_rates[i] = initial_LIBOR_rate[i];
    }
    
    // This loop iterates through the time steps for each forward rate
    // It is the same as equation 4 on our knowledge base website
    for(int i=0; i<c_time_steps; ++i) {
        float_point_precision spot_LIBOR_drift = c_zero;
		float_point_precision sqiz = sqrt(c_reset_interval)*normal_distribution_rand[i];
        for (int j=i+1; j<c_num_forward_rates; ++j) {
            float_point_precision volatility_local  = volatility[j-i-1];
            spot_LIBOR_drift += c_reset_interval*volatility_local*forward_LIBOR_rates[j]/(c_one+c_reset_interval*forward_LIBOR_rates[j]);
			float_point_precision vrat = exp(volatility_local *(spot_LIBOR_drift - c_one_half*volatility_local)*c_reset_interval + volatility_local*sqiz);
            forward_LIBOR_rates[j] *= vrat;
        }
    }

    // This next section calculates the projected discounted swaption payoffs of the portfolio
	// Both B and S are required to calculate swaption value
	// These values represent the future value of the swaption based on the Libor rate
	// It is the same as equation 5 on our knowledge base website
    float_point_precision b = c_one;
    float_point_precision s = c_zero;
    float_point_precision B[c_time_steps], S[c_time_steps];
    for (int i=c_time_steps; i<c_num_forward_rates; ++i) {
        b = b/(c_one+c_reset_interval*forward_LIBOR_rates[i]);
        s = s+c_reset_interval*b;
        B[i-c_time_steps] = b;
        S[i-c_time_steps] = s;
    }

    // Swapval will only contribute to the portfolio if it is positive
    float_point_precision swaption_payoff = c_zero;
    for (int i=0; i<c_num_options; ++i){
        int k = c_lengths[i]-1;
        float_point_precision swapval = c_one - B[k] - c_strike_prices[i]*S[k];
        if(swapval > c_zero) {
            swaption_payoff += c_hundred*swapval;
		}
    }

    // Finally, divide by the numeraire to get the numeraire based payoff (discounted)
    float_point_precision numeraire = c_one;
    for(int i=0; i<c_time_steps; ++i){
        numeraire *= (c_one+c_reset_interval*forward_LIBOR_rates[i]);
    }
    *discounted_swaption_payoffs = swaption_payoff/numeraire;
}

#ifdef __INTEL_COMPILER
// 1) calculates the forward LIBOR rates over the time interval
// 2) calculates the estimated strike price for the swaption
// 3) discounts the forward LIBOR rate
//
// Work is done the same as scalar function, but over multiple data points simultaneously
//
// [in]: initial_LIBOR_rate, volatility, normal_distribution_rand
// [out]: discounted_swaption_payoffs[c_simd_vector_length]
__declspec(noinline)
void calculate_path_for_swaption_kernel_array(
    float_point_precision *initial_LIBOR_rate,
    float_point_precision *volatility,
	float_point_precision *normal_distribution_rand,
    float_point_precision discounted_swaption_payoffs[c_simd_vector_length] // will not alias with other arguments
)
{
  return calculate_path_for_swaption_kernel_scalar(initial_LIBOR_rate,
					    volatility,
					    normal_distribution_rand,
					    discounted_swaption_payoffs);
  
  // // initial_LIBOR_rate holds constant values, but could be filled with real, varied values
  // 	__declspec(align(64)) float_point_precision forward_LIBOR_rates[c_num_forward_rates][c_simd_vector_length];
  //   for (int i=0; i<c_num_forward_rates; ++i) {
  //       forward_LIBOR_rates[i][:] = initial_LIBOR_rate[i];
  //   }

  //   // This loop iterates through the time steps for each forward rate
  //   // It is the same as equation 4 on our knowledge base website
  // 	for (int j=0; j<c_time_steps; ++j) {
  // 		__declspec(align(64)) float_point_precision spot_LIBOR_drift[c_simd_vector_length], sqiz[c_simd_vector_length];
  // 		spot_LIBOR_drift[:] = c_zero;
  // 		// Due to deterministic floating point addition, the normal distribution must be sorted
  // 		// For more information, see our knowledge base article
  //       sqiz[:] = sqrt(c_reset_interval) * normal_distribution_rand[j:c_simd_vector_length:c_time_steps];
  //       for (int i=j+1; i<c_num_forward_rates; ++i) {
  //           float_point_precision volatility_local = volatility[i-j-1];
  // 			spot_LIBOR_drift[:] += c_reset_interval*volatility_local*forward_LIBOR_rates[i][:]/(c_one+c_reset_interval*forward_LIBOR_rates[i][:]);
  // 			__declspec(align(64)) float_point_precision vrat[c_simd_vector_length];
  // 			vrat[:] = exp(volatility_local*(spot_LIBOR_drift[:] - c_one_half*volatility_local)*c_reset_interval + 
  // 				volatility_local*sqiz[:]);
  // 			forward_LIBOR_rates[i][:] *= vrat[:];
  // 		}
  //   }

  // 	// This next section calculates the projected discounted swaption payoffs of the portfolio
  // 	// Both B and S are required to calculate swaption value
  // 	// These values represent the future value of the swaption based on the Libor rate
  // 	// It is the same as equation 5 on our knowledge base website
  // 	__declspec(align(64)) float_point_precision b[c_simd_vector_length], s[c_simd_vector_length];
  // 	__declspec(align(64)) float_point_precision B[c_time_steps][c_simd_vector_length], S[c_time_steps][c_simd_vector_length];
  //   b[:] = c_one;
  //   s[:] = c_zero;

  //   for (int i=c_time_steps; i<c_num_forward_rates; ++i) {
  //       b[:] = b[:]/(c_one+c_reset_interval*forward_LIBOR_rates[i][:]);
  //       s[:] = s[:] + c_reset_interval*b[:];
  //       B[i-c_time_steps][:] = b[:];
  //       S[i-c_time_steps][:] = s[:];
  //   }

  // 	// Swapval will only contribute to the portfolio if it is positive
  // 	float_point_precision swaption_payoff[c_simd_vector_length];
  // 	swaption_payoff[:] = c_zero;
  //   for (int i=0; i<c_num_options; ++i){
  //       int k = c_lengths[i] - 1;
  // 		__declspec(align(64)) float_point_precision swapval[c_simd_vector_length];
  //       swapval[:] = c_one - B[k][:] - c_strike_prices[i]*S[k][:];
  // 		if(swapval[:] > c_zero) {
  // 			swaption_payoff[:] += c_hundred* swapval[:];
  // 		}
  //   }

  //   // Finally, divide by the numeraire to get the numeraire based payoff (discounted)
  // 	float_point_precision numeraire[c_simd_vector_length];
  // 	numeraire[:] = c_one;
  // 	for(int i=0; i<c_time_steps; ++i) {
  // 		numeraire[:] *= (c_one+c_reset_interval*forward_LIBOR_rates[i][:]);
  // 	}
  // 	discounted_swaption_payoffs[:] = swaption_payoff[:]/numeraire[:];
}
#endif // __INTEL_COMPILER
