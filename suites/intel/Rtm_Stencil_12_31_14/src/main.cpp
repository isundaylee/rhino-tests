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

// Stencil computation is the basis for the reverse time migration algorithm in seismic computing.  The underlying mathematical problem is to solve the wave equation using finite difference method. 

// This code computes a 25-point 3D stencil. 
// One methodology runs in straight scalar code, one using pragma simd to allow the code 
// to vectorize, one using Intel(R) Cilk(TM) Plus to include parallelization, and one with both.
// 
// You can optionally compile with GCC and MSC, but just the linear, scalar version will compile
// and it will not have all optimization.


#include "rtm_stencil.h"

int main(int argc, char* argv[]) {
    // Initialization
    g_grid3D = new float*[2];
    g_grid3D[0] = new float[c_num_x * c_num_y * c_num_z];
    g_grid3D[1] = new float[c_num_x * c_num_y * c_num_z];
    g_vsq = new float[c_num_x * c_num_y * c_num_z];

    //printf("Order-%d 3D-Stencil (%d points) with space %dx%dx%d and time %d\n", 
    //       2*c_distance, c_distance*2*3+1, c_num_x, c_num_y, c_num_z, c_time);

#ifndef __INTEL_COMPILER // Using cl       
    printf("Starting serial, scalar sample...\n");
    dotest_serial();
#else // __INTEL_COMPILER is defined, using icl
    int option = 3;
    /*
    // If PERF_NUM is defined, then no options taken...run all tests
#ifndef PERF_NUM
    // Checks to see if option was given at command line
    if(argc>1) {
        // Prints out instructions and quits
        if(argv[1][0] == 'h') {
            printf("[0] all tests\n[1] serial/scalar\n[2] serial/simd\n[3] cilk_spawn/scalar\n[4] cilk_spawn/simd\n");
#ifdef _WIN32
            system("PAUSE");
#endif // _WIN32
            return 0;
        }
        // option is assumed an option
        else {            
            option = atoi(argv[1]);
        }
    }
    // If no options are given, prompt user to choose an option
    else {
        printf("[0] all tests\n[1] serial/scalar\n[2] serial/simd\n[3] cilk_spawn/scalar\n[4] cilk_spawn/simd\n  > ");
        scanf("%i", &option);
    }
#endif // !PERF_NUM
    */

    // Load up the Intel(R) Cilk(TM) Plus runtime to to get accurate performance numbers
    load_cilk_runtime();

    switch (option) {
    case 0:
        printf("\nRunning all tests\n");

        printf("Starting serial, scalar sample...\n");
        dotest_serial();
        
        printf("\nStarting pragma simd sample...\n");
        dotest_simd();

        printf("\nStarting cilk_spawn sample...\n");
        dotest_cilk_spawn();     

        printf("\nStarting cilk_spawn + pragma simd sample...\n");
        dotest_cilk_spawn_simd();
        break;

    case 1:
        printf("Starting serial, scalar sample...\n");
        dotest_serial();
        break;

    case 2:
        printf("\nStarting pragma simd sample...\n");
        dotest_simd();
        break;

    case 3:
        //printf("\nStarting cilk_spawn sample...\n");
        dotest_cilk_spawn();
        break;

    case 4:
        printf("\nStarting cilk_spawn + pragma simd sample...\n");
        dotest_cilk_spawn_simd();
        break;

    default:
        printf("Please pick a valid option\n");
        break;
    }
#endif // __INTEL_COMPILER is defined

  //release memory
  delete[] g_grid3D[1];
  delete[] g_grid3D[0];
  delete[] g_grid3D;
  delete[] g_vsq;

#ifdef _WIN32
    system("PAUSE");
#endif
    return 0; 
}
