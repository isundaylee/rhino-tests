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

#include <cstdio>
#include <cstdlib>
#include <cstring>
#define NOMINMAX
#include <cmath>
#include <algorithm>
#include <atomic>
#include "binomial_lattice.h"
#include <mutex>
#include <cilk/cilk.h>
#include <cilk/reducer_opadd.h>

// The binomial value is found at each node, expected value is calculated using the option values from
// the later two nodes (Option up and Option down) weighted by their respective probabilities.
//
// Depending on the style of the option, evaluate the possibility of early exercise at each node .
// For an American option, since the option may either be held or exercised prior to expiry,
// the value at each node is: Max (Binomial Value, Exercise Value).

// Description:
// This function computes the price results for each node.
//
// Calculation is done in scalar.
//
// [in]: optionValues, idx, blend_vf
// [out]: priceResult
fptype binomial_lattice_scalar(OptionData* optionValues, fptype* priceResult) {
	const fptype ONE = fptype(1.);
	const fptype ZERO = fptype(0.);
	fptype total_priceResult = 0;

	for (int k = 0; k < NUM_OPTIONS; k++) {
		fptype* opt_price = new fptype[MAX_NTIMESTEPS];
		fptype* upow_tbl = new fptype[MAX_NTIMESTEPS];
		fptype* dpow_tbl = new fptype[MAX_NTIMESTEPS];
		fptype* tmp_val  = new fptype[MAX_NTIMESTEPS];

		fptype risk_free_rate = optionValues[k].r;
		fptype stock_price    = optionValues[k].s;
		fptype exe_price      = optionValues[k].strike;
		fptype volatility     = optionValues[k].v;
		fptype deltat         = optionValues[k].t;

		deltat /= OPT_TIMESTEPS;

		fptype u = exp(volatility * sqrt(deltat));
		fptype d = exp(-volatility * sqrt(deltat));
		fptype a = exp(risk_free_rate * deltat);
		fptype multiplier = exp(-risk_free_rate * deltat);

		fptype p = (a - d) / (u - d);

		upow_tbl[0] = stock_price;
		dpow_tbl[0] = ONE;

		for (int i = 1; i <= OPT_TIMESTEPS; i++) {
			upow_tbl[i] = u * upow_tbl[i - 1];
			dpow_tbl[i] = d * dpow_tbl[i - 1];
		}

		// Initial values at expiration time
		for (int i = 0; i <= OPT_TIMESTEPS; i++) {
			opt_price[i] = std::max((exe_price - upow_tbl[OPT_TIMESTEPS - i] * dpow_tbl[i]), ZERO);
		}

		// Move to earlier times
		for (int i = OPT_TIMESTEPS - 1; i >= 0; i--) {
			for (int j = 0; j <= i; j++) {
				tmp_val[j] = (p * opt_price[j] + (ONE - p) * opt_price[j+1]) * multiplier;
			}
			// If exercise is permitted at the node, then the model takes the greater of binomial and exercise value at the node.
			for (int j = 0; j <= i; j++) {
				fptype t1 = upow_tbl[i - j] * dpow_tbl[j];
				fptype early_exe_value = std::max(exe_price - t1, ZERO);
				opt_price[j] = std::max(early_exe_value, tmp_val[j]);
			}
		}
		priceResult[k] = opt_price[0];
		total_priceResult += priceResult[k];
		delete[] opt_price;
		delete[] upow_tbl;
		delete[] dpow_tbl;
		delete[] tmp_val;
	}
	return total_priceResult;
}

