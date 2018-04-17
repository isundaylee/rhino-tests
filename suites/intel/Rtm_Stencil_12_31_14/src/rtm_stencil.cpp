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

// Four functions will compute stencil arrays in different ways: 
// One using serial, scalar code, one using simd for innerest loop.
// One calls scalar code using cilk_spawn, and one calls simd code using cilk_spawn.

// In addition. there are 2 base functions used to be called by functions using cilk_spawn:
// One using scalar code, one using pragma simd for inner loop.

#include "rtm_stencil.h"

// Description:
// This function computes a 25-point 3D stencil with space from x0-x1, y0-y1, z0-z1, and time from t0-t1.
//
// Calculation is done serially and scalar.
//
// [in]: t0, t1, x0, x1, y0, y1, z0, z1
// [out]: g_grid3D
void loop_stencil(int t0, int t1, 
         int x0, int x1,
         int y0, int y1,
         int z0, int z1)
{
  int num_xy = c_num_x * c_num_y;
  int num_x2 = c_num_x * 2;
  int num_x3 = c_num_x * 3;
  int num_x4 = c_num_x * 4;
  int num_xy2 = num_xy * 2;
  int num_xy3 = num_xy * 3;
  int num_xy4 = num_xy * 4;

  // March forward in time
  for(int t = t0; t < t1; ++t) {

    // March over 3D Cartesian grid
    for(int z = z0; z < z1; ++z) {
      for(int y = y0; y < y1; ++y) {
        for(int x = x0; x < x1; ++x) {    
          
          // 25-point stencil applied to g_grid3D, centered at point x,y,z
          int point_xyz = z * num_xy + y * c_num_x + x;
          float *grid3D_cur = &g_grid3D[t & 1][point_xyz];
          float *grid3D_next = &g_grid3D[(t + 1) & 1][point_xyz];
          float div = c_coef[0] * grid3D_cur[0] 
            + c_coef[1] * ((grid3D_cur[0 + 1] + grid3D_cur[0 - 1])
              + (grid3D_cur[0 + c_num_x] + grid3D_cur[0 - c_num_x])
              + (grid3D_cur[0 + num_xy] + grid3D_cur[0 - num_xy]))
            + c_coef[2] * ((grid3D_cur[0 + 2] + grid3D_cur[0 - 2])
              + (grid3D_cur[0 + num_x2] + grid3D_cur[0 - num_x2])
              + (grid3D_cur[0 + num_xy2] + grid3D_cur[0 - num_xy2]))
            + c_coef[3] * ((grid3D_cur[0 + 3] + grid3D_cur[0 - 3])
              + (grid3D_cur[0 + num_x3] + grid3D_cur[0 - num_x3])
              + (grid3D_cur[0 + num_xy3] + grid3D_cur[0 - num_xy3]))
            + c_coef[4] * ((grid3D_cur[0 + 4] + grid3D_cur[0 - 4])
              + (grid3D_cur[0 + num_x4] + grid3D_cur[0 - num_x4])
              + (grid3D_cur[0 + num_xy4] + grid3D_cur[0 - num_xy4]));
          grid3D_next[0] = 2 * grid3D_cur[0] - grid3D_next[0] + g_vsq[point_xyz] * div;
        }
      }
    }
  }
}

