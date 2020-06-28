#ifndef __GEOMETRY_H__
#define __GEOMETRY_H__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

typedef struct Point {
    float x, y;
} Point;

typedef struct Edge {
    struct Vertex * origin;
    struct Face * face;
    struct Edge * twin;
    struct Edge * next;
    struct Edge * prev;
} Edge;

typedef struct Vertex {
    Point * p;
    Edge * e;
} Vertex;

typedef struct Face {
    Point * p;
    Edge * e;
} Face;

typedef struct circle {
    float cx;
    float cy;
    float r;
    bool valid;
} Circle;


Point * create_new_point(float x, float y);
Edge * create_new_edge(Vertex * a);
Vertex * create_new_vertex(Point * a);
Face * create_new_face(Point * a, Edge * e);

Circle * create_new_circle(float cx, float cy, float r);

void make_twin(Edge * e1, Edge *e2);
void chain(Edge *e1, Edge *e2);

float in_box(float x, float y);
Circle * get_circle(Point *a, Point *b, Point *c);
float dist_points(Point *a, Point *b);

float parabola(Point * site, float sl, float x);

#endif
