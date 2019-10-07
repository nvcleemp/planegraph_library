/*
 * Main developer: Nico Van Cleemput
 *
 * Copyright (C) 2019 Nico Van Cleemput.
 * Licensed under the GNU GPL, read the file LICENSE.txt for details.
 */

#ifndef PLANEGRAPH_CONNECTIVITY_H
#define PLANEGRAPH_CONNECTIVITY_H

#include "planegraphs_base.h"

/**
 * Compute the edge connectivity of the given graph.
 * @param graph
 * @return the size of a smallest edge-cut
 */
int edge_connectivity(PLANE_GRAPH *graph);

/**
 * Compute the edge connectivity of the dual of the given graph.
 * @param graph
 * @return the size of a smallest edge-cut in the dual of the given graph
 */
int edge_connectivity_dual(PLANE_GRAPH *dual);

#endif //PLANEGRAPH_CONNECTIVITY_H
