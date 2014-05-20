#ifndef _LIB_SET_H
#define _LIB_SET_H
 
#include <stdarg.h>
#include <assert.h>
#include "lib/rbtree.h"

/*
 * Ordered Set
 */
#define set_t_int(s)   (s)->type = SET_TYPE_INT
#define set_t_dbl(s)   (s)->type = SET_TYPE_DOUBLE
#define set_t_str(s)   (s)->type = SET_TYPE_STRING
#define set_t_ptr(s)   (s)->type = SET_TYPE_POINTER

typedef enum {
    SET_TYPE_INT,
    SET_TYPE_DOUBLE,
    SET_TYPE_STRING,
    SET_TYPE_POINTER
} set_type_t;

typedef struct _set_t {
	rb_tree_t t; /* inherit the rbtree_t */
	set_type_t type;
    
    int (*el_cmp)(const void *, const void *);
} set_t;

/*
 * Dynamically allocates a new set
 * destroy it with set_destroy()
 */
set_t * set_create();
void set_init(set_t *);
void set_free(set_t *);
void set_destroy(set_t *);

int set_add(set_t *, ...); /* map_add(map, el) returns 1 if found (updated) 0 if not */
int set_has(set_t *, ...); /* map_has(map, el) returns 1 if found 0 if not */
int set_del(set_t *, ...); /* map_del(map, el) returns 1 if found 0 if not */

#define set_count(s)    ((rb_tree_t *) s)->count

/* Iteration methods */
#define set_reset(s)    rb_tree_reset((rb_tree_t *) s)
#define set_valid(s)    rb_tree_valid((rb_tree_t *) s)
#define set_next(s)     rb_tree_next((rb_tree_t *) s)
void    set_cur(set_t *, ...); /* set_cur(map, &el); */

/* set foreach accepts a set and element pointer 
   i.e.
   set_t_str(s)
   ...
   char * el;
   set_foreach(s, &el) */
#define set_foreach(set, elp) assert(set), assert(elp);\
    for (set_reset(set); set_cur(set, elp), set_valid(set); set_next(set))

#endif

