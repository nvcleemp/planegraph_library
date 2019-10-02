/*
 * Main developer: Nico Van Cleemput
 * 
 * Copyright (C) 2019 Nico Van Cleemput.
 * Licensed under the GNU GPL, read the file LICENSE.txt for details.
 */

#ifndef PLANEGRAPH_INPUT_H
#define	PLANEGRAPH_INPUT_H

#include "planegraphs_base.h"
#include<stdio.h>

#ifdef	__cplusplus
extern "C" {
#endif
    
    typedef struct __plane_graph_input_options PG_INPUT_OPTIONS;
    
    struct __plane_graph_input_options {
        boolean contains_header;
        boolean remove_internal_headers;
        
        int initial_code_length;
        
        int maxn;
        int maxnFactor;
        int maxe;
        
        boolean construct_faces;
    };
    
//the default code length is sufficient to store any graph with less than 100 vertices
#define DEFAULT_PG_INPUT_OPTIONS(options) PG_INPUT_OPTIONS options = {TRUE, TRUE, 700, 0, 1, 0, FALSE}

    PLANE_GRAPH *decode_planar_code(unsigned short* code, PG_INPUT_OPTIONS *options);

    unsigned short *read_planar_code(FILE *file, PG_INPUT_OPTIONS *options);
    
    PLANE_GRAPH *read_and_decode_planar_code(FILE *f, PG_INPUT_OPTIONS *options);

#ifdef	__cplusplus
}
#endif

#endif	/* PLANEGRAPH_INPUT_H */