#ifdef __INTEL_COMPILER
// Description:
// This function computes a 25-point 3D stencil with space from x0-x1, y0-y1, z0-z1, and time from t0-t1.
//
// Calculation is done using pragma simd for inner most loop.
//
// [in]: t0, t1, x0, x1, y0, y1, z0, z1
// [out]: g_grid3D
void loop_stencil_simd(int t0, int t1, 
         int x0, int x1,
         int y0, int y1,
         int z0, int z1)
{
  int num_xy = c_num_x * c_num_y;
  int num_x2 = c_num_x * 2;
  int num_x3 = c_num_x * 3;
  int num_x4 = c_num_x * 4;
  int num_xy2 = num_xy * 2;
  int num_xy3 = num_xy * 3;
  int num_xy4 = num_xy * 4;

  // March forward in time
  for(int t = t0; t < t1; ++t) {

    // March over 3D Cartesian grid  
    for(int z = z0; z < z1; ++z) {
      for(int y = y0; y < y1; ++y) {
        int point_yz = z * num_xy + y * c_num_x;
        float * grid3D_cur = &g_grid3D[t & 1][point_yz];
        float * grid3D_next = &g_grid3D[(t + 1) & 1][point_yz];
#pragma simd
        for(int x = x0; x < x1; ++x) {

          // 25-point stencil applied to g_grid3D, centered at point x,y,z
          float div = c_coef[0] * grid3D_cur[x] 
            + c_coef[1] * ((grid3D_cur[x + 1] + grid3D_cur[x - 1])
              + (grid3D_cur[x + c_num_x] + grid3D_cur[x - c_num_x])
              + (grid3D_cur[x + num_xy] + grid3D_cur[x - num_xy]))
            + c_coef[2] * ((grid3D_cur[x + 2] + grid3D_cur[x - 2])
              + (grid3D_cur[x + num_x2] + grid3D_cur[x - num_x2])
              + (grid3D_cur[x + num_xy2] + grid3D_cur[x - num_xy2]))
            + c_coef[3] * ((grid3D_cur[x + 3] + grid3D_cur[x - 3])
              + (grid3D_cur[x + num_x3] + grid3D_cur[x - num_x3])
              + (grid3D_cur[x + num_xy3] + grid3D_cur[x - num_xy3]))
            + c_coef[4] * ((grid3D_cur[x + 4] + grid3D_cur[x - 4])
              + (grid3D_cur[x + num_x4] + grid3D_cur[x - num_x4])
              + (grid3D_cur[x + num_xy4] + grid3D_cur[x - num_xy4]));
          grid3D_next[x] = 2 * grid3D_cur[x] - grid3D_next[x] + g_vsq[point_yz+x] * div;
        }
      }
    }
  }
}


// Description:
// This function computes a 25-point 3D stencil with a chunk size space at a period of time 
// from x0-x1, y0-y1, z0-z1 and t0-t1. The points location is adjusted by dx0, dx1, dy0, dy1, 
// dz0 and dz1 respectively,
//
// Calculation is done in scalar.
//
// [in]: t0, t1, x0, dx0, x1, dx1, y0, dy0, y1, dy1, z0, dz0, z1, dz1
// [out]: g_grid3D
extern "C++" void co_basecase_nv(int t0, int t1, 
              int x0, int dx0, int x1, int dx1,
              int y0, int dy0, int y1, int dy1, 
              int z0, int dz0, int z1, int dz1 )
{
  float coef0 = c_coef[0], coef1 = c_coef[1], coef2 = c_coef[2], coef3 = c_coef[3], coef4 = c_coef[4];
  int num_x = c_num_x;
  int num_xy = num_x * c_num_y;
  int num_x2 = num_x * 2;
  int num_x3 = num_x * 3;
  int num_x4 = num_x * 4;
  int num_xy2 = num_xy * 2;
  int num_xy3 = num_xy * 3;
  int num_xy4 = num_xy * 4;

  // March forward from time period t0 to t1
  for(int t = t0; t < t1; ++t) {

    // March over the chunk size 3D Cartesian grid 
    for(int z = z0; z < z1; ++z) {
      for(int y = y0; y < y1; ++y) {
        for(int x = x0; x < x1; ++x) {

          // 25-point stencil applied to g_grid3D, centered at point x,y,z
          int point_xyz = z * num_xy + y * num_x + x;
          float *grid3D_cur = &g_grid3D[t & 1][point_xyz];
          float *grid3D_next = &g_grid3D[(t + 1) & 1][point_xyz];
          float div = coef0 * grid3D_cur[0] 
            + coef1 * ((grid3D_cur[1] + grid3D_cur[-1])
                    + (grid3D_cur[num_x] + grid3D_cur[0 - num_x])
                    + (grid3D_cur[num_xy] + grid3D_cur[0 - num_xy]))
            + coef2 * ((grid3D_cur[2] + grid3D_cur[-2])
                    + (grid3D_cur[num_x2] + grid3D_cur[0 - num_x2])
                    + (grid3D_cur[num_xy2] + grid3D_cur[0 - num_xy2]))
            + coef3 * ((grid3D_cur[3] + grid3D_cur[0 - 3])
                    + (grid3D_cur[num_x3] + grid3D_cur[0 - num_x3])
                    + (grid3D_cur[num_xy3] + grid3D_cur[0 - num_xy3]))
            + coef4 * ((grid3D_cur[4] + grid3D_cur[-4])
                    + (grid3D_cur[num_x4] + grid3D_cur[0 - num_x4])
                    + (grid3D_cur[num_xy4] + grid3D_cur[0 - num_xy4]));
          grid3D_next[0] = 2 * grid3D_cur[0] - grid3D_next[0] + g_vsq[point_xyz] * div;
        }
      }
    }

    x0 += dx0; x1 += dx1;
    y0 += dy0; y1 += dy1;
    z0 += dz0; z1 += dz1;
  }
}

