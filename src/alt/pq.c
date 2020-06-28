#include "pq.h"
#include "event.h"
#include "geometry.h"
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <math.h>

heap_t * create_new_heap(int size_max) {
    heap_t * new_heap = malloc(sizeof(heap_t));
    new_heap->data = malloc(sizeof(Event *)*size_max);
    new_heap->size_max = size_max;
    new_heap->size = 0; 
    for(int i= 0; i < size_max; i++) new_heap->data[i] = NULL;
    return new_heap;
}

void free_heap(heap_t * h) {
    if(h->data != NULL) { 
        free(h->data);
        h->data = NULL;
    }
    
    if(h != NULL) { 
        free(h);
        h = NULL;
    }
    
}

int left_node(int a) {return 2*a;}
int right_node(int a) {return 2*a+1;}
float priority(Event *e) {return e->p->y;}

void print_pq(heap_t *h) {
     
    for(int i= 0; i < h->size; i++) {
       for(int j= 0; j < (int) floor(log(i+1)/log(2)); j++) {
            printf("\t"); 
       } 
       printf("%f\n", priority(h->data[i]));
    }
}

void heap_push(heap_t *h, Event * new_event) {
    Event ** data = h->data; 
    data[h->size] = new_event;
    int curr = ++(h->size);
    while(curr > 1) {
        if(priority(h->data[curr-1]) > priority(h->data[(curr/2)-1])) {
            Event * temp = data[curr-1];
            data[curr-1] = data[(curr/2)-1];
            data[(curr/2)-1] = temp;
        }
        else break;
        
        curr /= 2;
    }

    //printf("IN : %f\n", new_event->p->y);
}

Event * heap_pop(heap_t * h) {
    assert(h->size > 0); 

    Event ** data = h->data; 
    Event * top_data = data[0];
    data[0] = data[--(h->size)];
    
    int curr = 1;

    
    while(curr < h->size) {
        int to_swap= -1;
        if(left_node(curr) > h->size) break;
        if(priority(data[left_node(curr)-1]) > priority(data[curr-1])) to_swap = left_node(curr);
        
        if(right_node(curr) <= h->size) {
            if(priority(data[right_node(curr)-1]) > priority(data[curr-1])) {
                if(to_swap == -1 || priority(data[right_node(curr)-1]) > priority(data[to_swap-1]))
                    to_swap = right_node(curr);
            } 
        }

        if(to_swap == -1) break;
        
        Event * temp = data[curr-1];
        data[curr-1] = data[to_swap-1];
        data[to_swap-1] = temp;
        curr = to_swap;
    }
    //printf("OUT : %f\n", top_data->p->y);
    return top_data;
}
