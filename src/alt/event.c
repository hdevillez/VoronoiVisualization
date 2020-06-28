#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "event.h"
#include "geometry.h"
#include "bbst.h"

Event * create_new_event(char type, Point * p) {
    Event * new_event = malloc(sizeof(Event));
    new_event->type = type;
    new_event->p = p;
    return new_event;
}
