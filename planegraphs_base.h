/*
 * Main developer: Nico Van Cleemput
 * 
 * Copyright (C) 2019 Nico Van Cleemput.
 * Licensed under the GNU GPL, read the file LICENSE.txt for details.
 */

#ifndef PLANEGRAPH_BASE_H
#define	PLANEGRAPH_BASE_H

#define FALSE 0
#define TRUE  1

typedef int boolean;
typedef struct __pg_edge PG_EDGE;
typedef struct __plane_graph PLANE_GRAPH;

/* The data type used for edges */ 
struct __pg_edge {
    //the start and end vertex of this edge
    int start;
    int end;
    
    //the face on the right side of the edge
    //NOTE: only valid if construct_faces called on the graph
    int right_face;
    
    PG_EDGE *prev; /* previous edge in clockwise direction */
    PG_EDGE *next; /* next edge in clockwise direction */
    PG_EDGE *inverse; /* the edge that is inverse to this one */
    
     /* two ints for temporary use;
      * Only access mark via the MARK macros.
      */
    int mark, index;
    
    //a label can be used to store any additional information you want to
    //associate with an edge
    void *label;
};

struct __plane_graph {
    int nv;
    int ne;
    int nf;
    
    int maxn;
    int maxe;
    int maxf;
    
    //an array containing all edges of this graph
    PG_EDGE *edges;
    
    //an array containing for each vertex a pointer to an edge leaving that vertex
    PG_EDGE **first_edge;
    
    //an array containing the degree of each vertex
    int *degree;
    
    //an array containing for each face a pointer to an edge in the clockwise 
    //boundary of that face
    PG_EDGE **face_start;
    
    //an array containing the size of each face
    int *face_size;
    
    //TRUE if the faces of this graph have been constructed
    boolean faces_constructed;
    
    //should be set to 30000 for a new graph
    int mark_value;
};

#define RESETMARKS(pg) {int mki; if (((pg)->mark_value += 2) > 30000) \
       { (pg)->mark_value = 2; for (mki=0;mki<(pg)->maxe;++mki) (pg)->edges[mki].mark=0;}}
#define MARK(pg, e) (e)->mark = (pg)->markvalue
#define MARKLO(pg, e) (e)->mark = (pg)->mark_value
#define MARKHI(pg, e) (e)->mark = (pg)->markvalue+1
#define UNMARK(pg, e) (e)->mark = (pg)->markvalue-1
#define ISMARKED(pg, e) ((e)->mark >= (pg)->markvalue)
#define ISMARKEDLO(pg, e) ((e)->mark == (pg)->mark_value)
#define ISMARKEDHI(pg, e) ((e)->mark > (pg)->markvalue)

/**
 * Creates a new planar graph data structure that can hold plane graphs with
 * up to maxn vertices and maxe oriented edges. If maxe is zero, then the 
 * theoretical maximum for maxn is computed and used.
 * 
 * This function might return a NULL pointer if insufficient memory was
 * available or the number of vertices is illegal.
 *
 * @param maxn the maximum number of vertices this plane graph will contain
 * @param maxe the maximum number of oriented edges this plane graph will contain,
 *             or zero if the theoretical maximum should be used
 */
PLANE_GRAPH *new_plane_graph(int maxn, int maxe);

void free_plane_graph(PLANE_GRAPH *pg);

/**
 *
 * @param pg
 */
void construct_faces(PLANE_GRAPH *pg);

/**
 * Check whether two vertices are adjacent
 * @param pg
 * @param v1
 * @param v2
 * @return TRUE if there is an edge from v1 to v2, and FALSE otherwise
 */
boolean are_adjacent(PLANE_GRAPH *pg, int v1, int v2);

PG_EDGE *find_edge(PLANE_GRAPH *pg, int from, int to);

void clear_all_edge_labels(PLANE_GRAPH *pg);

PLANE_GRAPH *get_dual_graph(PLANE_GRAPH *pg);

/**
 * Return the minimum degree of the graph.
 * @param pg
 * @return the smallest degree of a vertex in this graph.
 */
int minimum_degree(PLANE_GRAPH *pg);

/**
 * Return the maximum degree of the graph.
 * @param pg
 * @return the largest degree of a vertex in this graph.
 */
int maximum_degree(PLANE_GRAPH *pg);

#endif	/* PLANEGRAPH_BASE_H */

