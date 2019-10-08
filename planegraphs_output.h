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

/**
 * Output the planar code of the graph
 * @param pg
 * @param f
 * @param print_header Include the header for this code
 */
void write_planar_code(PLANE_GRAPH *pg, FILE *f, boolean print_header);

/**
 * Output the planar code of the dual of the graph
 * @param pg
 * @param f
 * @param print_header Include the header for this code
 */
void write_dual_planar_code(PLANE_GRAPH *pg, FILE *f, boolean print_header);

/**
 * Output the edge code of the graph
 * @param pg
 * @param f
 * @param print_header Include the header for this code
 */
void write_edge_code(PLANE_GRAPH *pg, FILE *f, boolean print_header);

/**
 * Output the edge code of the dual of the graph
 * @param pg
 * @param f
 * @param print_header Include the header for this code
 */
void write_dual_edge_code(PLANE_GRAPH *pg, FILE *f, boolean print_header);

#ifdef	__cplusplus
}
#endif

#endif	/* PLANEGRAPH_OUTPUT_H */

