#include "draw.h"
#include "bbst.h"
#include "geometry.h"
#include "draw_tools.h"
#include <gmodule.h>
#include <math.h>

int n_points_beachline = 1000;
float simulation_time = 6;
float step_time = 0.5;

bool gif_flag = false;
float gif_rate = 1.0/15.0; // 15 fps
float tcapture;
char * gif_dir;
int frame_index;

bool DRAW_CIRCLE_OPTION = 1;
bool DRAW_BEACHLINE_OPTION = 1;
bool DRAW_SWEEPLINE_OPTION = 1;
bool DRAW_CELL_OPTION = 1;

const int NO_ANIMATION = 0;
const int CONTINUOUS_ANIMATION = 1;
const int STEP_ANIMATION = 2;


// Limit of the bounding box
float left_lim = -0.75;
float right_lim = 0.75;
float up_lim = 0.75;
float down_lim = -0.75;

// Global variable of the fortune algorithm
extern bbst * beachline;
extern GList * voronoi_edges_set;
extern GList * circles_set;

// points_t and coords structures used in the visualization
extern Point ** points;
extern int n_points;

points_t* pointset;
points_t* box_pointset;
points_t* sweepline_pointset;
points_t* beachline_pointset;
points_t* edge_pointset;
points_t* circle_pointset;
points_t* breakpoint_pointset;

float * points_coords;
float * box_coords;
float * sweepline_coords;
float * beachline_coords;
float * edge_coords;
float * circle_coords;
float * breakpoint_coords;


void print_box(window_t * window) {
    points_set_width(box_pointset, 0.76);
	points_set_color(box_pointset, (float[4]) {0.8, 1.0, 1.0, 0.6});
    points_set_outline_color(box_pointset, (float[4]) {0, 0, 0, 1});
    points_set_outline_width(box_pointset, 0.01);

    box_coords[0] = 0;
    box_coords[0] = 0;

    box_pointset = points_update(box_pointset, box_coords, 1);
    points_draw(window, box_pointset);

}

void print_sweepline(window_t * window, float sl) {

    if(sl <= down_lim) return;
	
    points_set_width(sweepline_pointset, 0.01);
	points_set_color(sweepline_pointset, (float[4]) {0.7, 0.5, 0.0, 1.0});


    sweepline_coords[0] = left_lim;
    sweepline_coords[1] = sl;
    sweepline_coords[2] = right_lim;
    sweepline_coords[3] = sl;

    sweepline_pointset = points_update(sweepline_pointset, sweepline_coords, 2);
    line_strip_draw(window, sweepline_pointset);
}

void print_beachline(window_t * window, float sl) {

	points_set_width(beachline_pointset, 0.01);
	points_set_color(beachline_pointset, (float[4]) {0.2, 0.5, 10.0, 1.0});

    for(int i= 0; i < n_points_beachline; i++) {
        float x = left_lim + (i*1.0/(n_points_beachline-1))*(right_lim-left_lim) ;
        beachline_coords[2*i] = x;
        beachline_coords[2*i+1] = up_lim;
        for(int j= 0; j < n_points; j++) {
            if(points[j]->y > sl) {
                float y = parabola(points[j], sl, x);
                beachline_coords[2*i+1]  = fmax(down_lim, fmin(beachline_coords[2*i+1], y));
            }
        }
    }

    beachline_pointset = points_update(beachline_pointset, beachline_coords, n_points_beachline);
    line_strip_draw(window, beachline_pointset);
     
}

void print_points(window_t * window, float sl) {
	points_set_width(pointset, 0.01);
	points_set_marker(pointset, 5.0);
	points_set_color(pointset, (float[4]) {0.2, 0.5, 0.0, 1.0});
    points_draw(window, pointset);
}

void print_circles(window_t * window, float sl) {
    points_set_marker(circle_pointset, 5.0);
	points_set_color(circle_pointset, (float[4]) {0.2, 0.5, 0.6, 0});
    points_set_outline_width(circle_pointset, 0.01);

    int i= 0;
    for(GList * elem = circles_set; elem; elem = elem->next) {
        Circle * c = elem->data;
        circle_coords[0]  = c->cx;
        circle_coords[1]  = c->cy;
        if(c->valid) 
            points_set_outline_color(circle_pointset, (float[4]) {0.2, 0.5, 0.6, 0.1});
        else 
            points_set_outline_color(circle_pointset, (float[4]) {0.8, 0.5, 0.6, 0.1});
       
        points_set_width(circle_pointset, c->r);
        
        circle_pointset = points_update(circle_pointset, circle_coords, 1);
        points_draw(window, circle_pointset);
        i++;
    }
}