// Description:
// This function computes a 25-point 3D stencil with space from x0-x1, y0-y1, z0-z1, and time from t0-t1.
//
// The simplest strategy to parallelize this stencil problem is to use a parallel loop, however, 
// memory bandwidth could become the major bottleneck for getting good speedup on multi-core processors.  
// We developed cache oblivious implementations to optimize the cache usage and program performance.  
// For the description and analysis of the cache oblivious algorithm, please refer to the paper 
// "The cache complexity of multithreaded cache oblivious algorithms" written by Matteo Frigo and Volker Strumpen.
//
// Calculation is done using cilk_spawn calling a scalar base function.
//
// [in]: t0, t1, x0, dx0, x1, dx1, y0, dy0, y1, dy1, z0, dz0, z1, dz1
// [out]: g_grid3D
void co_cilk(int t0, int t1, 
           int x0, int dx0, int x1, int dx1,
           int y0, int dy0, int y1, int dy1, 
           int z0, int dz0, int z1, int dz1 )
{
  int dt = t1 - t0, dx = x1 - x0, dy = y1 - y0, dz = z1 - z0;
  int i;

  // Divide 3D Cartesian grid into chunk size and time period
  if (dx >= c_dx_threshold && dx >= dy && dx >= dz &&
      dt >= 1 && dx >= 2 * c_distance * dt * c_NPIECES) {
    //divide and conquer along x direction
    int chunk = dx / c_NPIECES;

    for (i = 0; i < c_NPIECES - 1; ++i)
      cilk_spawn co_cilk(t0, t1,
                       x0 + i * chunk, c_distance, x0 + (i+1) * chunk, -c_distance,
                       y0, dy0, y1, dy1,
                       z0, dz0, z1, dz1);
    /*nospawn*/co_cilk(t0, t1,
                       x0 + i * chunk, c_distance, x1, -c_distance,
                       y0, dy0, y1, dy1, 
                       z0, dz0, z1, dz1);
    cilk_sync;
    cilk_spawn co_cilk(t0, t1, 
                       x0, dx0, x0, c_distance,
                       y0, dy0, y1, dy1, 
                       z0, dz0, z1, dz1);
    for (i = 1; i < c_NPIECES; ++i)
      cilk_spawn co_cilk(t0, t1,
                       x0 + i * chunk, -c_distance, x0 + i * chunk, c_distance,
                       y0, dy0, y1, dy1, 
                       z0, dz0, z1, dz1);
    /*nospawn*/co_cilk(t0, t1, 
                       x1, -c_distance, x1, dx1,
                       y0, dy0, y1, dy1, 
                       z0, dz0, z1, dz1);
  } else if (dy >= c_dyz_threshold && dy >= dz && dt >= 1 && dy >= 2 * c_distance * dt * c_NPIECES) {
    //similarly divide and conquer along y direction
    int chunk = dy / c_NPIECES;

    for (i = 0; i < c_NPIECES - 1; ++i)
      cilk_spawn co_cilk(t0, t1,
                       x0, dx0, x1, dx1,
                       y0 + i * chunk, c_distance, y0 + (i+1) * chunk, -c_distance, 
                       z0, dz0, z1, dz1);
    /*nospawn*/co_cilk(t0, t1,
                       x0, dx0, x1, dx1,
                       y0 + i * chunk, c_distance, y1, -c_distance, 
                       z0, dz0, z1, dz1);
    cilk_sync;
    cilk_spawn co_cilk(t0, t1, 
                     x0, dx0, x1, dx1,
                     y0, dy0, y0, c_distance, 
                     z0, dz0, z1, dz1);
    for (i = 1; i < c_NPIECES; ++i)
      cilk_spawn co_cilk(t0, t1,
                       x0, dx0, x1, dx1,
                       y0 + i * chunk, -c_distance, y0 + i * chunk, c_distance, 
                       z0, dz0, z1, dz1);
    /*nospawn*/co_cilk(t0, t1, 
                       x0, dx0, x1, dx1,
                       y1, -c_distance, y1, dy1, 
                       z0, dz0, z1, dz1);
  } else if (dz >= c_dyz_threshold && dt >= 1 && dz >= 2 * c_distance * dt * c_NPIECES) {
    //similarly divide and conquer along z direction
    int chunk = dz / c_NPIECES;

    for (i = 0; i < c_NPIECES - 1; ++i)
      cilk_spawn co_cilk(t0, t1,
                       x0, dx0, x1, dx1,
                       y0, dy0, y1, dy1,
                       z0 + i * chunk, c_distance, z0 + (i+1) * chunk, -c_distance);
    /*nospawn*/co_cilk(t0, t1,
                       x0, dx0, x1, dx1,
                       y0, dy0, y1, dy1, 
                       z0 + i * chunk, c_distance, z1, -c_distance);
    cilk_sync;
    cilk_spawn co_cilk(t0, t1, 
                       x0, dx0, x1, dx1,
                       y0, dy0, y1, dy1,
                       z0, dz0, z0, c_distance);
    for (i = 1; i < c_NPIECES; ++i)
      cilk_spawn co_cilk(t0, t1,
                       x0, dx0, x1, dx1,
                       y0, dy0, y1, dy1,
                       z0 + i * chunk, -c_distance, z0 + i * chunk, c_distance);
    /*nospawn*/ co_cilk(t0, t1, 
                       x0, dx0, x1, dx1,
                       y0, dy0, y1, dy1,
                       z1, -c_distance, z1, dz1);
  }  else if (dt > c_dt_threshold) {
    int halfdt = dt / 2;
    //decompose over time direction
    co_cilk(t0, t0 + halfdt,
          x0, dx0, x1, dx1,
          y0, dy0, y1, dy1, 
          z0, dz0, z1, dz1);
    co_cilk(t0 + halfdt, t1, 
          x0 + dx0 * halfdt, dx0, x1 + dx1 * halfdt, dx1,
          y0 + dy0 * halfdt, dy0, y1 + dy1 * halfdt, dy1, 
          z0 + dz0 * halfdt, dz0, z1 + dz1 * halfdt, dz1);
  } else {
    co_basecase_nv(t0, t1, 
                 x0, dx0, x1, dx1,
                 y0, dy0, y1, dy1,
                 z0, dz0, z1, dz1);
  }
  cilk_sync;
}

