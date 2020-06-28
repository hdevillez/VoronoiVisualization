#include "fortune.h"
#include "geometry.h" 
#include "bbst.h"
#include "event.h" 
#include "pq.h"
#include "draw_tools.h"
#include "draw.h"
#include <math.h>
#include <gmodule.h>
#include <assert.h>
#include <stdbool.h> 

#define EPS 1e-9

bool verbose = 1;
bool debug = 0;

extern float up_lim;
extern float down_lim;

extern bool DRAW_CIRCLE_OPTION;
extern bool DRAW_BEACHLINE_OPTION;
extern bool DRAW_SWEEPLINE_OPTION;
extern bool DRAW_CELL_OPTION;

extern const int NO_ANIMATION;
extern const int CONTINUOUS_ANIMATION;
extern const int STEP_ANIMATION;

int event_compare(gconstpointer a, gconstpointer b) {
    Event * ea = (Event *) a;
    Event * eb = (Event *) b;

    return (ea->p->y) - (eb->p->y);
}

GList * voronoi_edges_set = NULL;
GList * voronoi_vertices_set = NULL;
GList * voronoi_faces_set = NULL;
GList * circles_set = NULL;

bbst * beachline;
heap_t * pq_events;

int n_points;
Point ** points = NULL;

void free_points() {
    for(int i= 0; i < n_points; i++) {
        if(points[i] != NULL)  
            free(points[i]);
        points[i] = NULL; 
    }
}

void free_circles() {
    for(GList * elem = circles_set; elem; elem = elem->next) {
        if(elem->data != NULL)    
            free(elem->data);
        elem->data = NULL; 
    }
}

void free_edges() {
    for(GList * elem = voronoi_edges_set; elem; elem = elem->next) {
        Edge *e = elem->data; 

        if(e != NULL)
            free(e);
        e = NULL; 
    }

}

void free_vertices() {
    for(GList * elem = voronoi_vertices_set; elem; elem = elem->next) {
        Vertex *v = elem->data; 

        if(v->p != NULL)
            free(v->p);
        v->p = NULL; 

        if(v != NULL)
            free(v);
        v = NULL; 
    }
}

void free_faces() {
    for(GList * elem = voronoi_faces_set; elem; elem = elem->next) {
        Face *f = elem->data; 

        if(f != NULL)
            free(f);
        f = NULL; 
    }
}

void create_left_circle(node_t * right_arc) {
    node_t *middle_arc = right_arc->left_leaf;
    if(middle_arc != NULL) {
        node_t *left_arc = middle_arc->left_leaf;
        if(left_arc != NULL) {
            Circle * circle = get_circle(left_arc->arc_point, middle_arc->arc_point, right_arc->arc_point); 
            if(fabs(circle->r) < EPS) {  
                if(verbose) {
                    printf(" - - Not convergent new breakpoint \n");
                }
                if(circle != NULL)
                    free(circle);
                circle = NULL; 
                return ;
            }

            Event * circle_event = create_new_event('c', create_new_point(circle->cx, circle->cy-circle->r)); 
            circle_event->leaf = middle_arc;
            circle_event->circle = circle;
            middle_arc->circle = circle; 
            heap_push(pq_events, circle_event); 
            circles_set = g_list_append(circles_set, circle); 
            if(verbose) {
                printf(" - - Create new circle event; center (%f %f) - radius %f\n", circle->cx, circle->cy, circle->r);
            }
        }
    } 
}

void create_right_circle(node_t * left_arc) {
    node_t *middle_arc = left_arc->right_leaf;
    if(middle_arc != NULL) {
        node_t *right_arc = middle_arc->right_leaf;
        if(right_arc != NULL) {
            Circle * circle = get_circle(left_arc->arc_point, middle_arc->arc_point, right_arc->arc_point); 
            if(fabs(circle->r) < EPS) {   
                if(verbose) {
                    printf(" - - Not convergent new breakpoint \n");
                }
                if(circle != NULL)
                    free(circle);
                circle = NULL; 
                return; 
            }
            Event * circle_event = create_new_event('c', create_new_point(circle->cx, circle->cy-circle->r)); 
            circle_event->leaf = middle_arc;
            circle_event->circle = circle;
            middle_arc->circle = circle; 
            heap_push(pq_events, circle_event); 
            circles_set = g_list_append(circles_set, circle); 
            if(verbose) {
                printf(" - - Create new circle event center - (%f %f) - radius %f\n",  circle->cx, circle->cy, circle->r);
            }
        }
    } 

}