void draw_edge(window_t * window, float bp_x_1, float bp_y_1,float bp_x_2, float bp_y_2) {

    float lmbd_1 = 1.0;
    
    lmbd_1 = fmin(lmbd_1, (fmin(bp_x_1, right_lim) - bp_x_2) / (bp_x_1 - bp_x_2));
    lmbd_1 = fmin(lmbd_1, (fmax(bp_x_1, left_lim) - bp_x_2) / (bp_x_1 - bp_x_2));
    lmbd_1 = fmin(lmbd_1, (fmin(bp_y_1, up_lim) - bp_y_2) / (bp_y_1 - bp_y_2));
    lmbd_1 = fmin(lmbd_1, (fmax(bp_y_1, down_lim) - bp_y_2) / (bp_y_1 - bp_y_2));
    lmbd_1 = fmax(0, lmbd_1);

    bp_x_1 = bp_x_2 + lmbd_1 * (bp_x_1 - bp_x_2);
    bp_y_1 = bp_y_2 + lmbd_1 * (bp_y_1 - bp_y_2);
   
    float lmbd_2 = 1.0;

    lmbd_2 = fmin(lmbd_2, (fmin(bp_x_2, right_lim) - bp_x_1) / (bp_x_2 - bp_x_1));
    lmbd_2 = fmin(lmbd_2, (fmax(bp_x_2, left_lim) - bp_x_1) / (bp_x_2 - bp_x_1));
    lmbd_2 = fmin(lmbd_2, (fmin(bp_y_2, up_lim) - bp_y_1) / (bp_y_2 - bp_y_1));
    lmbd_2 = fmin(lmbd_2, (fmax(bp_y_2, down_lim) - bp_y_1) / (bp_y_2 - bp_y_1));

    lmbd_2 = fmax(0, lmbd_2);

    bp_x_2 = bp_x_1 + lmbd_2 * (bp_x_2 - bp_x_1);
    bp_y_2 = bp_y_1 + lmbd_2 * (bp_y_2 - bp_y_1);

    edge_coords[0]  = bp_x_1;
    edge_coords[1]  = bp_y_1;
    edge_coords[2]  = bp_x_2;
    edge_coords[3]  = bp_y_2;

    edge_pointset = points_update(edge_pointset, edge_coords, 2);
    line_strip_draw(window, edge_pointset);
}

void print_edges_(window_t * window, node_t * node, float y) {
    
    if(node == NULL) return;
    if(node->is_leaf) return;
    if(y < fmin(node->site_left->y, node->site_right->y)) {
        Edge *e = node->voronoi_edge;
        Edge *e_twin = e->twin;
        if(true || e->origin->p->x != e_twin->origin->p->x || e->origin->p->y != e_twin->origin->p->y) {
            float bp_x_1 = get_breakpoint(node, y);                
            float bp_y_1 = parabola(node->site_right, y, bp_x_1);  
            
            draw_edge(window, bp_x_1, bp_y_1, e->origin->p->x, e->origin->p->y);
        }
    }
    print_edges_(window, node->left, y);
    print_edges_(window, node->right, y);
}

void print_edges(window_t * window, float y) {
    
	points_set_color(edge_pointset, (float[4]) {0.0, 0.0, 0.0, 1.0});
    points_set_width(edge_pointset, 0.01); 

    // print infinite half-edges
    print_edges_(window, beachline->root, y);

    // print complete segments
    int i= 0;
    for(GList * elem = voronoi_edges_set; elem; elem = elem->next) {
        Edge * e = elem->data;
        Edge * e_twin = e->twin;
        
        //printf("%f %f\n", e->origin->p->x,e_twin->origin->p->x);
        if(e->origin->p->x == e_twin->origin->p->x && e->origin->p->y == e_twin->origin->p->y)
            continue;
            
        draw_edge(window, e->origin->p->x, e->origin->p->y, e_twin->origin->p->x, e_twin->origin->p->y);
        i++;
    }

}

