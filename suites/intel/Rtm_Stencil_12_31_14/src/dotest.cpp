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


// This code listed utility functions used to initialize the stencil array, print out summary report
// and run test with different parallelization and vectorization methods.


#include "rtm_stencil.h"

// Points to memory storing the current and new/next space value
// The current and new space value is switching from g_grid3D[0] and g_grid3D[1] through Time
float **g_grid3D;

// Operator array stores values to calculate stencil in each dimension
float *g_vsq;

const int c_num_xy = c_num_x*c_num_y;

// Description:
// This function computes reference point from 3D space and Time in g_grid3D.
// [in]: t, x, y, z
// [out]: aref
static inline float &aref(int t, int x, int y, int z)
{
  return g_grid3D[t & 1][c_num_xy * z + c_num_x * y + x];
}

// Description:
// This function computes reference point from one dimension space and Time in g_grid3D.
// [in]: t, point_xyz
// [out]: aref
static inline float &aref(int t, int point_xyz) {
  return g_grid3D[t & 1][point_xyz];
}

// Description:
// This function computes reference point from 3D space in g_vsq.
// [in]: x, y, z
// [out]: vsqref
static inline float &vsqref(int x, int y, int z)
{
  return g_vsq[c_num_xy * z + c_num_x * y + x];
}

// Description:
// This function computes the reference of one point from one dimension space in g_vsq.
// [in]: point_xyz
// [out]: vsqref
static inline float &vsqref(int point_xyz)
{
  return g_vsq[point_xyz];
}

// Description:
// This function initialize stencil array g_grid3D and operator array in g_vsq.
// [in]: 
// [out]: g_grid3D, g_vsq
void init_variables() 
{ 
  for (int z = 0; z < c_num_z; ++z)
    for (int y = 0; y < c_num_y; ++y) 
      for (int x = 0; x < c_num_x; ++x) {
        /* set initial values */
        float r = fabs((float)(x - c_num_x/2 + y - c_num_y/2 + z - c_num_z/2) / 30);
        r = max(1 - r, 0.0f) + 1;

        aref(0, x, y, z) = r;
        aref(1, x, y, z) = r;
        vsqref(x, y, z) = 0.001f;  
      }
}


// Description:
// This function calculates throughput performance and 
// print out both time and throughput perfomrance result.
// [in]: header, interval
// [out]: 
void print_summary(char *header, double interval) {
  /* print timing information */
  long total = (long)c_num_x * c_num_y * c_num_z;
  //printf("++++++++++ %s ++++++++++\n", header);
  //printf("first non-zero numbers\n");
  for(int i = 0; i < total; i++) {
    if(g_grid3D[c_time%2][i] != 0) {
      //printf("%d: %fs\n", i, g_grid3D[c_time%2][i]);
      break;
    }
  }
  
  double mul = c_num_x-8;
  mul *= c_num_y-8;
  mul *= c_num_z-8;
  mul *= c_time;
  double perf = mul / (interval * 1e6);

  printf("%f\n", interval);

  // Mcells/sec means number of Million cells are processed per second
  // M-FAdd/s means number of floating point add operations are processed per second for one million cells
  // M-FMul/s means number of floating point multiply operations are processed per second for one million cells
  //printf("Perf: %f Mcells/sec (%f M-FAdd/s, %f M-FMul/s)\n", 
  //  perf, 
  //  perf * 26, 
  //  perf * 7);
}


// Description:
// This function output value of points in dimension y to file y_points_name.txt file.
// Name is passing to identify the result for different methods.
// [in]: name
// [out]: 
void print_y(char *name) {
  char filename[35];  
  sprintf(filename, "y_points_%s.txt", name);  
  FILE *fout = fopen(filename, "w");
  int z = c_num_z/2;
  int x = c_num_x/2;
  for(int y = 0; y < c_num_y; y++) {
    fprintf(fout, "%f\n", aref(c_time, x, y, z));
  }
  fclose(fout);
  //printf("Done writing output\n");
}


// Description:
// This function runs stencil calculation test using strictly serial, scalar methods.
// Calls loop_stencil to do the calculation
// Calls print_summary to give out the timing report.
// Calls print_y to output result values of points in dimension y.
// [in]: 
// [out]: 
void dotest_serial()
{
  CUtilTimer timer;
  
  //initialization
  init_variables();

#ifdef PERF_NUM
  double avg_time = 0;
  for(int i=0; i<5; ++i) {
#endif // PERF_NUM
  timer.start();
          
  loop_stencil(0, c_time, 
             c_distance, c_num_x - c_distance, 
             c_distance, c_num_y - c_distance,  
             c_distance, c_num_z - c_distance);

  timer.stop();
#ifdef PERF_NUM
    printf("Calculation finished. Time taken is %.0fms\n", timer.get_time()*1000.0);
#else
  print_summary((char*)"stencil_loop", timer.get_time());
#endif
#ifdef PERF_NUM
    avg_time += timer.get_time();
  }
  print_summary((char*)"stencil_loop", avg_time/5);
#endif // PERF_NUM*/

  //print result in y axis
  print_y("serial");  
}


