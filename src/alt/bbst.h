#ifndef __BBST_H__
#define __BBST_H__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "geometry.h"

typedef struct Node {
    struct Node * left;
    struct Node * right;
    struct Node * parent;

    Point * site_left;
    Point * site_right;
    Edge * voronoi_edge;


    bool is_leaf;
    Point * arc_point; 
    Circle * circle; 
    struct Node *left_leaf;
    struct Node *right_leaf;
} node_t;

typedef struct bbst {
    node_t * root; 
    
    bool is_empty;
} bbst;

bbst* create_new_bbst();
node_t* create_new_node();

/////////

bool is_empty(bbst * tree);
void insert_site(bbst * tree, Point * p);
float get_breakpoint(node_t * node, float sl);
node_t * get_arc_above(node_t * tree, Point * p);
node_t * replace_leaf(node_t * leaf, Point * new_site);
void get_arc_junctions(node_t * arc, node_t ** j_1, node_t ** j_2);
void delete_leaf(node_t * node, node_t * j_1, node_t * j_2);

void free_bbst(bbst * tree);

void print_tree(bbst * tree);
#endif

