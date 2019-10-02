/*
 * Main developer: Nico Van Cleemput
 * 
 * Copyright (C) 2019 Nico Van Cleemput.
 * Licensed under the GNU GPL, read the file LICENSE.txt for details.
 */

#include <stdlib.h>
#include <stdio.h>
#include <limits.h>

#include "planegraphs_automorphismgroup.h"

#define ABORT_IF_NULL(pointer) if(pointer==NULL){exit(-1);}

//////////////////////////////////////////////////////////////////////////////

typedef struct __pg_aut_comp_data PG_AUT_COMP_DATA;

struct __pg_aut_comp_data {
    int *certificate;
    int *canonical_labelling;
    int *reverse_canonical_labelling;
    PG_EDGE **canonical_first_edge;
    int *alternate_labelling;
    PG_EDGE **alternate_first_edge;
    
    PG_EDGE **orientation_preserving_starting_edges;
    PG_EDGE **orientation_reversing_starting_edges;
    int starting_edges_count; //the number of starting edges is always the same for both orientations
    
    int *queue;
    
    boolean has_chiral_group;
};

PG_AUT_COMP_DATA *get_automorphism_computation_workspace(PLANE_GRAPH *pg){
    int e, n;
    
    e = pg->ne;
    n = pg->nv;
    
    PG_AUT_COMP_DATA *workspace = malloc(sizeof(PG_AUT_COMP_DATA));
    ABORT_IF_NULL(workspace);
    
    workspace->certificate = malloc(sizeof(int)*(e+n));
    ABORT_IF_NULL(workspace->certificate);
    workspace->canonical_labelling = malloc(sizeof(int) * n);
    ABORT_IF_NULL(workspace->canonical_labelling);
    workspace->reverse_canonical_labelling = malloc(sizeof(int) * n);
    ABORT_IF_NULL(workspace->reverse_canonical_labelling);
    workspace->canonical_first_edge = malloc(sizeof(PG_EDGE*) * n);
    ABORT_IF_NULL(workspace->canonical_first_edge);
    workspace->alternate_labelling = malloc(sizeof(int) * n);
    ABORT_IF_NULL(workspace->alternate_labelling);
    workspace->alternate_first_edge = malloc(sizeof(PG_EDGE*) * n);
    ABORT_IF_NULL(workspace->alternate_first_edge);
    
    workspace->orientation_preserving_starting_edges = malloc(sizeof(PG_EDGE*) * e);
    ABORT_IF_NULL(workspace->orientation_preserving_starting_edges);
    workspace->orientation_reversing_starting_edges = malloc(sizeof(PG_EDGE*) * e);
    ABORT_IF_NULL(workspace->orientation_reversing_starting_edges);
    
    workspace->queue = malloc(sizeof(int)*n);
    ABORT_IF_NULL(workspace->queue);
    
    return workspace;
}

PG_AUTOMORPHISM_GROUP *allocate_automorphism_group(PLANE_GRAPH *pg){
    int i;
    
    PG_AUTOMORPHISM_GROUP *aut = (PG_AUTOMORPHISM_GROUP *)malloc(sizeof(PG_AUTOMORPHISM_GROUP));
    
    aut->automorphisms = (int **)malloc(sizeof(int *)*(pg->ne)*2);
    
    for(i = 0; i < (pg->ne)*2; i++){
        aut->automorphisms[i] = (int *)malloc(sizeof(int)*(pg->nv));
    }
    
    aut->allocated_size = (pg->ne) * 2;
    aut->graph = pg;
    
    aut->size = 0;
    aut->orientation_preserving_count = 0;
    aut->orientation_reversing_count = 0;
    
    return aut;
}

void trim_automorphism_group(PG_AUTOMORPHISM_GROUP *aut){
    int i;
    
    for(i = aut->size; i < aut->allocated_size; i++){
        free(aut->automorphisms[i]);
    }
    
    aut->automorphisms = (int **)realloc(aut->automorphisms, sizeof(int *)*(aut->size));
    aut->allocated_size = aut->size;
}

void free_automorphism_group(PG_AUTOMORPHISM_GROUP *aut){
    int i;
    
    for(i = 0; i < aut->allocated_size; i++){
        free(aut->automorphisms[i]);
    }
    
    free(aut->automorphisms);
    free(aut);
}

