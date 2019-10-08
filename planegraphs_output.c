/*
 * Main developer: Nico Van Cleemput
 * 
 * Copyright (C) 2019 Nico Van Cleemput.
 * Licensed under the GNU GPL, read the file LICENSE.txt for details.
 */

#include <stdlib.h>
#include "planegraphs_output.h"

//----------------PLANAR CODE--------------------

void write_planar_code_char(PLANE_GRAPH *pg, FILE *f){
    int i;
    PG_EDGE *e, *elast;
    
    //write the number of vertices
    fputc(pg->nv, f);
    
    for(i=0; i<pg->nv; i++){
        e = elast = pg->first_edge[i];
        do {
            fputc(e->end + 1, f);
            e = e->next;
        } while (e != elast);
        fputc(0, f);
    }
}

void write_planar_code_short(PLANE_GRAPH *pg, FILE *f){
    int i;
    PG_EDGE *e, *elast;
    unsigned short temp;
    
    //write the number of vertices
    fputc(0, f);
    temp = pg->nv;
    if (fwrite(&temp, sizeof (unsigned short), 1, f) != 1) {
        fprintf(stderr, "fwrite() failed -- exiting!\n");
        exit(EXIT_FAILURE);
    }
    
    for(i=0; i<pg->nv; i++){
        e = elast = pg->first_edge[i];
        do {
            temp = e->end + 1;
            if (fwrite(&temp, sizeof (unsigned short), 1, f) != 1) {
                fprintf(stderr, "fwrite() failed -- exiting!\n");
                exit(EXIT_FAILURE);
            }
            e = e->next;
        } while (e != elast);
        temp = 0;
        if (fwrite(&temp, sizeof (unsigned short), 1, f) != 1) {
            fprintf(stderr, "fwrite() failed -- exiting!\n");
            exit(EXIT_FAILURE);
        }
    }
}

/**
 * Output the planar code of the graph
 * @param pg
 * @param f
 * @param print_header Include the header for this code
 */
void write_planar_code(PLANE_GRAPH *pg, FILE *f, boolean print_header){
    if(print_header){
        fprintf(f, ">>planar_code<<");
    }
    
    if (pg->nv + 1 <= 255) {
        write_planar_code_char(pg, f);
    } else if (pg->nv + 1 <= 65535) {
        write_planar_code_short(pg, f);
    } else {
        fprintf(stderr, "Graphs of that size are currently not supported -- exiting!\n");
        exit(EXIT_FAILURE);
    }
    
}

void write_dual_planar_code_char(PLANE_GRAPH *pg, FILE *f){
    PG_EDGE *e, *elast;

    //write the number of vertices of the dual
    fputc(pg->nf, f);

    for(int i=0; i<pg->nf; i++){
        e = elast = pg->face_start[i];
        do {
            fputc(e->inverse->right_face + 1, f);
            e = e->inverse->prev;
        } while (e != elast);
        fputc(0, f);
    }
}

void write_dual_planar_code_short(PLANE_GRAPH *pg, FILE *f){
    PG_EDGE *e, *elast;
    unsigned short temp;

    //write the number of vertices of the dual
    fputc(0, f);
    temp = pg->nf;
    if (fwrite(&temp, sizeof (unsigned short), 1, f) != 1) {
        fprintf(stderr, "fwrite() failed -- exiting!\n");
        exit(-1);
    }

    for(int i=0; i<pg->nf; i++){
        e = elast = pg->face_start[i];
        do {
            temp = e->inverse->right_face + 1;
            if (fwrite(&temp, sizeof (unsigned short), 1, f) != 1) {
                fprintf(stderr, "fwrite() failed -- exiting!\n");
                exit(-1);
            }
            e = e->inverse->prev;
        } while (e != elast);
        temp = 0;
        if (fwrite(&temp, sizeof (unsigned short), 1, f) != 1) {
            fprintf(stderr, "fwrite() failed -- exiting!\n");
            exit(-1);
        }
    }
}

/**
 * Output the planar code of the dual of the graph
 * @param pg
 * @param f
 * @param print_header Include the header for this code
 */
void write_dual_planar_code(PLANE_GRAPH *pg, FILE *f, boolean print_header){
    if(print_header){
        fprintf(f, ">>planar_code<<");
    }

    if(!pg->faces_constructed) construct_faces(pg);

    if (pg->nf + 1 <= 255) {
        write_dual_planar_code_char(pg, f);
    } else if (pg->nf + 1 <= 65535) {
        write_dual_planar_code_short(pg, f);
    } else {
        fprintf(stderr, "Graphs of that size are currently not supported -- exiting!\n");
        exit(-1);
    }
}

//----------------EDGE CODE--------------------

