cmake_minimum_required(VERSION 3.15)
project(planegraph C)

set(CMAKE_C_STANDARD 99)

add_library(planegraph planegraphs_base.c planegraphs_base.h planegraphs_input.c planegraphs_input.h planegraphs_output.c planegraphs_output.h planegraphs_automorphismgroup.c planegraphs_automorphismgroup.h)