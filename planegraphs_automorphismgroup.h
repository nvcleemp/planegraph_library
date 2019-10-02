/*
 * Main developer: Nico Van Cleemput
 *
 * Copyright (C) 2019 Nico Van Cleemput.
 * Licensed under the GNU GPL, read the file LICENSE.txt for details.
 */

#ifndef PLANEGRAPH_AUTOMORPHISMGROUP_H
#define	PLANEGRAPH_AUTOMORPHISMGROUP_H

#include "planegraphs_base.h"

typedef struct __pg_automorphism_group PG_AUTOMORPHISM_GROUP;

struct __pg_automorphism_group {
    int **automorphisms;
    int size;
    int orientation_preserving_count;
    int orientation_reversing_count;
    
    PLANE_GRAPH *graph;
    
    int allocated_size;
};

PG_AUTOMORPHISM_GROUP *determine_automorphisms(PLANE_GRAPH *pg);

void free_automorphism_group(PG_AUTOMORPHISM_GROUP *aut);


#endif	/* PLANEGRAPH_AUTOMORPHISMGROUP_H */

