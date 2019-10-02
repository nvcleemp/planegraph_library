/*
 * Main developer: Nico Van Cleemput
 * 
 * Copyright (C) 2013 Nico Van Cleemput.
 * Licensed under the GNU GPL, read the file LICENSE.txt for details.
 */

#include "planegraphs_input.h"
#include <stdlib.h>
#include <string.h>

PLANE_GRAPH *decode_planar_code(unsigned short* code, PG_INPUT_OPTIONS *options) {
    int i, j, code_position, nv, maxn;
    int edge_counter = 0;
    PG_EDGE *inverse;

    nv = code[0];
    code_position = 1;
    
    if(options->maxn <= 0){
        maxn = nv*options->maxnFactor;
    } else {
        maxn = options->maxn;
    }
    
    PLANE_GRAPH *pg = new_plane_graph(maxn, options->maxe);
    pg->nv = nv;

    for (i = 0; i < nv; i++) {
        pg->degree[i] = 0;
        pg->first_edge[i] = pg->edges + edge_counter;
        pg->edges[edge_counter].start = i;
        pg->edges[edge_counter].end = code[code_position] - 1;
        pg->edges[edge_counter].next = pg->edges + edge_counter + 1;
        if (code[code_position] - 1 < i) {
            inverse = find_edge(pg, code[code_position] - 1, i);
            pg->edges[edge_counter].inverse = inverse;
            inverse->inverse = pg->edges + edge_counter;
        } else {
            pg->edges[edge_counter].inverse = NULL;
        }
        edge_counter++;
        code_position++;
        for (j = 1; code[code_position]; j++, code_position++) {
            pg->edges[edge_counter].start = i;
            pg->edges[edge_counter].end = code[code_position] - 1;
            pg->edges[edge_counter].prev = pg->edges + edge_counter - 1;
            pg->edges[edge_counter].next = pg->edges + edge_counter + 1;
            if (code[code_position] - 1 < i) {
                inverse = find_edge(pg, code[code_position] - 1, i);
                pg->edges[edge_counter].inverse = inverse;
                inverse->inverse = pg->edges + edge_counter;
            } else {
                pg->edges[edge_counter].inverse = NULL;
            }
            edge_counter++;
        }
        pg->first_edge[i]->prev = pg->edges + edge_counter - 1;
        pg->edges[edge_counter - 1].next = pg->first_edge[i];
        pg->degree[i] = j;

        code_position++; /* read the closing 0 */
    }

    pg->ne = edge_counter;

    if(options->construct_faces){
        construct_faces(pg);
    }
    
    return pg;
}

/**
 *
 * @param file
 * @param options
 * @return
 */
