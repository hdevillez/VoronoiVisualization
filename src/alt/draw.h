#ifndef __DRAW_H__
#define __DRAW_H__

#include "draw_tools.h"
#include "geometry.h"

void print_frame(window_t * window, float y);
void print_animation(window_t * window, float from_sl, float to_sl);
void print_animation_step(window_t * window, float y);
void print_cell(window_t * window, Face * f);

void init_draw(float * coords);
void free_draw();


#endif