void write_edge_code_small(PLANE_GRAPH *pg, FILE *f){
    int i;
    PG_EDGE *e, *elast;
    
    //write the length of the body
    fputc(pg->ne + pg->nv - 1, f);
    
    for(i=0; i<pg->nv; i++){
        e = elast = pg->first_edge[i];
        do {
            fputc(e->index, f);
            e = e->next;
        } while (e != elast);
        if(i < pg->nv - 1){
            fputc(255, f);
        }
    }
}

void write_big_endian_multibyte(FILE *f, int number, int bytecount){
    if(bytecount==1){
        fprintf(f, "%c", number);
    } else {
        int i;
        unsigned int mask = 256 << ((bytecount - 2)*8);
        fprintf(f, "%c", number/mask);
        for(i = 0; i < bytecount - 1; i++){
            number %= mask;
            mask >>= 8;
            fprintf(f, "%c", number/mask);
        }
    }    
}

void write_edge_code_large(PLANE_GRAPH *pg, FILE *f){
    int i;
    PG_EDGE *e, *elast;
    
    fprintf(stderr, "Graphs of that size are currently not supported -- exiting!\n");
    exit(-1);
    
    int k, l, s;
    
    //determine the maximum number of bytes needed to store each of the edges
    l = 1;
    while(pg->ne > (1 << l)){
        l++;
    }
    if(l%8){
        l = l/8 + 1;
    } else {
        l /= 8;
    }
    //check that the largest number does not start with the byte 255.
    if (pg->ne/(256 << ((l-1)*8)) == 255){
        l++;
    }
    if(l>15){
        fprintf(stderr, "This graph can not be stored in edge code format -- exiting!\n");
    }
    
    //s is the number of bytes needed to store the body
    s = (pg->ne + pg->nv - 1)*l;
    
    //k is the number of bytes needed to encode s
    k = 1;
    while(s > (1 << k)){
        k++;
    }
    if(k%8){
        k = k/8 + 1;
    } else {
        k /= 8;
    }
    
    //we start the code with a zero
    fputc(0, f);
    
    //write the byte encoding k and l
    fputc((k<<4)+l, f);
    
    //write the length of the body (s)
    write_big_endian_multibyte(f, s, k);
    
    
    for(i=0; i<pg->nv; i++){
        e = elast = pg->first_edge[i];
        do {
            write_big_endian_multibyte(f, e->index, l);
            e = e->next;
        } while (e != elast);
        if(i <pg-> nv - 1){
            fputc(255, f);
        }
    }
}

void write_edge_code(PLANE_GRAPH *pg, FILE *f, boolean print_header){
    int i, counter=0;
    PG_EDGE *e, *elast;
    
    if(print_header){
        fprintf(f, ">>edge_code<<");
    }
    
    //label the edges
    for(i=0; i<pg->nv; i++){
        e = elast = pg->first_edge[i];
        do {
            e->index = -1;
            e = e->next;
        } while (e != elast);
    }
    for(i=0; i<pg->nv; i++){
        e = elast = pg->first_edge[i];
        do {
            if(e->index == -1){
                e->index = counter;
                e->inverse->index = counter;
                counter++;
            }
            e = e->next;
        } while (e != elast);
    }
    
    if (pg->ne + pg->nv - 1 <= 255) {
        write_edge_code_small(pg, f);
    } else {
        write_edge_code_large(pg, f);
    }
    
}

void write_dual_edge_code_small(PLANE_GRAPH *pg, FILE *f){
    PG_EDGE *e, *elast;

    //write the length of the body
    fputc(pg->ne + pg->nf - 1, f);

    for(int i=0; i<pg->nf; i++){
        e = elast = pg->face_start[i];
        do {
            fputc(e->index, f);
            e = e->inverse->prev;
        } while (e != elast);
        if(i < pg->nf - 1){
            fputc(255, f);
        }
    }
}

void write_dual_edge_code_large(PLANE_GRAPH *pg, FILE *f){
    fprintf(stderr, "Graphs of that size are currently not supported -- exiting!\n");
    exit(-1);
}

/**
 * Output the edge code of the dual of the graph
 * @param pg
 * @param f
 * @param print_header Include the header for this code
 */
void write_dual_edge_code(PLANE_GRAPH *pg, FILE *f, boolean print_header){
    int counter=0;
    PG_EDGE *e, *elast;

    if(print_header){
        fprintf(f, ">>edge_code<<");
    }

    if(!pg->faces_constructed) construct_faces(pg);

    //label the edges
    for(int i=0; i<pg->nv; i++){
        e = elast = pg->first_edge[i];
        do {
            e->index = -1;
            e = e->next;
        } while (e != elast);
    }
    for(int i=0; i<pg->nv; i++){
        e = elast = pg->first_edge[i];
        do {
            if(e->index == -1){
                e->index = counter;
                e->inverse->index = counter;
                counter++;
            }
            e = e->next;
        } while (e != elast);
    }

    if (pg->ne + pg->nf - 1 <= 255) {
        write_dual_edge_code_small(pg, f);
    } else {
        write_dual_edge_code_large(pg, f);
    }

}
