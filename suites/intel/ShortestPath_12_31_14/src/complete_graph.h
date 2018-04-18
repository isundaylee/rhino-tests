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

#ifndef __COMPLETE_GRAPH_H_
#define __COMPLETE_GRAPH_H_

// Vertex number in the graph
#define VNUM 1000
// Seed for for random generator
#define RSEED 1
// Max edge length value in the graph
#define EDGE_MAX 1000.0
// Min edge length value in the graph
#define EDGE_MIN 10.0
// Max unsigned integer value representing no edge between two vertexes
#define INFINITE ((unsigned int)(0xFFFFFFFF))
// Graph adjancency matrix dump file name
#define EDGE_FILE_NAME  "edges.txt"
// Shortest path dump file name
#define PATH_FILE_NAME  "path.txt"

// Create a complete graph having VNUM vertexes and randomly generated edge length
void init_graph(void);
// Dump the graph adjacency matrix to a file named EDGE_FILE_NAME for debugging purpose
void dump_graph_edge(void);
// Dump the shortest path result to a file named PATH_FILE_NAME for debugging purpose
void dump_shortest_path(void);
// Print out the shortest path from vertex "start" to vertex "end" to standard output
void print_shortest_path(unsigned int start, unsigned int end);
// Calculate shortest path between each pair of vertex in the graph using Dijkstra algorithm with serial code

// Calculate shortest path between each pair of vertex in the graph using Dijkstra algorithm with Cilk_for
void calculate_shortest_path_cfor(void);

#ifdef CHECK_RESULT
// Calculate shortest path between each pair of vertex in the graph using Dijkstra algorithm with no optimization
void calculate_shortest_path_base(void);
// Check result by comparing with the result of no optimization
unsigned char check_result(void);
#endif

#endif // __COMPLETE_GRAPH_H_