#ifdef __INTEL_COMPILER
// Description:
// This function runs test using pragma simd for vectorized work
// Calls loop_stencil_simd to do the calculation
// Calls print_summary to give out the timing report.
// Calls print_y to output result values of points in dimension y.
// [in]: 
// [out]: 
void dotest_simd()
{
  CUtilTimer timer;

  //initialization
  init_variables();
  
#ifdef PERF_NUM
  double avg_time = 0;
  for(int i=0; i<5; ++i) {
#endif // PERF_NUM
  timer.start();
          
  loop_stencil_simd(0, c_time, 
                  c_distance, c_num_x - c_distance, 
                  c_distance, c_num_y - c_distance,  
                  c_distance, c_num_z - c_distance);

  timer.stop();
#ifdef PERF_NUM
    printf("Calculation finished. Time taken is %.0fms\n", timer.get_time()*1000.0);
#else
    print_summary((char*)"stencil_loop_simd", timer.get_time());
#endif
#ifdef PERF_NUM
    avg_time += timer.get_time();
  }
  print_summary((char*)"stencil_loop_simd", avg_time/5);  
#endif // PERF_NUM*/
  
  //print result in y axis
  print_y("simd");
}


// Description:
// This function runs test using cilk_spawn for parallelized work
// Calls co_cilk to do the calculation
// Calls print_summary to give out the timing report.
// Calls print_y to output result values of points in dimension y.
// [in]: 
// [out]: 
void dotest_cilk_spawn()
{
  //initialization
  init_variables();

  CUtilTimer timer;
  
#ifdef PERF_NUM
  double avg_time = 0;
  for(int i=0; i<5; ++i) {
#endif // PERF_NUM
  timer.start();
         
  co_cilk(0, c_time, 
        c_distance, 0, c_num_x - c_distance, 0,
        c_distance, 0, c_num_y - c_distance, 0, 
        c_distance, 0, c_num_z - c_distance, 0);

  timer.stop();
#ifdef PERF_NUM
     printf("Calculation finished. Time taken is %.0fms\n", timer.get_time()*1000.0);
#else
  print_summary((char*)"cilk_spawn", timer.get_time());
#endif
#ifdef PERF_NUM
    avg_time += timer.get_time();
    }
  print_summary((char*)"cilk_spawn", avg_time/5);
#endif // PERF_NUM*/

  //print result in y axis
  print_y("cilk_spawn"); 
}


// Description:
// This function runs test using cilk_spawn and pragma simd for parallelized and vectorized work
// Calls co_cilk_simd to do the calculation
// Calls print_summary to give out the timing report.
// Calls print_y to output result values of points in dimension y.
// [in]: 
// [out]: 
void dotest_cilk_spawn_simd()
{
  //initialization
  g_grid3D = new float*[2];
  g_grid3D[0] = new float[c_num_x * c_num_y * c_num_z];
  g_grid3D[1] = new float[c_num_x * c_num_y * c_num_z];
  g_vsq = new float[c_num_x * c_num_y * c_num_z];
  CUtilTimer timer;

  ///////////////////////////////////////////////
  init_variables();  
  
#ifdef PERF_NUM
  double avg_time = 0;
  for(int i=0; i<5; ++i) {
#endif // PERF_NUM
  timer.start();
         
  co_cilksimd(0, c_time, 
           c_distance, 0, c_num_x - c_distance, 0,
           c_distance, 0, c_num_y - c_distance, 0, 
           c_distance, 0, c_num_z - c_distance, 0);

  timer.stop();
#ifdef PERF_NUM
    printf("Calculation finished. Time taken is %.0fms\n", timer.get_time()*1000.0);
#else
  print_summary((char*)"cilk_spawn_simd", timer.get_time());
#endif
#ifdef PERF_NUM
    avg_time += timer.get_time();
  }
  print_summary((char*)"cilk_spawn_simd", avg_time/5);
#endif // PERF_NUM*/
  
  print_y("cilk_spawn_simd");
}

// Description:
// This function calculates using cilk_spawn and array notation
// Runs to load up the Intel(R) Cilk(TM) Plus runtime to get accurate performance numbers for later calculation
// [in]: 
// [out]: 
void load_cilk_runtime()
{
  init_variables();
  //printf("Initialized variables\n"); fflush(0);
  co_cilksimd(0, c_time, 
           c_distance, 0, c_num_x - c_distance, 0,
           c_distance, 0, c_num_y - c_distance, 0, 
           c_distance, 0, c_num_z - c_distance, 0);
}
#endif
