/*
 * Main developer: Nico Van Cleemput
 * 
 * Copyright (C) 2019 Nico Van Cleemput.
 * Licensed under the GNU GPL, read the file LICENSE.txt for details.
 */

#include "planegraphs_base.h"
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>

PLANE_GRAPH *new_plane_graph(int maxn, int maxe) {
    if(maxn <= 0){
        fprintf(stderr, "maxn should be a positive integer -- exiting!\n");
        return NULL;
    }
    
    if(maxe <= 0 || maxe > (6*maxn-12)){
        maxe = 6*maxn-12;
    }
    
    PLANE_GRAPH *pg = (PLANE_GRAPH *)malloc(sizeof(PLANE_GRAPH));
    
    if(pg == NULL){
        fprintf(stderr, "Insufficient memory for plane_graph -- exiting!\n");
        return NULL;
    }
    
    pg->maxn = maxn;
    pg->maxe = maxe;
    
    pg->nv = pg->ne = pg->nf = 0;
    
    pg->edges = (PG_EDGE *)malloc(sizeof(PG_EDGE)*maxe);
    
    if(pg->edges == NULL){
        fprintf(stderr, "Insufficient memory for edges -- exiting!\n");
        free(pg);
        return NULL;
    }
    
    pg->first_edge = (PG_EDGE **)malloc(sizeof(PG_EDGE *) * maxn);
    
    if(pg->first_edge == NULL){
        fprintf(stderr, "Insufficient memory for firstedges -- exiting!\n");
        free(pg->edges);
        free(pg);
        return NULL;
    }
    
    pg->degree = (int *)malloc(sizeof(int)*maxn);
    
    if(pg->degree == NULL){
        fprintf(stderr, "Insufficient memory for degrees -- exiting!\n");
        free(pg->degree);
        free(pg->edges);
        free(pg);
        return NULL;
    }
    
    //by default the dual is not yet initialised
    pg->maxf = 0;
    pg->face_start = NULL;
    pg->face_size = NULL;
    pg->faces_constructed = FALSE;
    pg->mark_value = 30000;
    
    return pg;
}

void free_plane_graph(PLANE_GRAPH *pg){
    if(pg->face_start != NULL){
        free(pg->face_start);
    }
    
    if(pg->face_size != NULL){
        free(pg->face_size);
    }
    
    free(pg->degree);
    free(pg->first_edge);
    free(pg->edges);
    free(pg);
}

/**
 * Check whether two vertices are adjacent
 * @param pg
 * @param v1
 * @param v2
 * @return TRUE if there is an edge from v1 to v2, and FALSE otherwise
 */
boolean are_adjacent(PLANE_GRAPH *pg, int v1, int v2) {
    PG_EDGE *e, *elast;

    e = elast = pg->first_edge[v1];
    do {
        if (e->end == v2) {
            return TRUE;
        }
        e = e->next;
    } while (e != elast);
    return FALSE;
}

PG_EDGE *find_edge(PLANE_GRAPH *pg, int from, int to) {
    PG_EDGE *e, *elast;

    e = elast = pg->first_edge[from];
    do {
        if (e->end == to) {
            return e;
        }
        e = e->next;
    } while (e != elast);
    fprintf(stderr, "error while looking for edge from %d to %d.\n", from, to);
    exit(EXIT_FAILURE);
}

/* Store in the right_face field of each edge the number of the face on
   the right hand side of that edge.  Faces are numbered 0,1,....  Also
   store in face_start[i] an example of an edge in the clockwise orientation
   of the face boundary, and the size of the face in facesize[i], for each i.
   Returns the number of faces. */