// Description:
// This function computes a 25-point 3D stencil with a chunk size space at a period of time 
// from x0-x1, y0-y1, z0-z1 and t0-t1. The points location is adjusted by dx0, dx1, dy0, dy1, 
// dz0 and dz1 respectively,
//
// Calculation is done using pragma simd for the inner most loop.
//
// [in]: t0, t1, x0, dx0, x1, dx1, y0, dy0, y1, dy1, z0, dz0, z1, dz1
// [out]: g_grid3D
extern "C++" void co_basecase_simd(int t0, int t1, 
              int x0, int dx0, int x1, int dx1,
              int y0, int dy0, int y1, int dy1, 
              int z0, int dz0, int z1, int dz1 )
{
  float coef0 = c_coef[0], coef1 = c_coef[1], coef2 = c_coef[2], coef3 = c_coef[3], coef4 = c_coef[4];
  int num_x = c_num_x;
  int num_y = c_num_y;
  int num_z = c_num_z;
  int num_xy = num_x * num_y;
  int num_x2 = num_x * 2;
  int num_x3 = num_x * 3;
  int num_x4 = num_x * 4;
  int num_xy2 = num_xy * 2;
  int num_xy3 = num_xy * 3;
  int num_xy4 = num_xy * 4;

  // March forward from time period t0 to t1
  for(int t = t0; t < t1; ++t) {
    
    // March over the chunk size 3D Cartesian grid
    for(int z = z0; z < z1; ++z) {
      assert( 0 <= z <= num_z );
      for(int y = y0; y < y1; ++y) {
        int point_yz = z * num_xy + y * num_x;
        float *grid3D_cur = &g_grid3D[t & 1][point_yz];
        float *grid3D_next = &g_grid3D[(t + 1) & 1][point_yz];
#pragma simd
        for(int x = x0; x < x1; ++x) {        
          assert( 0 <= x <= num_x );
          assert( 0 <= y <= num_y );
          // 25-point stencil applied to g_grid3D, centered at point x,y,z        
          float div = coef0 * grid3D_cur[x] 
            + coef1 * ((grid3D_cur[x+1] + grid3D_cur[x-1])
                  + (grid3D_cur[x+num_x] + grid3D_cur[x - num_x])
                  + (grid3D_cur[x+num_xy] + grid3D_cur[x - num_xy]))
            + coef2 * ((grid3D_cur[x+2] + grid3D_cur[x-2])
                  + (grid3D_cur[x+num_x2] + grid3D_cur[x - num_x2])
                  + (grid3D_cur[x+num_xy2] + grid3D_cur[x - num_xy2]))
            + coef3 * ((grid3D_cur[x+3] + grid3D_cur[x - 3])
                  + (grid3D_cur[x+num_x3] + grid3D_cur[x - num_x3])
                  + (grid3D_cur[x+num_xy3] + grid3D_cur[x - num_xy3]))
            + coef4 * ((grid3D_cur[x+4] + grid3D_cur[x-4])
                  + (grid3D_cur[x+num_x4] + grid3D_cur[x - num_x4])
                  + (grid3D_cur[x+num_xy4] + grid3D_cur[x - num_xy4]));
          grid3D_next[x] = 2 * grid3D_cur[x] - grid3D_next[x] + g_vsq[point_yz+x] * div;
        }
      }
    }
    x0 += dx0; x1 += dx1;
    y0 += dy0; y1 += dy1;
    z0 += dz0; z1 += dz1;
  }
}