void handle_site_event(Event * e) {
    Point * p = e->p;
    if(verbose) printf(" - Handle site event : %f %f\n", p->x, p->y); 

    if(is_empty(beachline)) {
        insert_site(beachline, p); 
    }
    else {
        node_t * arc_above = get_arc_above(beachline->root, p); 
        Point * site_above = arc_above->arc_point; 
        if(arc_above->circle != NULL) {
            arc_above->circle->valid = false;
        }

        if(verbose) printf(" - - Site above : %f %f\n", site_above->x, site_above->y);

        assert(arc_above != NULL);        

        node_t * i_node = replace_leaf(arc_above, p);
        if(i_node->parent == NULL) {
            beachline->root = i_node; 
        } 
        Point * mp = create_new_point(p->x, parabola(site_above, p->y, p->x));

        Vertex * new_vertex = create_new_vertex(mp);
        Edge * new_edge_1 = create_new_edge(new_vertex); 
        Edge * new_edge_1_twin = create_new_edge(new_vertex);
        make_twin(new_edge_1, new_edge_1_twin);

        Edge * new_edge_2 = create_new_edge(new_vertex); 
        Edge * new_edge_2_twin = create_new_edge(new_vertex); 
        make_twin(new_edge_2, new_edge_2_twin);

        chain(new_edge_1, new_edge_1_twin);
        chain(new_edge_1_twin, new_edge_2);
        chain(new_edge_2, new_edge_2_twin);
        chain(new_edge_2_twin, new_edge_1);

        
        Face * f1 = create_new_face(p, new_edge_1);
        new_edge_1->face = f1;
        new_edge_2_twin->face = f1;
        
        Face * f2 = create_new_face(site_above, new_edge_1->twin);
        new_edge_2->face = f2;
        new_edge_1_twin->face = f2;
    
        voronoi_edges_set = g_list_append(voronoi_edges_set, new_edge_1);
        voronoi_edges_set = g_list_append(voronoi_edges_set, new_edge_1_twin);
        voronoi_edges_set = g_list_append(voronoi_edges_set, new_edge_2);
        voronoi_edges_set = g_list_append(voronoi_edges_set, new_edge_2_twin);
        voronoi_vertices_set = g_list_append(voronoi_vertices_set, new_vertex);
        voronoi_faces_set = g_list_append(voronoi_faces_set, f1);
        voronoi_faces_set = g_list_append(voronoi_faces_set, f2);

        i_node->voronoi_edge = new_edge_1;
        i_node->right->voronoi_edge = new_edge_2;

        // TODO rebalance

        node_t *middle_arc = i_node->right->left;
        create_left_circle(middle_arc); 
        create_right_circle(middle_arc); 

    }
}

void handle_circle_event(Event * e) {

    Circle * c = e->circle;
    if(verbose) printf(" - Handle circle event : %f %f\n", c->cx, c->cy); 
    if(!c->valid) { 

        if(e->p != NULL)
            free(e->p);
        e->p = NULL;

        if(verbose) printf(" - - invalid event \n"); 
        return; 
    } 

    // get disappearing arc
    node_t *leaf = e->leaf;
    node_t *left_leaf = leaf->left_leaf;
    node_t *right_leaf = leaf->right_leaf;

    if(left_leaf != NULL && left_leaf->circle != NULL) {
        left_leaf->circle->valid = false;
    }
    if(right_leaf != NULL && right_leaf->circle != NULL) {
        right_leaf->circle->valid = false;
    }

    node_t * j_1 = NULL; node_t * j_2 = NULL;
    get_arc_junctions(leaf, &j_1, &j_2);

    Edge *left_edge = j_1->voronoi_edge;
    Edge *right_edge = j_2->voronoi_edge;

    if(j_1->left == leaf) {
        Edge * tmp = left_edge;
        left_edge = right_edge;
        right_edge = tmp;
    }

    ////

    Point * bp = create_new_point(c->cx, c->cy);

    Vertex * new_vertex = create_new_vertex(bp);
    Edge * new_edge = create_new_edge(new_vertex); 
    Edge * new_edge_twin = create_new_edge(new_vertex);
    make_twin(new_edge, new_edge_twin);
    new_vertex->e = new_edge;


    chain(new_edge, new_edge_twin);   
    chain(left_edge, right_edge->twin);   
    chain(new_edge_twin, left_edge->twin);
    chain(right_edge, new_edge);   

    new_edge->face = right_edge->face;
    new_edge_twin->face = left_edge->twin->face;

    voronoi_edges_set = g_list_append(voronoi_edges_set, new_edge);
    voronoi_edges_set = g_list_append(voronoi_edges_set, new_edge_twin);
    voronoi_vertices_set = g_list_append(voronoi_vertices_set, new_vertex);

    j_2->voronoi_edge = new_edge;
    left_edge->twin->origin = new_vertex;
    right_edge->twin->origin = new_vertex;

    ////

    delete_leaf(leaf, j_1, j_2);

    if(left_leaf != NULL &&  left_leaf->left_leaf != NULL) {
        create_right_circle(left_leaf->left_leaf); 
    }
    if(right_leaf != NULL &&  right_leaf->right_leaf != NULL) {
        create_left_circle(right_leaf->right_leaf); 
    }

    if(e->p != NULL)
        free(e->p);
    e->p = NULL;
}

