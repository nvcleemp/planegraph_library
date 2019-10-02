/*
 * Main developer: Nico Van Cleemput
 * 
 * Copyright (C) 2019 Nico Van Cleemput.
 * Licensed under the GNU GPL, read the file LICENSE.txt for details.
 */

#ifndef PLANEGRAPH_OUTPUT_H
#define	PLANEGRAPH_OUTPUT_H

#include "planegraphs_base.h"
#include <stdio.h>

#ifdef	__cplusplus
extern "C" {
#endif

void write_planar_code(PLANE_GRAPH *pg, FILE *f);

void write_edge_code(PLANE_GRAPH *pg, FILE *f);

#ifdef	__cplusplus
}
#endif

#endif	/* PLANEGRAPH_OUTPUT_H */

