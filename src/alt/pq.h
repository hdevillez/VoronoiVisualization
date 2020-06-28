#ifndef __PQ_H__
#define __PQ_H__

#include <stdio.h>
#include <stdlib.h>
#include "geometry.h"
#include "event.h"

typedef struct {
    int size_max;
    int size;
    Event ** data;
} heap_t;

heap_t * create_new_heap(int size_max);
void free_heap(heap_t * h);
void print_pq(heap_t *h);

int left_node(int a);
int right_node(int a);
float priority(Event *e);

void heap_push(heap_t *h, Event * new_event);
Event * heap_pop(heap_t *h);
#endif 
