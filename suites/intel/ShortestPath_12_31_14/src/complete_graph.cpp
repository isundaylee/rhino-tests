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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef __INTEL_COMPILER
#include <cilk/cilk.h>
#endif

#include "timer.h"
#include "complete_graph.h"

// Graph adjacency matrix
unsigned int graph[VNUM][VNUM];

// Shortest path length result matrix of the optimized
unsigned int spath_opt[VNUM][VNUM];
// Shortest path previous vertex matrix of the optimized
unsigned int pvertex_opt[VNUM][VNUM];

#ifdef CHECK_RESULT
// Shortest path length result matrix of base line for correctness checking
unsigned int spath_base[VNUM][VNUM];
// Shortest path previous vertex matrix of base line for correctness checking
unsigned int pvertex_base[VNUM][VNUM];
#endif

// Description:
// Create a complete graph having VNUM vertexes and save it to adjacency matrix "graph"
// Initialize shortest path length to edge length
// Initialize previous vertex on each path to its start vertex
void init_graph(void)
{
 int i,j;

 srand(RSEED);
 for (i = 0;i < VNUM;++i) {
    for (j = i; j < VNUM;++j) {
        if (i == j) 
            // Set adjacency matrix and shortest path matrix diagonal to zero
            graph[i][j] = 0;
        else
            // Create edge length between "EDGE_MIN" and "EDGE_MAX" by calling random number generator function
            graph[i][j] = graph[j][i] = (int) (EDGE_MIN + (EDGE_MAX * (rand() / (RAND_MAX + EDGE_MIN))));
    }
 }
}


// Description:
// Dump the graph adjacency matrix to a file named EDGE_FILE_NAME
// For debug purpose 
void dump_graph_edge(void)
{
 int i,j;
 // Edge dumping file pointer
 FILE * f_edge;
 
 // Open the dumping file for writing
 if ((f_edge = fopen(EDGE_FILE_NAME,"w")) == NULL) {
    // Return if opening file fails
    printf("Can't open edge file for write!\n");
    return;
 }

 printf("Dumping adjacency matrix to file \"%s\"...\n",EDGE_FILE_NAME);
 // Print header info to the dumping file
 fprintf(f_edge,"        ******** Adjacency Matrix Dump ********\n\n");
 fprintf(f_edge,"   Start             End    Length:\n");
 fprintf(f_edge,"=======================================================\n");
 
 // Dump each edge start vertex, end vertex and edge length
 for (i = 0;i < VNUM;++i) 
    for (j = 0; j < VNUM;++j) 
        fprintf(f_edge,"%8d    --->%8d: %8u\n",i,j,graph[i][j]);

 // Close the dumping file
 fclose(f_edge);
}

// Description:
// Dump the shortest path result to a file named PATH_FILE_NAME
// For debug purpose
void dump_spath(void)
{
 int i,j;
 // Shortest path length dumping file pointer
 FILE * f_path;
 
 // Open the dumping file for writing
 if ((f_path = fopen(PATH_FILE_NAME,"w")) == NULL) {
    // Return if opening file fails
    printf("Can't open path file for write!\n");
    return;
 }

 printf("Dumping shortest path result to file \"%s\"...\n",PATH_FILE_NAME);
 // Print header info to the dumping file
 fprintf(f_path,"        ******** Shortest path length dump ********\n\n");
 fprintf(f_path,"   Start             End    Length:\n");
 fprintf(f_path,"=======================================================\n");

 // Dump each shortest path start vertex, end vertex and path length
 for (i = 0;i < VNUM;++i)
    for (j = 0; j < VNUM;++j) 
        fprintf(f_path,"%8d    ---> %8d: %8u\n",i,j,spath_opt[i][j]);
 
 // Close the dumping file
 fclose(f_path);
}

// Description
// Help function printing out each edge on a shortest path from "start" to "end" using backtracing algorithm
void backtracing_spath(unsigned int start, unsigned int end)
{
 if (pvertex_opt[start][end] == start)
    // If previous vertex on the path is "start" we arrive at the first edge and print it out
    printf("%10u    ----> %10u:   %10u\n",start,end,graph[start][end]);
 else {
    // If previous vertex on the path isn't "start" backtrace the path 
    backtracing_spath(start,pvertex_opt[start][end]);
    // Print out current edge and its length
    printf("%10u    ----> %10u:   %10u\n",pvertex_opt[start][end],end,graph[pvertex_opt[start][end]][end]);
 }
}

// Description
// Print out the length and the edges of a shortest path from "start" to "end" 
void print_spath(unsigned int start, unsigned int end)
{
 printf("The shortest path length from %u to %u is \"%u\".\n",start,end,spath_opt[start][end]);
 printf("The edges on this path are:\n");

 // Call the help function to backtrace the path
 backtracing_spath(start,end);
}

