/*
 * Main developer: Nico Van Cleemput
 *
 * Copyright (C) 2019 Nico Van Cleemput.
 * Licensed under the GNU GPL, read the file LICENSE.txt for details.
 */

#ifndef PLANEGRAPH_CYCLES_H
#define PLANEGRAPH_CYCLES_H

#include "planegraphs_base.h"

/**
 * Compute the length of a shortest cycle in the graph if it is shorter than maximum.
 * @param graph
 * @param maximum An upper bound for the shortest cycle
 * @return the length of a shortest cycle or the number of vertices plus one if the graph is acyclic
 */
int shorter_cycle(PLANE_GRAPH *graph, int maximum);

/**
 * Compute the length of a shortest cycle in the graph
 * @param graph
 * @return the length of a shortest cycle or the number of vertices plus one if the graph is acyclic
 */
int shortest_cycle(PLANE_GRAPH *graph);

#endif //PLANEGRAPH_CYCLES_H
