#include "geometry.h"
#include <math.h>
#include <gmodule.h>
#include <assert.h>
#include <stdbool.h> 

#define EPS 1e-9

extern float left_lim;
extern float right_lim;
extern float up_lim;
extern float down_lim;

Point * create_new_point(float x, float y) {
    Point * new_point = malloc(sizeof(Point));
    new_point->x = x;
    new_point->y = y;
    return new_point;
}

Edge * create_new_edge(Vertex * v){
    Edge * new_edge = malloc(sizeof(Edge));
    new_edge->origin = v;
    new_edge->face = NULL;
    new_edge->twin = NULL;
    new_edge->next = NULL;
    new_edge->prev = NULL;
    return new_edge;
}

Vertex * create_new_vertex(Point *a) {
    Vertex * new_vertex = malloc(sizeof(Vertex));
    new_vertex->p = a;
    new_vertex->e = NULL;
    return new_vertex;
}
Face * create_new_face(Point * a, Edge * e){
    Face * new_face = malloc(sizeof(Face));
    new_face->p = a;
    new_face->e = e;
    return new_face;
} 
Circle * create_new_circle(float cx, float cy, float r){
    Circle * new_circle = malloc(sizeof(Circle));
    new_circle->cx = cx;
    new_circle->cy = cy;
    new_circle->r = r;
    new_circle->valid = true; 
    return new_circle;
}

void make_twin(Edge * e1, Edge *e2) {
    e1->twin = e2;
    e2->twin = e1;
}

void chain(Edge *e1, Edge *e2) {
    e1->next = e2;
    e2->prev = e1;
}

float in_box(float x, float y) {
    return (x >= left_lim && x <= right_lim && y <= up_lim && y >= down_lim);
}

Circle * get_circle(Point *a, Point *b, Point *c) {
    if(fabs(a->x - b->x) < EPS && fabs(a->y - b->y) < EPS) {
        // two times same point
        return create_new_circle(-1, -1, 0); 
    }
    if(fabs(a->x - c->x) < EPS && fabs(a->y - c->y) < EPS) {
        // two times same point
        return create_new_circle(-1, -1, 0); 
    }
    if(fabs(b->x - c->x) < EPS && fabs(b->y - c->y) < EPS) {
        // two times same point
        return create_new_circle(-1, -1, 0); 
    }
    if(a->y == b->y && b->y == c->y) {
        // colinear points => no circle
        return create_new_circle(-1, -1, 0); 
    }
   
    Point * ba = create_new_point(b->x-a->x, b->y-a->y);
    Point * ca = create_new_point(c->x-a->x, c->y-a->y);

    float ba_l = ba->x*ba->x + ba->y*ba->y;
    float ca_l = ca->x*ca->x + ca->y*ca->y;

    float d = 2*(ba->x *ca->y - ba->y * ca->x);

    if(d >= 0) {
        free(ba);
        ba = NULL;
        free(ca);
        ca = NULL;
        return create_new_circle(-1, -1, 0); 
    }
    float cx = a->x + (ca->y*ba_l - ba->y *ca_l)/d;
    float cy = a->y + (ba->x*ca_l - ca->x *ba_l)/d;

    float dx = (a->x - cx);
    float dy = (a->y - cy);
    float r = sqrt(dx*dx+dy*dy);

    free(ba);
    free(ca);
    ba = NULL;
    ca = NULL;

    return create_new_circle(cx, cy, r);
}

float dist_points(Point *a, Point *b) {
    float dx =a->x-b->x;
    float dy = a->y-b->y;
    return sqrt(dx*dx+dy*dy);
}

float parabola(Point * site, float sl, float x) {
    assert(site->y > sl);
    return (x*x + site->x*site->x + site->y*site->y - sl*sl - 2*site->x*x)/(2*(site->y - sl)); 
}

