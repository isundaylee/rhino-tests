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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "timer.h"
#include "complete_graph.h"

// Run set flag
#define RUN_ALL     0x00000001    
#define RUN_SERIAL  0x00000002
#define RUN_AN      0x00000004
#define RUN_CFOR    0x00000008
#define RUN_AN_CFOR 0x00000010

// Description
// Main function of the program
// Return value: 0 on success and -1 on an invalid user input path
int main(int argc, char * argv[])
{
 // Timer 
 CUtilTimer tm;

 // Create a graph
 init_graph();

#ifdef DEBUG
 // Dump the graph adjacency matrix to a file for debugging
 dump_graph_edge();
#endif

// Initialize run set flag to run all tests.
 unsigned int run_flag = 0;
 int option = 3;

#ifdef __INTEL_COMPILER
#ifdef PERF_NUM
// Run all tests if PERF_NUM defined
 run_flag = RUN_ALL;
#else
/*
 if(argc>1) {
     // Prints out instructions and quits
     if(argv[1][0] == 'h') {
         printf("Dijkstra shortest path algorithm for a complete graph having 2000 vertexes\n");
         printf("[0] all tests\n[1] serial/scalar\n[2] serial/array notation\n[3] cilk_for/scalar\n[4] cilk_for/array notation\n");
#ifdef _WIN32
         system("PAUSE");
#endif // _WIN32
         return 0;
     }
 // option is assumed an option
     else 
         option = atoi(argv[1]);
 }
 else {
     printf("Dijkstra shortest path algorithm for a complete graph having 2000 vertexes\n");
     printf("[0] all tests\n[1] serial/scalar\n[2] serial/array notation\n[3] cilk_for/scalar\n[4] cilk_for/array notation\n  > ");
     scanf("%i", &option);
 }
 if ((option < 0) || (option > 4)) {
     printf("Please pick a valid option\n");
#ifdef _WIN32
     system("PAUSE");
#endif
     return 0;
 }
*/
 run_flag = 0x01 << option;
#endif
#else
// Only run serial, scalar sample if not Intel compiler used
 run_flag = RUN_SERIAL;
#endif

// Load up the Intel(R) Cilk(TM) Plus runtime to to get accurate performance numbers
 double g = 2.0;
 for (int i = 0; i < 100; i++) {
     g /= sin(g);
 }

#ifdef CHECK_RESULT
 printf("Starting non-optimized serial, scalar shortest path for correctness checking.\n");
 // Start the timer
 tm.start();
 // Calcuate the shortest path
 calculate_shortest_path_base();
 // Stop the timer
 tm.stop();
 // Print the time consumed by calculating the shortest path
 printf("Calculating finished. Time taken is %.0fms\n",tm.get_time()*1000.0);
#endif

#ifdef DEBUG
 // Dump the shortest path length result to a file for debugging
 dump_spath();
#endif
 

 double avg_time = 0.0;

#ifdef PERF_NUM
 for (int i = 0; i < 5;i++) {
#endif

     if (run_flag & (RUN_CFOR|RUN_ALL)) {
         //printf("\nStarting cilk_for shortest path...\n");
         // Start the timer
         tm.start();
         // Calcuate the shortest path
         calculate_shortest_path_cfor();
         // Stop the timer
         tm.stop();
         // Print the time consumed by calculating the shortest path
         avg_time += tm.get_time();
         printf("%.0f",tm.get_time()*1000.0);
#ifdef CHECK_RESULT
         if (!check_result()) {
             printf("Result is incorrect!\n");
             return -1;
         }
#endif
     }
     
#ifdef PERF_NUM
 }
 printf("avg time: %.0fms\n", avg_time*1000.0/5);
#endif

#ifdef CHECK_RESULT
 printf("Result is correct!\n");
#endif

 return 0;
}