// Description:
// This function computes a 25-point 3D stencil with space from x0-x1, y0-y1, z0-z1, and time from t0-t1.
//
// The simplest strategy to parallelize this stencil problem is to use a parallel loop, however, 
// memory bandwidth could become the major bottleneck for getting good speedup on multi-core processors.  
// We developed cache oblivious implementations to optimize the cache usage and program performance.  
// For the description and analysis of the cache oblivious algorithm, please refer to the paper 
// "The cache complexity of multithreaded cache oblivious algorithms" written by Matteo Frigo and Volker Strumpen.
//
// Calculation is done using cilk_spawn calling a simd base function.
//
// [in]: t0, t1, x0, dx0, x1, dx1, y0, dy0, y1, dy1, z0, dz0, z1, dz1
// [out]: g_grid3D
void co_cilksimd(int t0, int t1, 
           int x0, int dx0, int x1, int dx1,
           int y0, int dy0, int y1, int dy1, 
           int z0, int dz0, int z1, int dz1 )
{
  int dt = t1 - t0, dx = x1 - x0, dy = y1 - y0, dz = z1 - z0;
  int i;

  // Divide 3D Cartesian grid into chunk size and time period
  if (dx >= c_dx_threshold && dx >= dy && dx >= dz &&
      dt >= 1 && dx >= 2 * c_distance * dt * c_NPIECES) {
    //divide and conquer along x direction
    int chunk = dx / c_NPIECES;

    for (i = 0; i < c_NPIECES - 1; ++i)
      cilk_spawn co_cilksimd(t0, t1,
                           x0 + i * chunk, c_distance, x0 + (i+1) * chunk, -c_distance,
                           y0, dy0, y1, dy1,
                           z0, dz0, z1, dz1);
    /*nospawn*/co_cilksimd(t0, t1,
                           x0 + i * chunk, c_distance, x1, -c_distance,
                           y0, dy0, y1, dy1, 
                           z0, dz0, z1, dz1);
    cilk_sync;
    cilk_spawn co_cilksimd(t0, t1, 
                           x0, dx0, x0, c_distance,
                           y0, dy0, y1, dy1, 
                           z0, dz0, z1, dz1);
    for (i = 1; i < c_NPIECES; ++i)
      cilk_spawn co_cilksimd(t0, t1,
                           x0 + i * chunk, -c_distance, x0 + i * chunk, c_distance,
                           y0, dy0, y1, dy1, 
                           z0, dz0, z1, dz1);
    /*nospawn*/co_cilksimd(t0, t1, 
                           x1, -c_distance, x1, dx1,
                           y0, dy0, y1, dy1, 
                           z0, dz0, z1, dz1);
  } else if (dy >= c_dyz_threshold && dy >= dz && dt >= 1 && dy >= 2 * c_distance * dt * c_NPIECES) {
    //similarly divide and conquer along y direction
    int chunk = dy / c_NPIECES;

    for (i = 0; i < c_NPIECES - 1; ++i)
      cilk_spawn co_cilksimd(t0, t1,
                           x0, dx0, x1, dx1,
                           y0 + i * chunk, c_distance, y0 + (i+1) * chunk, -c_distance, 
                           z0, dz0, z1, dz1);
    /*nospawn*/co_cilksimd(t0, t1,
                           x0, dx0, x1, dx1,
                           y0 + i * chunk, c_distance, y1, -c_distance, 
                           z0, dz0, z1, dz1);
    cilk_sync;
    cilk_spawn co_cilksimd(t0, t1, 
                           x0, dx0, x1, dx1,
                           y0, dy0, y0, c_distance, 
                           z0, dz0, z1, dz1);
    for (i = 1; i < c_NPIECES; ++i)
      cilk_spawn co_cilksimd(t0, t1,
                           x0, dx0, x1, dx1,
                           y0 + i * chunk, -c_distance, y0 + i * chunk, c_distance, 
                           z0, dz0, z1, dz1);
    /*nospawn*/co_cilksimd(t0, t1, 
                           x0, dx0, x1, dx1,
                           y1, -c_distance, y1, dy1, 
                           z0, dz0, z1, dz1);
  } else if (dz >= c_dyz_threshold && dt >= 1 && dz >= 2 * c_distance * dt * c_NPIECES) {
    //similarly divide and conquer along z
    int chunk = dz / c_NPIECES;

    for (i = 0; i < c_NPIECES - 1; ++i)
      cilk_spawn co_cilksimd(t0, t1,
                           x0, dx0, x1, dx1,
                           y0, dy0, y1, dy1,
                           z0 + i * chunk, c_distance, z0 + (i+1) * chunk, -c_distance);
    /*nospawn*/co_cilksimd(t0, t1,
                           x0, dx0, x1, dx1,
                           y0, dy0, y1, dy1, 
                           z0 + i * chunk, c_distance, z1, -c_distance);
    cilk_sync;
    cilk_spawn co_cilksimd(t0, t1, 
                           x0, dx0, x1, dx1,
                           y0, dy0, y1, dy1,
                           z0, dz0, z0, c_distance);
    for (i = 1; i < c_NPIECES; ++i)
      cilk_spawn co_cilksimd(t0, t1,
                           x0, dx0, x1, dx1,
                           y0, dy0, y1, dy1,
                           z0 + i * chunk, -c_distance, z0 + i * chunk, c_distance);
    /*nospawn*/co_cilksimd(t0, t1, 
                           x0, dx0, x1, dx1,
                           y0, dy0, y1, dy1,
                           z1, -c_distance, z1, dz1);
  }  else if (dt > c_dt_threshold) {
    int halfdt = dt / 2;
    //decompose over time direction
    co_cilksimd(t0, t0 + halfdt,
              x0, dx0, x1, dx1,
              y0, dy0, y1, dy1, 
              z0, dz0, z1, dz1);
    co_cilksimd(t0 + halfdt, t1, 
              x0 + dx0 * halfdt, dx0, x1 + dx1 * halfdt, dx1,
              y0 + dy0 * halfdt, dy0, y1 + dy1 * halfdt, dy1, 
              z0 + dz0 * halfdt, dz0, z1 + dz1 * halfdt, dz1);
  } else {
    co_basecase_simd(t0, t1, 
                   x0, dx0, x1, dx1,
                   y0, dy0, y1, dy1,
                   z0, dz0, z1, dz1);
  }
  cilk_sync; 
}
#endif