void find_starting_edges(PLANE_GRAPH *pg, PG_AUT_COMP_DATA *workspace){
    int i, starting_degree, starting_face_size, minimum_frequency;
    int vertex_degree_frequency[pg->nv];
    int face_size_frequency[pg->nv];
    PG_EDGE *start, *edge;
    
    for(i = 0; i < pg->nv; i++){
        vertex_degree_frequency[i] = face_size_frequency[i] = 0;
    }
    
    workspace->starting_edges_count = 0;
    
    //build the degree frequency table
    for(i = 0; i < pg->nv; i++){
        vertex_degree_frequency[pg->degree[i]]++;
    }
    
    //find the smallest degree with the lowest frequency
    minimum_frequency = pg->nv + 1;
    for(i = 0; i < pg->nv; i++){
        if(vertex_degree_frequency[i] && vertex_degree_frequency[i] < minimum_frequency){
            starting_degree = i;
            minimum_frequency = vertex_degree_frequency[i];
        }
    }
    
    //build the frequency table of face sizes incident to a vertex with starting_degree
    for(i = 0; i < pg->nv; i++){
        if(pg->degree[i] == starting_degree){
            start = edge = pg->first_edge[i];
            
            do {
                face_size_frequency[pg->face_size[edge->right_face]]++;
                edge = edge->next;
            } while (start != edge);
        }
    }
    
    //find the smallest face size incident with a vertex of starting_degree and with smallest frequency
    minimum_frequency = pg->ne + 1;
    for(i = 0; i < pg->nv; i++){
        if(face_size_frequency[i] && face_size_frequency[i] < minimum_frequency){
            starting_face_size = i;
            minimum_frequency = face_size_frequency[i];
        }
    }
    
    //store all starting edges
    for(i = 0; i < pg->nv; i++){
        if(pg->degree[i] == starting_degree){
            start = edge = pg->first_edge[i];
            
            do {
                if(pg->face_size[edge->right_face] == starting_face_size){
                    workspace->orientation_preserving_starting_edges[workspace->starting_edges_count] = edge;
                    workspace->orientation_reversing_starting_edges[workspace->starting_edges_count] = edge->next;
                    workspace->starting_edges_count++;
                }
                edge = edge->next;
            } while (start != edge);
        }
    }
}

void construct_certificate(PLANE_GRAPH *pg, PG_EDGE *e_start, PG_AUT_COMP_DATA *workspace){
    int i;
    for(i=0; i<pg->nv; i++){
        workspace->canonical_labelling[i] = INT_MAX;
    }
    PG_EDGE *e, *elast;
    int head = 1;
    int tail = 0;
    int vertex_counter = 1;
    int position = 0;
    workspace->queue[0] = e_start->start;
    workspace->canonical_first_edge[e_start->start] = e_start;
    workspace->canonical_labelling[e_start->start] = 0;
    while(head>tail){
        int current_vertex = workspace->queue[tail++];
        e = elast = workspace->canonical_first_edge[current_vertex];
        do {
            if(workspace->canonical_labelling[e->end] == INT_MAX){
                workspace->queue[head++] = e->end;
                workspace->canonical_labelling[e->end] = vertex_counter++;
                workspace->canonical_first_edge[e->end] = e->inverse;
            }
            workspace->certificate[position++] = workspace->canonical_labelling[e->end];
            e = e->next;
        } while (e!=elast);
        workspace->certificate[position++] = INT_MAX;
    }
    for(i = 0; i < pg->nv; i++){
        workspace->reverse_canonical_labelling[workspace->canonical_labelling[i]] = i;
    }
}

void construct_certificate_orientation_reversed(PLANE_GRAPH *pg, PG_EDGE *e_start, PG_AUT_COMP_DATA *workspace){
    int i;
    for(i=0; i<pg->nv; i++){
        workspace->canonical_labelling[i] = INT_MAX;
    }
    PG_EDGE *e, *elast;
    int head = 1;
    int tail = 0;
    int vertex_counter = 1;
    int position = 0;
    workspace->queue[0] = e_start->start;
    workspace->canonical_first_edge[e_start->start] = e_start;
    workspace->canonical_labelling[e_start->start] = 0;
    while(head>tail){
        int currentVertex = workspace->queue[tail++];
        e = elast = workspace->canonical_first_edge[currentVertex];
        do {
            if(workspace->canonical_labelling[e->end] == INT_MAX){
                workspace->queue[head++] = e->end;
                workspace->canonical_labelling[e->end] = vertex_counter++;
                workspace->canonical_first_edge[e->end] = e->inverse;
            }
            workspace->certificate[position++] = workspace->canonical_labelling[e->end];
            e = e->prev;
        } while (e!=elast);
        workspace->certificate[position++] = INT_MAX;
    }
    for(i = 0; i < pg->nv; i++){
       workspace->reverse_canonical_labelling[workspace->canonical_labelling[i]] = i;
    }
}

/* returns 1 if this edge leads to a better certificate
 * returns 0 if this edge leads to the same certificate
 * returns -1 if this edge leads to a worse certificate
 */
