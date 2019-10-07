/*
 * Main developer: Nico Van Cleemput
 *
 * Copyright (C) 2019 Nico Van Cleemput.
 * Licensed under the GNU GPL, read the file LICENSE.txt for details.
 */

#include "planegraphs_base.h"
#include "planegraphs_cycles.h"

/**
 * Uses BFS to find the shortest cycle through the specified vertex.
 * @param graph
 * @param v
 * @param current_best An upper bound for the return value
 * @return the minimum of current_best and the length of the shortest cycle through the given vertex.
 */
int improve_shortest_cycle(PLANE_GRAPH *graph, int v, int current_best) {
    //store the number of vertices just to have more readable code below
    int n = graph->nv;

    int queue[n];
    int head = 0;
    int tail = 0;

    int distance_to_v[n];
    for(int i=0; i<n; i++){
        distance_to_v[i]=-1;
    }
    queue[head] = v;

    int lim = current_best / 2;

    head ++;
    distance_to_v[v] = 0;
    while (head > tail) {
        int vertex = queue[tail];
        int current_distance = distance_to_v[vertex];
        if (current_distance >= lim)
            return current_best; // can never improve current best
        tail ++;
        PG_EDGE *e, *e_last;
        e = e_last = graph->first_edge[vertex];
        do {
            int neighbour = e->end;
            int neighbour_distance = distance_to_v[neighbour];
            if (neighbour_distance < 0) {
                // not yet encountered: set distance and add to queue
                distance_to_v[neighbour] = current_distance + 1;
                queue[head] = neighbour;
                head ++;
            } else if (neighbour_distance == current_distance) {
                // odd cycle
                return 2 * current_distance + 1;
            } else if (neighbour_distance > current_distance) {
                // even cycle
                if (current_best > 2 * neighbour_distance) {
                    current_best = 2 * neighbour_distance; // == 2*current_distance+2
                }
            } //else {
            // returns towards v, ignore
            //}
            e = e->next;
        } while (e != e_last);
    }
    return current_best;
}

/**
 * Compute the length of a shortest cycle in the graph if it is shorter than maximum.
 * @param graph
 * @param maximum An upper bound for the shortest cycle
 * @return the length of a shortest cycle or the number of vertices plus one if the graph is acyclic
 */
int shorter_cycle(PLANE_GRAPH *graph, int maximum){
    //initially we set shortest_cycle to the size of the smallest face, if faces have been constructed
    int shortest_cycle = maximum;
    if(graph->nv + 1 < shortest_cycle){
        shortest_cycle = graph->nv + 1;
    }
    if(graph->faces_constructed){
        for (int i = 0; i < graph->nf; ++i) {
            if(graph->face_size[i] < shortest_cycle){
                shortest_cycle = graph->face_size[i];
            }
        }
    }

    //start BFS in each vertex to find a shorter cycle
    for(int i=0; i<graph->nv - 2; i++){
        shortest_cycle = improve_shortest_cycle(graph, i, shortest_cycle);
    }

    return shortest_cycle;
}

/**
 * Compute the length of a shortest cycle in the graph
 * @param graph
 * @return the length of a shortest cycle or the number of vertices plus one if the graph is acyclic
 */
int shortest_cycle(PLANE_GRAPH *graph){
    return shorter_cycle(graph, graph->nv + 1);
}