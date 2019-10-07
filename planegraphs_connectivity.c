/*
 * Main developer: Nico Van Cleemput
 *
 * Copyright (C) 2019 Nico Van Cleemput.
 * Licensed under the GNU GPL, read the file LICENSE.txt for details.
 */

#include "planegraphs_base.h"
#include "planegraphs_cycles.h"
#include "planegraphs_connectivity.h"

/**
 * Compute the edge connectivity of the given graph.
 * @param graph
 * @return the size of a smallest edge-cut
 */
int edge_connectivity(PLANE_GRAPH *graph){
    PLANE_GRAPH *dual = get_dual_graph(graph);
    int edge_connectivity = shortest_cycle(dual);
    free_plane_graph(dual);
    return edge_connectivity;
}

/**
 * Compute the edge connectivity of the dual of the given graph.
 * @param graph
 * @return the size of a smallest edge-cut in the dual of the given graph
 */
int edge_connectivity_dual(PLANE_GRAPH *dual){
    return shortest_cycle(dual);
}