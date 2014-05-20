#ifndef _LIB_MAP_H
#define _LIB_MAP_H
 
#include <stdarg.h>
#include <assert.h>

/* 
 * Ordered Map
 * it's supposed to be slower than the unordered map, but it's actually faster
 * at insertion and retrieval. So use this for now until a better hashtable implementation
 * avails itself.
 */

#define map_key_t_int(m)   m->key_type = MAP_KEY_TYPE_INT
#define map_key_t_dbl(m)   m->key_type = MAP_KEY_TYPE_DOUBLE
#define map_key_t_str(m)   m->key_type = MAP_KEY_TYPE_STRING
#define map_val_t_int(m)   m->val_type = MAP_VAL_TYPE_INT
#define map_val_t_dbl(m)   m->val_type = MAP_VAL_TYPE_DOUBLE
#define map_val_t_ptr(m)   m->val_type = MAP_VAL_TYPE_POINTER

typedef union _map_datum {
    void * p;
    int i;
    double d;
} map_datum_t;

typedef enum {
    MAP_KEY_TYPE_INT,
    MAP_KEY_TYPE_DOUBLE,
    MAP_KEY_TYPE_STRING
} map_key_type_t;

typedef enum {
    MAP_VAL_TYPE_INT,
    MAP_VAL_TYPE_DOUBLE,
    MAP_VAL_TYPE_POINTER
} map_val_type_t;

typedef enum {
	RB_COLOR_BLACK,
	RB_COLOR_RED
} rb_color_t;

struct _rb_node {
	map_datum_t key,
	                 value;

	rb_color_t color;
	
	struct _rb_node * parent,
                    * left,
                    * right;
};

typedef struct _rb_node rb_node_t;

typedef struct {
	int count;
	rb_node_t * root,
	          * iter_p,
	          nil;

	map_key_type_t key_type;
    map_val_type_t val_type;
    
    int (*key_cmp)(const void *, const void *);
} map_t;

/*
 * Dynamically allocates a new map
 * destroy it with map_destroy()
 */
map_t * map_create();
void map_init(map_t *);
void map_destroy(map_t *);

/* for debugging */
/* void map_print(map_t * _this, FILE * stream); */

/*
 * Adds an element to the map. returns 0 if the key already exists
 */
int map_add(map_t *, ...); /* map_add(map, key, val); returns 1 if found (updated) 0 if not */
int map_get(map_t *, ...); /* map_get(map, key, &val) returns 1 if found 0 if not */
int map_has(map_t *, ...); /* map_has(map, key) returns 1 if found 0 if not */
int map_del(map_t *, ...); /* map_del(map, key) returns 1 if found 0 if not */

/* Iteration methods */
void    map_reset(map_t *);
#define map_valid(m) ((m)->iter_p != &(m)->nil)
void    map_next(map_t *);
void    map_cur(map_t *, ...); /* map_cur(map, &key, &value); */

/* map foreach accepts a map, key, and value pointer 
   i.e.
   map_key_t_str(m)
   map_val_t_ptr(m)
   ...
   char * key;
   void * p;
   map_foreach(m, &key, &p) */
#define map_foreach(map, keyp, valuep) assert(map), assert(keyp), assert(valuep);\
    for (map_reset(map); map_cur(map, keyp, valuep), map_valid(map); map_next(map))

#endif

