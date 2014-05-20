#ifndef _LIB_RBTREE_H
#define _LIB_RBTREE_H

typedef enum {
	RB_COLOR_BLACK,
	RB_COLOR_RED
} rb_color_t;

typedef union {
    void * p;
    int    i;
    double d;
} rb_key_u;

typedef struct _rb_node_t {
    rb_color_t color;
    
    rb_key_u key;
	
	struct _rb_node_t * parent,
                      * left,
                      * right;
} rb_node_t;

typedef struct _rb_tree_t {
	rb_node_t * root,
	          * iter_p,
	          nil;
	int count, /* num elements in the tree */
	    node_size; /* size of the nodes to allocate */
	
	void * key_cmp_state;
    int (*key_cmp)(const rb_key_u, const rb_key_u, void *);
} rb_tree_t;

void rb_tree_init(
    rb_tree_t *,                                        /* this */
    int,                                                /* node size */
    int (*)(const rb_key_u, const rb_key_u, void *),    /* key_cmp */
    void *                                              /* key_cmp_state */
);
void rb_tree_free(rb_tree_t *);

rb_node_t * rb_tree_insert(rb_tree_t *, const rb_key_u, int * /* was inserted */); /* returns the node created or the node found */
rb_node_t * rb_tree_search(rb_tree_t *, const rb_key_u); /* returns the node found or null */
rb_node_t * rb_tree_delete(rb_tree_t *, const rb_key_u); /* returns the node deleted or null */

void    rb_tree_reset(rb_tree_t *);
void    rb_tree_next(rb_tree_t *);
#define rb_tree_valid(t)    ((t)->iter_p != &(t)->nil)
#define rb_tree_cur(t)      (t)->iter_p

#endif
