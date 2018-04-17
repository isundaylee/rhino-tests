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


#ifndef BINOMIAL_LATTICE_H
#define BINOMIAL_LATTICE_H

#ifdef DOUBLE_PRECISION
  typedef double fptype;
#else
  typedef float fptype;
#endif //SINGLE_PRECISION

// Problem size configuration:
extern int OPT_TIMESTEPS;
extern int MAX_NTIMESTEPS;
extern int NUM_OPTIONS;

/// OptionData keeps the data that describes a single option contract
struct OptionData {
  const char* OptionType;   // Option type.  "P"=PUT, "C"=CALL
  fptype strike;      // strike price
  fptype s;           // spot price
  fptype q;           // dividend
  fptype r;           // risk-free interest rate
  fptype t;           // time to maturity (in year units) (1yr = 1.0, 6mos = 0.5, 3mos = 0.25, ..., etc)
  fptype v;           // volatility
  fptype result;      // result
  fptype tol;         // tolerance (not used in this test)
};

extern OptionData inOptionValues[];

fptype binomial_lattice_scalar(OptionData* optionValues, fptype* priceResult);

fptype binomial_lattice_cilk(OptionData* optionValues, fptype* priceResult);

fptype binomial_lattice_cilk_simd(OptionData* optionValues, fptype* priceResult);

void writeOutput(const char *filename, fptype *PriceResult);

#endif // BINOMIAL_LATTICE_H
