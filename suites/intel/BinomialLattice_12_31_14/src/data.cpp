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


#pragma warning(disable:4305)

#include "binomial_lattice.h"
#include <fstream>

using namespace std;

//TODO: check whether NUM_OPTIONS is larger than the size of the f/dOptionValues

// The data below are from An Approximate Formula for Pricing American Options
//
// Fields: type, strike, spot, q, r, t, vol, value, tolerance

OptionData inOptionValues[] = {
#include "option_values.txt"
};

// Write the calculated output price for this model to the price_result.txt file
void writeOutput(const char *filename, fptype *PriceResult) {
	fstream Out;
	Out.open( filename, ios::out );

	for( int i=0; i<NUM_OPTIONS; i++ ) 
		Out << PriceResult[i] << endl;
	Out.close();
}