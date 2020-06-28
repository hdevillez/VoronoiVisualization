#include "bbst.h"
#include "geometry.h"
#include <assert.h>
#include <math.h>

int max(int a, int b) {
    if(a < b) return b;
    else return a;
}

bbst * create_new_bbst() {
    bbst * new_bbst = malloc(sizeof(bbst));
    new_bbst->is_empty = true;
    new_bbst->root= NULL;
    return new_bbst;
}

node_t* create_new_node(node_t * parent) {
    node_t *new_node = malloc(sizeof(node_t));

    new_node->left = NULL; new_node->right = NULL;
    new_node->parent = parent;
    
    new_node->site_left = NULL; new_node->site_right = NULL;
    new_node->voronoi_edge = NULL;

    new_node->is_leaf = false;
    new_node->arc_point = NULL;
    new_node->circle = NULL;
    new_node->left_leaf = NULL;    
    new_node->right_leaf = NULL;    
    return new_node;
}

bool is_empty(bbst * tree) {
    return tree->is_empty;
}


void insert_site(bbst * tree, Point * p) {
    // note : we only insert node when tree is empty;
    node_t * new_node = create_new_node(NULL);
    
    new_node->is_leaf = true;
    new_node->arc_point = p; 

    tree->root = new_node;
    tree->is_empty = false;
}

float get_breakpoint(node_t * node, float sl) {
    Point * A = node->site_left;
    Point * B = node->site_right;

    float den_a = (2*(A->y-sl));
    float den_b = (2*(B->y-sl));

    if(den_a == den_b) 
        return (A->x + B->x) / 2;
    else if(den_a == 0)
        return A->x;
    else if(den_b == 0)
        return B->x; 

    float a = 1.0/den_a - 1.0/den_b;
    float b = 2*B->x/den_b - 2*A->x/den_a;
    float c = (A->x*A->x + A->y * A->y - sl*sl)/den_a - (B->x*B->x + B->y*B->y-sl*sl)/den_b;
    

    float delta = b*b - 4*a*c;

    float x1 = (-b - sqrt(delta))/(2*a);
    float x2 = (-b + sqrt(delta))/(2*a);


    if(A->y < B->y) {
        return fmax(x1, x2);
    }
    else {
        return fmin(x1, x2);
    }
}

node_t *  get_arc_above(node_t * root, Point * p) {
    if(root->is_leaf) {
        return root;
    }
    else {
        float x_star = get_breakpoint(root, p->y);
        if(p->x < x_star) 
            return get_arc_above(root->left, p);
        else if(p->x >= x_star) 
            return get_arc_above(root->right, p);
    }
}


node_t * replace_leaf(node_t * leaf, Point * new_site) {
    // return root of the replacing subtree 

    node_t * new_node_1 = create_new_node(leaf->parent);
    node_t * new_node_2 = create_new_node(new_node_1);
    
    new_node_1->parent = leaf->parent;
    if(leaf->parent != NULL) {
        node_t * p = leaf->parent;
        if(p->left == leaf) {
            p->left = new_node_1;
        };
        if(p->right == leaf) {
            p->right = new_node_1;
        }
    } 

    new_node_1->site_left = leaf->arc_point;
    new_node_1->site_right = new_site;

    new_node_2->site_left = new_site;
    new_node_2->site_right = leaf->arc_point;


    node_t * new_leaf_left = create_new_node(new_node_1);
    node_t * new_leaf_middle = create_new_node(new_node_2);
    node_t * new_leaf_right = create_new_node(new_node_2);

    new_leaf_left->is_leaf = true;  
    new_leaf_middle->is_leaf = true;  
    new_leaf_right->is_leaf = true;  

    new_leaf_left->arc_point = leaf->arc_point;  
    new_leaf_middle->arc_point = new_site;  
    new_leaf_right->arc_point = leaf->arc_point;  

    new_leaf_right->right_leaf = leaf->right_leaf;
    new_leaf_left->left_leaf = leaf->left_leaf;
    
    new_leaf_left->right_leaf = new_leaf_middle; 
    new_leaf_middle->right_leaf = new_leaf_right; 
    new_leaf_right->left_leaf = new_leaf_middle; 
    new_leaf_middle->left_leaf = new_leaf_left; 

    if(leaf->left_leaf != NULL) 
        leaf->left_leaf->right_leaf = new_leaf_left;
    if(leaf->right_leaf != NULL) 
        leaf->right_leaf->left_leaf = new_leaf_right;

    new_node_1->left = new_leaf_left;
    new_node_1->right = new_node_2;

    new_node_2->left = new_leaf_middle;
    new_node_2->right = new_leaf_right;

    if(leaf != NULL) {
        free(leaf);
        leaf = NULL;
    }

    return new_node_1;
}

void get_arc_junctions(node_t * arc, node_t ** j_1, node_t ** j_2) {

    assert(arc->is_leaf);

    *j_1 = arc->parent;
    bool is_left = false;
    if((*j_1)->left == arc) {
        is_left = true;
    }

    *j_2 = *j_1;
    while(true) {
        node_t * new_j_2 = (*j_2)->parent;
        assert(j_2 != NULL); 
        
        if(new_j_2->left == *j_2) {
            if(!is_left) {
                *j_2 = new_j_2;
                break;
            } 
        } 
        else {
            if(is_left) {
                *j_2 = new_j_2;
                break; 
            }
        }

        *j_2 = new_j_2;
    }
}

void delete_leaf(node_t * node, node_t * j_1, node_t * j_2) {
    
    // delete junctions and update tree

    bool is_left = false;
    if(j_1->left == node) {
        is_left = true;
    }
    
    node_t * other_subtree;
    if(is_left) {
        other_subtree = j_1->right;
    }
    else {
        other_subtree = j_1->left;
    }

    node_t * grand_parent = j_1->parent;
    if(grand_parent->left == j_1) {
        grand_parent->left = other_subtree;
    }
    else {
        grand_parent->right = other_subtree;
    }
    other_subtree->parent = grand_parent; 
    
    if(is_left) {
        j_2->site_right = j_1->site_right;
    }
    else {
        j_2->site_left =  j_1->site_left;
    }
    
    
    free(j_1);
    j_1 = NULL;



    // delete leaf 
    
    if(node->left_leaf != NULL) {
        node->left_leaf->right_leaf = node->right_leaf;
    }
    if(node->right_leaf != NULL) {
        node->right_leaf->left_leaf = node->left_leaf;
    }
    free(node);
    node = NULL;

}

void free_bbst_(node_t * node) {
    if(!node->is_leaf) {
        free_bbst_(node->left); 
        free_bbst_(node->right);
    }
    free(node);
    node = NULL;
}

void free_bbst(bbst * tree) {
    free_bbst_(tree->root);    
    free(tree);
    tree = NULL;
}

void print_tree_(node_t* node, int deep) {
    for(int i= 0; i < deep; i++) printf("\t"); 
    
    if(node->is_leaf) {
        printf("- LEAF (%f %f)\n", node->arc_point->x, node->arc_point->y); 
    }
    else {
        printf("- JUNCTION (%f %f) - (%f %f)\n", node->site_left->x, node->site_left->y, node->site_right->x, node->site_right->y); 
        print_tree_(node->left, deep+1);
        print_tree_(node->right, deep+1);
    } 
}

void print_tree(bbst * tree) {
    if(tree == NULL || tree->root == NULL) return;
    print_tree_(tree->root, 0); 
}