void construct_faces(PLANE_GRAPH *pg) {
    register int i, sz;
    register PG_EDGE *e, *ex, *ef, *efx;

    RESETMARKS(pg);
    
    //first allocate the memory to store faces if this has not yet been done
    int maxf = 2*pg->maxn - 4;
    if(pg->maxf < maxf){
        if(pg->face_start != NULL){
            free(pg->face_start);
        }
        if(pg->face_size != NULL){
            free(pg->face_size);
        }
        pg->maxf = maxf;
        
        pg->face_start = (PG_EDGE **)malloc(sizeof(PG_EDGE *) * maxf);
        
        if(pg->face_start == NULL){
            pg->maxf = 0;
            return;
        }
        
        pg->face_size = (int *)malloc(sizeof(int) * maxf);
        
        if(pg->face_size == NULL){
            pg->maxf = 0;
            free(pg->face_start);
            return;
        }
    }

    int nf = 0;
    for (i = 0; i < pg->nv; ++i) {

        e = ex = pg->first_edge[i];
        do {
            if (!ISMARKEDLO(pg, e)) {
                pg->face_start[nf] = ef = efx = e;
                sz = 0;
                do {
                    ef->right_face = nf;
                    MARKLO(pg, ef);
                    ef = ef->inverse->prev;
                    ++sz;
                } while (ef != efx);
                pg->face_size[nf] = sz;
                ++nf;
            }
            e = e->next;
        } while (e != ex);
    }
    pg->nf = nf;
    pg->faces_constructed = TRUE;
}

void clear_all_edge_labels(PLANE_GRAPH *pg){
    int i;
    
    for(i = 0; i < pg->maxe; i++){
        pg->edges[i].label = NULL;
    }
}

PLANE_GRAPH *get_dual_graph(PLANE_GRAPH *pg){
    int i;
    
    if(!pg->faces_constructed){
        construct_faces(pg);
    }
    
    PLANE_GRAPH *dual = new_plane_graph(pg->nf, pg->ne);
    
    if(dual==NULL){
        fprintf(stderr, "Insufficient memory to create dual.\n");
        return NULL;
    }
    
    for(i = 0; i < pg->ne; i++){
        pg->edges[i].index = i;
    }
    
    dual->nv = pg->nf;
    dual->ne = pg->ne;
    dual->nf = pg->nv;
    
    for(i = 0; i < pg->ne; i++){
        dual->edges[i].start = pg->edges[i].right_face;
        dual->edges[i].end = pg->edges[i].inverse->right_face;
        dual->edges[i].right_face = pg->edges[i].end;
        
        dual->edges[i].inverse = dual->edges + pg->edges[i].inverse->index;
        dual->edges[i].next = dual->edges + pg->edges[i].inverse->prev->index;
        dual->edges[i].prev = dual->edges + pg->edges[i].next->inverse->index;
    }
    
    for(i = 0; i < pg->nf; i++){
        dual->degree[i] = pg->face_size[i];
        dual->first_edge[i] = dual->edges + pg->face_start[i]->index;
    }
    
    dual->maxf = pg->nv;
    
    dual->face_start = (PG_EDGE **)malloc(sizeof(PG_EDGE *) * (dual->maxf));
        
    if(dual->face_start == NULL){
        dual->maxf = 0;
        return dual;
    }

    dual->face_size = (int *)malloc(sizeof(int) * (dual->maxf));

    if(dual->face_size == NULL){
        dual->maxf = 0;
        free(dual->face_start);
        return dual;
    }
    
    for(i = 0; i < pg->nv; i++){
        dual->face_size[i] = pg->degree[i];
        dual->face_start[i] = dual->edges + pg->first_edge[i]->inverse->index;
    }
    
    dual->faces_constructed = TRUE;
    
    return dual;
}

/**
 * Return the minimum degree of the graph.
 * @param pg
 * @return the smallest degree of a vertex in this graph.
 */
int minimum_degree(PLANE_GRAPH *pg){
    if(pg->nv == 0) return 0;
    int minimum = pg->degree[0];
    for (int i = 1; i < pg->nv; ++i) {
        if(pg->degree[i] < minimum){
            minimum = pg->degree[i];
        }
    }
    return minimum;
}

/**
 * Return the maximum degree of the graph.
 * @param pg
 * @return the largest degree of a vertex in this graph.
 */
int maximum_degree(PLANE_GRAPH *pg){
    if(pg->nv == 0) return 0;
    int maximum = pg->degree[0];
    for (int i = 1; i < pg->nv; ++i) {
        if(pg->degree[i] > maximum){
            maximum = pg->degree[i];
        }
    }
    return maximum;
}