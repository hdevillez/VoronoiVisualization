#ifndef __EVENT_H__
#define __EVENT_H__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "geometry.h"
#include "bbst.h"

typedef struct event {
    char type;
    Point * p;
    Circle * circle;
    node_t * leaf; 
} Event;

Event * create_new_event(char type, Point * p);
#endif 
