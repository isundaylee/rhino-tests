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

// ===============================================================
// Based on the code example from book "Structured Parallel Programming" 
//		by Michael McCool, Arch Robison, James Reinders
// ===============================================================


#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <cmath>

#ifdef _WIN32
#include <Windows.h>
#endif

#ifdef __INTEL_COMPILER
#include <cilk/cilk.h>
#include <cilk/cilk_api.h>
#endif

#include "timer.h"

#include <algorithm>

using std::min;
using std::max;

//constants
// Neighborhood distance in each dimension
const int c_distance = 4;
// Number of points in direction x
const int c_num_x = 200;
// Number of points in direction y
const int c_num_y = 200;
// Number of points in direction z
const int c_num_z = 100;
// Time
const int c_time = 40;

// Coefficients for differnt distances in order
const float c_coef[c_distance + 1] = {-1435.0f/504 * 3, 1.6f, -0.2f, 8.0f/315, -1.0f / 560.0f};

// Points to array space storing 3D grid values
// The code uses two 3D arrays of coordinates, one for even values of t and the other for odd values.
extern float **g_grid3D;

// Phase velocities
extern float *g_vsq;

// Number of PIECES to partition in each dimension for parallelization using cilk_spawn
const int c_NPIECES = 2;
// Threshold for chunk partition in Time
const int c_dt_threshold = 3;
// Threshold for chunk partition in direction x
const int c_dx_threshold = 1000;
// Threshold for chunk partition in direction y and direction z
const int c_dyz_threshold = 3;

// This function runs test using strictly serial, scalar methods
// Calls loop_stencil to do the calculation
// Calls print_summary to give out the timing report.
// Calls print_y to output result values of points in dimension y.
void dotest_serial();

// This function runs test using pragma simd for vectorized work
// Calls loop_stencil_simd to do the calculation
// Calls print_summary to give out the timing report.
// Calls print_y to output result values of points in dimension y.
void dotest_simd();

// This function runs test using cilk_spawn for parallelized work
// Calls co_cilk to do the calculation
// Calls print_summary to give out the timing report.
// Calls print_y to output result values of points in dimension y.
void dotest_cilk_spawn();

// This function runs test using cilk_spawn and pragma simd for parallelized and vectorized work
// Calls co_cilk_simd to do the calculation
// Calls print_summary to give out the timing report.
// Calls print_y to output result values of points in dimension y.
void dotest_cilk_spawn_simd();

// This function calculates using cilk_spawn and array notation
// Runs to load up the Intel(R) Cilk(TM) Plus runtime to get accurate performance numbers for later calculation
void load_cilk_runtime();

// This function calculates the 25-points 3D stencil
// Calculates in serial and scalar methods
void loop_stencil(int t0, int t1, 
         int x0, int x1,
         int y0, int y1,
         int z0, int z1);

// This function calculates the 25-points 3D stencil
// Calculates using pragma simd method
void loop_stencil_simd(int t0, int t1, 
         int x0, int x1,
         int y0, int y1,
         int z0, int z1);

// This function calculates the 25-points 3D stencil
// Calls a scalar base function using cilk_spawn
void co_cilk(int t0, int t1, 
           int x0, int dx0, int x1, int dx1,
           int y0, int dy0, int y1, int dy1, 
           int z0, int dz0, int z1, int dz1 );

// This function calculates the 25-points 3D stencil
// Calls a simd base function using cilk_spawn
void co_cilksimd(int t0, int t1, 
           int x0, int dx0, int x1, int dx1,
           int y0, int dy0, int y1, int dy1, 
           int z0, int dz0, int z1, int dz1 );