void print_cell(window_t * window, Face * f) {
    
    Edge * start = f->e;
    Edge * curr = start->next;

	points_set_color(edge_pointset, (float[4]) {1.0, 0.0, 0.0, 1.0});
    points_set_width(edge_pointset, 0.011); 
    
    while(start != curr) {
        draw_edge(window, curr->origin->p->x, curr->origin->p->y, curr->twin->origin->p->x, curr->twin->origin->p->y); 
        curr = curr->next;
    }
    draw_edge(window, curr->origin->p->x, curr->origin->p->y, curr->twin->origin->p->x, curr->twin->origin->p->y); 
}

void print_breakpoints_(window_t * window, node_t * node, float y) {

    if(node == NULL) return;
    if(node->is_leaf) return;
    if(y < fmin(node->site_left->y, node->site_right->y)) {

        breakpoint_coords[0] = get_breakpoint(node, y);                
        breakpoint_coords[1] = parabola(node->site_right, y, breakpoint_coords[0]);  
        if(in_box(breakpoint_coords[0], breakpoint_coords[1])) {
            breakpoint_pointset = points_update(breakpoint_pointset, breakpoint_coords, 1);
            points_draw(window, breakpoint_pointset);
        }
    }
    print_breakpoints_(window, node->left, y);
    print_breakpoints_(window, node->right, y);

}

void print_breakpoints(window_t * window, float y) {
    points_set_width(breakpoint_pointset, 0.05);
    points_set_marker(breakpoint_pointset, 3.0);
    points_set_color(breakpoint_pointset, (float[4]) {0.6, 0.9, 0.6, 1});

    print_breakpoints_(window, beachline->root, y);
}

void print_frame(window_t * window, float y) {
    print_box(window);
    if(DRAW_CIRCLE_OPTION) print_circles(window, y);
    print_edges(window, y);
    if(DRAW_SWEEPLINE_OPTION) print_sweepline(window, y);
    if(DRAW_BEACHLINE_OPTION) print_beachline(window, y);
    print_points(window, y);
    print_breakpoints(window, y);

}

void print_animation(window_t * window, float from_sl, float to_sl) {
    double tbegin = window_get_time(window);
    double tnow = tbegin;

    while(tnow - tbegin < simulation_time * (from_sl-to_sl)/(up_lim-down_lim)) {
        tnow = window_get_time(window);
        double delta = (tnow-tbegin)/simulation_time*(up_lim-down_lim); 
        float y = from_sl - delta;

        print_frame(window, y);
        window_update(window); 

        if(gif_flag && tnow - tcapture > gif_rate) {
            char filename[50];
            sprintf(filename, "%s/anim_%03d.png", gif_dir, frame_index); 
            window_screenshot(window, filename);
            tcapture = window_get_time(window); 
            frame_index++; 
        }
    }
}

void print_animation_step(window_t * window, float y) {
    double tbegin = window_get_time(window);
    double tnow = tbegin;
    while(tnow - tbegin < step_time) {
        tnow = window_get_time(window);
        print_frame(window, y);
        window_update(window); 
    }

}

void init_draw(float * coords) {

    if(gif_flag) {
        tcapture = 0; 
        frame_index = 0;
    }

    box_coords = malloc(sizeof(float)*2);
    sweepline_coords = malloc(sizeof(float)*4);
    beachline_coords = malloc(sizeof(float)*n_points_beachline*2);
    edge_coords = malloc(sizeof(float)*4);
    circle_coords = malloc(sizeof(float)*2); 
    breakpoint_coords = malloc(sizeof(float)*2); 

    pointset = points_new(coords, n_points, GL_STATIC_DRAW);
    box_pointset = points_new(box_coords, 1, GL_STATIC_DRAW);
    sweepline_pointset = points_new(sweepline_coords, 2, GL_STATIC_DRAW);
    beachline_pointset = points_new(beachline_coords, n_points_beachline, GL_STATIC_DRAW);
    edge_pointset = points_new(edge_coords, 2, GL_STATIC_DRAW); 
    circle_pointset = points_new(circle_coords, 1, GL_STATIC_DRAW); 
    breakpoint_pointset = points_new(breakpoint_coords, 1, GL_STATIC_DRAW); 
}

void free_draw() {
    points_delete(pointset);
    points_delete(sweepline_pointset);
    points_delete(beachline_pointset);
    points_delete(box_pointset);	
    points_delete(circle_pointset);	
    points_delete(edge_pointset);	
    points_delete(breakpoint_pointset);

    free(box_coords); 
    free(sweepline_coords);
    free(beachline_coords);
    free(edge_coords);
    free(circle_coords);
    free(breakpoint_coords);
}