void extract_half_lines_(node_t * node) {

    if(node == NULL) return;
    if(node->is_leaf) return;

    Edge *e = node->voronoi_edge;

    // Extract breakpoint à infinity for half-edges
    float bp_x = get_breakpoint(node, -1000);                
    float bp_y = parabola(node->site_right, -1000, bp_x);  

    Point * bp = create_new_point(bp_x, bp_y);

    Vertex * new_vertex = create_new_vertex(bp);
    e->twin->origin = new_vertex;

    voronoi_vertices_set = g_list_append(voronoi_vertices_set, new_vertex);

    extract_half_lines_(node->left);
    extract_half_lines_(node->right);
}

void extract_half_lines() {
    extract_half_lines_(beachline->root);
}

void connect_half_lines() {
    Edge * start;
    for(GList * elem = voronoi_edges_set; elem; elem = elem->next) {
        Edge *e = elem->data;
        if(!in_box(e->origin->p->x, e->origin->p->x)) {
            start = e->prev;
            break;
        }
    }
    Edge * prev = start->next;
    Edge * curr = prev->next;

    while(curr != start) {
        if(!in_box(curr->origin->p->x, curr->origin->p->y)) {
            Edge * new_edge = create_new_edge(prev->origin);  
            Edge * new_edge_twin = create_new_edge(curr->origin);
            make_twin(new_edge, new_edge_twin);
            
            chain(prev->prev, new_edge);
            chain(new_edge_twin, prev);
            chain(curr->prev, new_edge_twin);
            chain(new_edge, curr);

            voronoi_edges_set = g_list_append(voronoi_edges_set, new_edge);
            voronoi_edges_set = g_list_append(voronoi_edges_set, new_edge_twin);

            prev = curr;
        }
        curr = curr->next; 
    }
    Edge * new_edge = create_new_edge(prev->origin);  
    Edge * new_edge_twin = create_new_edge(start->origin);
    make_twin(new_edge, new_edge_twin);
    
    chain(prev->prev, new_edge);
    chain(new_edge, start->next);
    chain(new_edge_twin, prev);
    chain(start, new_edge_twin);
    
    voronoi_edges_set = g_list_append(voronoi_edges_set, new_edge);
    voronoi_edges_set = g_list_append(voronoi_edges_set, new_edge_twin);
}

void fortune_algorithm(window_t * window, float * coords, int n_points_, int animation_type) {

    n_points = n_points_;

    beachline = create_new_bbst();
    pq_events = create_new_heap(100*n_points); 

    init_draw(coords);
    points = malloc(sizeof(Point *)*n_points);

    for(int i= 0; i < n_points; i++) {
        Point * p = create_new_point(coords[2*i], coords[2*i+1]);
        Event * e = create_new_event('s', p); 
        points[i] = p;  
        heap_push(pq_events, e);
    }

    float curr_sl = up_lim;
    while(pq_events->size > 0) {
        Event *e = heap_pop(pq_events);
        float new_sl = e->p->y;


        if(animation_type == CONTINUOUS_ANIMATION)
            print_animation(window, curr_sl, new_sl);
        else if(animation_type == STEP_ANIMATION)
            print_animation_step(window, new_sl);
            

        if(e->type == 's')
            handle_site_event(e);
        else if(e->type == 'c')
            handle_circle_event(e);
        if(debug) {
            printf("\nPRINT TREE\n");
            print_tree(beachline);
            printf("\n\n\n");
        }
        curr_sl = new_sl;

        free(e);
    }

    extract_half_lines();
    connect_half_lines(); 
    
     

    printf("Algorithm done\n");

    DRAW_BEACHLINE_OPTION = 0;
    DRAW_SWEEPLINE_OPTION = 0;
    DRAW_CIRCLE_OPTION = 0;

   

    // draw
    while(!window_is_closed(window)) {
        print_frame(window, -1000);	
        
        if(DRAW_CELL_OPTION && in_box(window->cursorPos[0], window->cursorPos[1])) {
           
            Point * cursor_pos = create_new_point(window->cursorPos[0], window->cursorPos[1]);
            int i_min =-1;

            for(int i= 0; i < n_points; i++) {
                if(i_min == -1 || dist_points(points[i], cursor_pos) < dist_points(points[i_min], cursor_pos)) {
                    i_min = i;
                } 
            }

            GList * elem = voronoi_faces_set;
            while(((Face *) (elem->data))->p != points[i_min]) elem = elem->next;
            Face *f = elem->data;    
            
            print_cell(window, f);
        
            free(cursor_pos); 
            cursor_pos = NULL;
        }


        window_update_and_wait_events(window);
    }

    free_draw();
    free_bbst(beachline);
    free_points();
    free_circles();
    free_edges();
    free_vertices(); 
    free_faces();
    free_heap(pq_events);

    free(points);

    g_list_free(circles_set);
    g_list_free(voronoi_edges_set);
    g_list_free(voronoi_vertices_set);
    g_list_free(voronoi_faces_set);
}