// Description:
// This function computes the price results for each node.
//
// Calculation is done using cilk_for calling a scalar base function.
//
// [in]: optionValues, idx, blend_vf
// [out]: priceResult
fptype binomial_lattice_cilk(OptionData* optionValues, fptype* priceResult) {
	const fptype ZERO = fptype(0.);
	const fptype ONE = fptype(1.);

	cilk::reducer<cilk::op_add<fptype> > total_priceResult;

	cilk_for (int k = 0; k < NUM_OPTIONS; ++k) {
		fptype* opt_price = new fptype[MAX_NTIMESTEPS];
		fptype* upow_tbl  = new fptype[MAX_NTIMESTEPS];
		fptype* dpow_tbl  = new fptype[MAX_NTIMESTEPS];
		fptype* tmp_val   = new fptype[MAX_NTIMESTEPS];

		fptype risk_free_rate = optionValues[k].r;
		fptype stock_price    = optionValues[k].s;
		fptype exe_price      = optionValues[k].strike;
		fptype volatility     = optionValues[k].v;
		fptype deltat         = optionValues[k].t;

		deltat /= OPT_TIMESTEPS;

		fptype u = exp(volatility * sqrt(deltat));
		fptype d = exp(-volatility * sqrt(deltat));
		fptype a = exp(risk_free_rate * deltat);
		fptype multiplier = exp(-risk_free_rate * deltat);

		fptype p = (a - d) / (u - d);

		upow_tbl[0] = stock_price;
		dpow_tbl[0] = ONE;

		for (int i = 1; i <= OPT_TIMESTEPS; i++) {
			upow_tbl[i] = u * upow_tbl[i - 1];
			dpow_tbl[i] = d * dpow_tbl[i - 1];
		}

		// Initial values at expiration time
		for (int i = 0; i <= OPT_TIMESTEPS; i++) {
			opt_price[i] = std::max((exe_price - upow_tbl[OPT_TIMESTEPS - i] * dpow_tbl[i]), ZERO);
		}
		// Move to earlier times
		for (int i = OPT_TIMESTEPS - 1; i >= 0; i--) {
			for (int j = 0; j <= i; j++) {
				tmp_val[j] = (p * opt_price[j] + (ONE - p) * opt_price[j+1]) * multiplier;
			}
			// If exercise is permitted at the node, then the model takes the greater of binomial and exercise value at the node.
			for (int j = 0; j <= i; j++) {
				fptype t1 = upow_tbl[i - j] * dpow_tbl[j];
				fptype early_exe_value = std::max(exe_price - t1, ZERO);
				opt_price[j] = std::max(early_exe_value, tmp_val[j]);
			}
		}
		priceResult[k] = opt_price[0];
		*total_priceResult += priceResult[k];
		delete[] opt_price;
		delete[] upow_tbl;
		delete[] dpow_tbl;
		delete[] tmp_val;
	}
	return total_priceResult.get_value();
}

// Description:
// This function computes the price results for each node.
//
// Calculation is done using cilk_for with inner loops being vectorized with #pragma simd
//
// [in]: optionValues, idx, blend_vf
// [out]: priceResult
fptype binomial_lattice_cilk_simd(OptionData* optionValues, fptype* priceResult) {
	const fptype ZERO = fptype(0.);
	const fptype ONE = fptype(1.);
	double total_priceResult(0);
	std::mutex mtx;

	cilk_for (int k = 0; k < NUM_OPTIONS; ++k) {
		fptype* opt_price = new fptype[MAX_NTIMESTEPS];
		fptype* upow_tbl = new fptype[MAX_NTIMESTEPS];
		fptype* dpow_tbl = new fptype[MAX_NTIMESTEPS];
		fptype* tmp_val  = new fptype[MAX_NTIMESTEPS];

		fptype risk_free_rate = optionValues[k].r;
		fptype stock_price    = optionValues[k].s;
		fptype exe_price      = optionValues[k].strike;
		fptype volatility     = optionValues[k].v;
		fptype deltat         = optionValues[k].t;

		deltat /= OPT_TIMESTEPS;

		fptype u = exp(volatility * sqrt(deltat));
		fptype d = exp(-volatility * sqrt(deltat));
		fptype a = exp(risk_free_rate * deltat);
		fptype multiplier = exp(-risk_free_rate * deltat);

		fptype p = (a - d) / (u - d);

		upow_tbl[0] = stock_price;
		dpow_tbl[0] = ONE;

		for (int i = 1; i <= OPT_TIMESTEPS; ++i) {
			upow_tbl[i] = u * upow_tbl[i - 1];
			dpow_tbl[i] = d * dpow_tbl[i - 1];
		}

		// Initial values at expiration time
#pragma simd
		for (int i = 0; i <= OPT_TIMESTEPS; ++i) {
			opt_price[i] = std::max((exe_price - upow_tbl[OPT_TIMESTEPS - i] * dpow_tbl[i]), ZERO);
		}
		// Move to earlier times
		for (int i = OPT_TIMESTEPS - 1; i >= 0; --i) {
			// Vectorization is possible even with forward reference of the array opt_price (j and j+1)
#pragma simd
			for (int j = 0; j <= i; ++j) {
				tmp_val[j] = (p * opt_price[j] + (ONE - p) * opt_price[j+1]) * multiplier;
			}
			// If exercise is permitted at the node, then the model takes the greater of binomial and exercise value at the node.
#pragma simd
			for (int j = 0; j <= i; ++j) {
				fptype t1 = upow_tbl[i - j] * dpow_tbl[j];
				fptype early_exe_value = std::max(exe_price - t1, ZERO);
				opt_price[j] = std::max(early_exe_value, tmp_val[j]);
			}
		}
		priceResult[k] = opt_price[0];
		mtx.lock();
		total_priceResult+=priceResult[k];
		mtx.unlock();
		delete[] opt_price;
		delete[] upow_tbl;
		delete[] dpow_tbl;
		delete[] tmp_val;
	}
	return total_priceResult;
}