unsigned short *read_planar_code(FILE *file, PG_INPUT_OPTIONS *options) {
    static boolean first = TRUE;
    unsigned char c;
    char testheader[20];
    int buffer_size, zero_counter;
    
    int read_count;
    
    int code_length = options->initial_code_length;
    unsigned short* code = malloc(code_length * sizeof(unsigned short));
    if(code == NULL){
        fprintf(stderr, "Insufficient memory to store code for this graph.\n");
        return NULL;
    }

    if (first) {
        first = FALSE;
        
        if(options->contains_header){
            //we check that there is a header
            if (fread(&testheader, sizeof (unsigned char), 13, file) != 13) {
                fprintf(stderr, "can't read header: file too small.\n");
                free(code);
                return NULL;
            }
            testheader[13] = 0;
            if (strcmp(testheader, ">>planar_code") != 0) {
                fprintf(stderr, "No planarcode header detected.\n");
                free(code);
                return NULL;
            }

            //read reminder of header (either empty or le/be specification)
            if (fread(&c, sizeof (unsigned char), 1, file) == 0) {
                fprintf(stderr, "Invalid formatted header.\n");
                free(code);
                return NULL;
            }
            while (c!='<'){
                if (fread(&c, sizeof (unsigned char), 1, file) == 0) {
                    fprintf(stderr, "Invalid formatted header.\n");
                    free(code);
                    return NULL;
                }
            }
            //read one more character (header is closed by <<)
            if (fread(&c, sizeof (unsigned char), 1, file) == 0) {
                fprintf(stderr, "Invalid formatted header.\n");
                free(code);
                return NULL;
            }
        }
    }

    /* possibly removing interior headers */
    if(options->remove_internal_headers){
        if (fread(&c, sizeof (unsigned char), 1, file) == 0) {
            //nothing left in file
            return NULL;
        }

        if (c == '>') {
            // could be a header, or maybe just a 62 (which is also possible for unsigned char
            code[0] = c;
            buffer_size = 1;
            zero_counter = 0;
            code[1] = (unsigned short) getc(file);
            if (code[1] == 0) zero_counter++;
            code[2] = (unsigned short) getc(file);
            if (code[2] == 0) zero_counter++;
            buffer_size = 3;
            // 3 characters were read and stored in buffer
            if ((code[1] == '>') && (code[2] == 'p')) /*we are sure that we're dealing with a header*/ {
                while ((c = getc(file)) != '<');
                /* read 2 more characters: */
                c = getc(file);
                if (c != '<') {
                    fprintf(stderr, "Problems with header -- single '<'\n");
                    free(code);
                    return NULL;
                }
                if (!fread(&c, sizeof (unsigned char), 1, file)) {
                    //nothing left in file
                    code[0] = 0;
                    return code;
                }
                buffer_size = 1;
                zero_counter = 0;
            }
        } else {
            //no header present
            buffer_size = 1;
            zero_counter = 0;
        }
    }

    //start reading the graph
    if (c != 0) {
        code[0] = c;
        while (zero_counter < code[0]) {
            if(buffer_size == code_length){
                code_length = (2 * code_length < (7 * code[0] - 9)) ? 2 * code_length : (7 * code[0] - 9);
                // 7*code[0]-9 is the maximum code length for this number of vertices 
                unsigned short* new_code = realloc(code, code_length * sizeof(unsigned short));
                if(new_code == NULL){
                    free(code);
                    fprintf(stderr, "Insufficient memory to store code for this graph.\n");
                    return NULL;
                } else {
                    code = new_code;
                }
            }
            code[buffer_size] = (unsigned short) getc(file);
            if (code[buffer_size] == 0) zero_counter++;
            buffer_size++;
        }
    } else {
        read_count = fread(code, sizeof (unsigned short), 1, file);
        if(!read_count){
            fprintf(stderr, "Unexpected EOF.\n");
            free(code);
            return NULL;
        }
        buffer_size = 1;
        zero_counter = 0;
        while (zero_counter < code[0]) {
            if(buffer_size == code_length){
                code_length = (2 * code_length < (7 * code[0] - 9)) ? 2 * code_length : (7 * code[0] - 9);
                // 7*code[0]-9 is the maximum code length for this number of vertices 
                unsigned short* new_code = realloc(code, code_length * sizeof(unsigned short));
                if(new_code == NULL){
                    free(code);
                    fprintf(stderr, "Insufficient memory to store code for this graph.\n");
                    return NULL;
                } else {
                    code = new_code;
                }
            }
            read_count = fread(code + buffer_size, sizeof (unsigned short), 1, file);
            if(!read_count){
                fprintf(stderr, "Unexpected EOF.\n");
                free(code);
                return NULL;
            }
            if (code[buffer_size] == 0) zero_counter++;
            buffer_size++;
        }
    }

    return code;
}

PLANE_GRAPH *read_and_decode_planar_code(FILE *f, PG_INPUT_OPTIONS *options){
    unsigned short *code = read_planar_code(f, options);
    if(code==NULL){
        return NULL;
    }
    PLANE_GRAPH *pg = decode_planar_code(code, options);
    free(code);
    return pg;
}