#ifdef CHECK_RESULT
// Description
// Calculate the shortest path between each pair of vertexes in the complete graph using Disjkstra algorithm without any optimization
// Its result will be used for correctness checking
#pragma optimize ("",off)
void calculate_shortest_path_base(void)
{
 int i;
 // Temporary array storing intermedia path length result to each vertex
 unsigned int vtemp[VNUM];
 // Flag array: 
 // "1" means the shortest path hasn't been finished
 // "0" menas the shortest path has been finished
 unsigned char vflag[VNUM];

 // Main loop calculate the shortest path to all other vertexes from vertex "i" in each iteration
 for (i = 0;i < VNUM;++i) { 
    int j;

    // Initialize intermedia path length to INFINITE
    memset(vtemp,0xff,VNUM*sizeof(unsigned int));
    // Place the source vertex 
    vtemp[i] = 0;
    pvertex_base[i][i] = i;
    // Intialize flag array to all "1"
    memset(vflag,1,VNUM);
    // Calculate the "j+1"th shortest path from vertext "i"
    for (j = 0;j < VNUM;++j) {
        // minval: shortest path lengh in vtemp
        // minpos: index of the vertex having the shortest path length
        unsigned int minval, minpos;
        int k;

        // Initialze shortest lenght and vertex index to INFINITE
        minval = minpos = INFINITE;
        // Loop scan vtemp to find the index of vertex having the shortest path in vtemp and its length
        for (k = 0; k < VNUM;k++)
            if (vtemp[k] < minval) {
                minpos = k;
                minval = vtemp[k];
            } 

        // Store the shortest path length found to result matrix
        spath_base[i][minpos] = minval; 
        // Update the length value of the vertex found to INFINITE so that it will be ignored in next round of MIN reduction
        vtemp[minpos] = INFINITE; 
        // Flag the path to the vertex found as finished
        vflag[minpos] = 0;        

        // Update unfinished vertexes path length value using edge length to the found vertex using loop scan
        for (k = 0; k < VNUM;k++)  
            if (vflag[k] && ((graph[minpos][k] + minval) < vtemp[k])) {
                vtemp[k] = (graph[minpos][k] + minval);
                pvertex_base[i][k] = minpos;
            }
    }
 }
}
#pragma optimize ("",on)

// Description
// Check if the result is the same as base line output generated without any optimization
unsigned char check_result(void)
{
 return (!memcmp(spath_base,spath_opt,VNUM*VNUM*sizeof(unsigned int)) && !memcmp(pvertex_base,pvertex_opt,VNUM*VNUM*sizeof(unsigned int)));
}
#endif

// Description
// Calculate the shortest path between each pair of vertexes in the complete graph using Disjkstra algorithm with Cilk_for
void calculate_shortest_path_cfor(void)
{
 // Main loop calculate the shortest path to all other vertexes from vertex "i" in each iteration
 // Declare loop control variable in "cilk_for" statement
 cilk_for (int i = 0;i < VNUM;++i) { 
    // Declare temporary arrays inside "cilk_for" loop body to make them private
    // Temporary array storing intermedia path length result to each vertex
    unsigned int vtemp[VNUM]; 
    // Flag array: 
    // "1" means the shortest path hasn't been finished
    // "0" menas the shortest path has been finished
    unsigned char vflag[VNUM];
    int j;
    // Initialize intermedia path length to INFINITE
    memset(vtemp,0xff,VNUM*sizeof(unsigned int));
    // Place the source vertex
    vtemp[i] = 0;
    pvertex_opt[i][i] = i;
    // Intialize flag array to all "1"
    memset(vflag,1,VNUM);
    // Calculate the "j+1"th shortest path from vertext "i"
    for (j = 0;j < VNUM;++j) {
        // minval: shortest path lengh in vtemp
        // minpos: index of the vertex having the shortest path length
        unsigned int minval, minpos;
        int k;

        // Initialze shortest lenght and vertex index to INFINITE
        minval = minpos = INFINITE;
        // Loop scan vtemp to find the index of vertex having the shortest path in vtemp and its length
        for (k = 0; k < VNUM;k++)
            if (vtemp[k] < minval) {
                minpos = k;
                minval = vtemp[k];
            } 

        // Store the shortest path length found to result matrix
        spath_opt[i][minpos] = minval; 
        // Update the length value of the vertex found to INFINITE so that it will be ignored in next round of MIN reduction
        vtemp[minpos] = INFINITE; 
        // Flag the path to the vertex found as finished
        vflag[minpos] = 0;        

        // Update unfinished vertexes path length value using edge length to the found vertex using loop scan
        for (k = 0; k < VNUM;k++)  
            if (vflag[k] && ((graph[minpos][k] + minval) < vtemp[k])) {
                vtemp[k] = (graph[minpos][k] + minval);
                pvertex_opt[i][k] = minpos;
        }
    }
 }
}