int has_better_certificate_orientation_preserving(PLANE_GRAPH *pg, PG_EDGE *e_start, PG_AUTOMORPHISM_GROUP *aut_group, PG_AUT_COMP_DATA *workspace){
    int i, j;
    for(i=0; i<pg->nv; i++){
        workspace->alternate_labelling[i] = INT_MAX;
    }
    PG_EDGE *e, *elast;
    int head = 1;
    int tail = 0;
    int vertex_counter = 1;
    int current_pos = 0;
    workspace->queue[0] = e_start->start;
    workspace->alternate_first_edge[e_start->start] = e_start;
    workspace->alternate_labelling[e_start->start] = 0;
    while(head>tail){
        int current_vertex = workspace->queue[tail++];
        e = elast = workspace->alternate_first_edge[current_vertex];
        do {
            if(workspace->alternate_labelling[e->end] == INT_MAX){
                workspace->queue[head++] = e->end;
                workspace->alternate_labelling[e->end] = vertex_counter++;
                workspace->alternate_first_edge[e->end] = e->inverse;
            }
            if(workspace->alternate_labelling[e->end] < workspace->certificate[current_pos]){
                construct_certificate(pg, e_start, workspace);
                aut_group->size = 1;
                aut_group->orientation_preserving_count = 1;
                return 1;
            } else if(workspace->alternate_labelling[e->end] > workspace->certificate[current_pos]){
                return -1;
            }
            current_pos++;
            e = e->next;
        } while (e!=elast);
        //INT_MAX will always be at least the value of certificate[current_pos]
        if(INT_MAX > workspace->certificate[current_pos]){
            return -1;
        }
        current_pos++;
    }
    for(j = 0; j < pg->nv; j++){
        aut_group->automorphisms[aut_group->size][j]
                = workspace->reverse_canonical_labelling[workspace->alternate_labelling[j]];
    }
    aut_group->size++;
    aut_group->orientation_preserving_count++;
    return 0;
}

int has_better_certificate_orientation_reversing(PLANE_GRAPH *pg, PG_EDGE *e_start, PG_AUTOMORPHISM_GROUP *aut_group, PG_AUT_COMP_DATA *workspace){
    int i, j;
    for(i=0; i<pg->nv; i++){
        workspace->alternate_labelling[i] = INT_MAX;
    }
    PG_EDGE *e, *elast;
    int head = 1;
    int tail = 0;
    int vertex_counter = 1;
    int current_pos = 0;
    workspace->queue[0] = e_start->start;
    workspace->alternate_first_edge[e_start->start] = e_start;
    workspace->alternate_labelling[e_start->start] = 0;
    while(head>tail){
        int current_vertex = workspace->queue[tail++];
        e = elast = workspace->alternate_first_edge[current_vertex];
        do {
            if(workspace->alternate_labelling[e->end] == INT_MAX){
                workspace->queue[head++] = e->end;
                workspace->alternate_labelling[e->end] = vertex_counter++;
                workspace->alternate_first_edge[e->end] = e->inverse;
            }
            if(workspace->alternate_labelling[e->end] < workspace->certificate[current_pos]){
                construct_certificate_orientation_reversed(pg, e_start, workspace);
                workspace->has_chiral_group = TRUE;
                aut_group->size = 1;
                aut_group->orientation_preserving_count = 1;
                aut_group->orientation_reversing_count = 0;
                return 1;
            } else if(workspace->alternate_labelling[e->end] > workspace->certificate[current_pos]){
                return -1;
            }
            current_pos++;
            e = e->prev;
        } while (e!=elast);
        //INT_MAX will always be at least the value of certificate[current_pos]
        if(INT_MAX > workspace->certificate[current_pos]){
            return -1;
        }
        current_pos++;
    }
    for(j = 0; j < pg->nv; j++){
        aut_group->automorphisms[aut_group->size][j]
                = workspace->reverse_canonical_labelling[workspace->alternate_labelling[j]];
    }
    if(workspace->has_chiral_group){
        aut_group->orientation_preserving_count++;
    } else {
        aut_group->orientation_reversing_count++;
    }
    aut_group->size++;
    return 0;
}

PG_AUTOMORPHISM_GROUP *determine_automorphisms(PLANE_GRAPH *pg){
    int i;
    
    PG_AUTOMORPHISM_GROUP *aut = allocate_automorphism_group(pg);
    
    //identity
    for(i = 0; i < pg->nv; i++){
        aut->automorphisms[0][i] = i;
    }
    
    aut->size = 1;
    aut->orientation_preserving_count = 1;
    aut->orientation_reversing_count = 0;
    
    PG_AUT_COMP_DATA *workspace = get_automorphism_computation_workspace(pg);
    workspace->has_chiral_group = FALSE;
    
    //find starting edges
    find_starting_edges(pg, workspace);
    
    //construct initial certificate
    construct_certificate(pg, workspace->orientation_preserving_starting_edges[0], workspace);
    
    //look for better automorphism
    for(i = 1; i < workspace->starting_edges_count; i++){
        int result = has_better_certificate_orientation_preserving(pg,
                                                                   workspace->orientation_preserving_starting_edges[i],
                                                                   aut, workspace);
        //if result == 1, then the counts are already reset and the new certificate is stored
        //if result == 0, then the automorphism is already stored
    }
    for(i = 0; i < workspace->starting_edges_count; i++){
        int result = has_better_certificate_orientation_reversing(pg,
                                                                  workspace->orientation_reversing_starting_edges[i],
                                                                  aut, workspace);
        //if result == 1, then the counts are already reset and the new certificate is stored
        //if result == 0, then the automorphism is already stored
    }
    
    return aut;
}