#ifndef _LIB_MAP_H
#define _LIB_MAP_H
 
#include <stdarg.h>
#include <assert.h>
#include <lib/set.h>

/* 
 * Ordered Map
 * it's supposed to be slower than the unordered map, but it's actually faster
 * at insertion and retrieval. So use this for now until a better hashtable implementation
 * avails itself.
 */

#define map_key_t_int(m)   (m)->s.type = SET_TYPE_INT
#define map_key_t_dbl(m)   (m)->s.type = SET_TYPE_DOUBLE
#define map_key_t_str(m)   (m)->s.type = SET_TYPE_STRING
#define map_key_t_ptr(m)   (m)->s.type = SET_TYPE_POINTER
#define map_val_t_int(m)   (m)->val_type = MAP_VAL_TYPE_INT
#define map_val_t_dbl(m)   (m)->val_type = MAP_VAL_TYPE_DOUBLE
#define map_val_t_ptr(m)   (m)->val_type = MAP_VAL_TYPE_POINTER

typedef union _map_datum {
    void * p;
    int i;
    double d;
} map_datum_t;

typedef enum {
    MAP_VAL_TYPE_INT,
    MAP_VAL_TYPE_DOUBLE,
    MAP_VAL_TYPE_POINTER
} map_val_type_t;

typedef struct _map_node_t {
    rb_node_t n;
    map_datum_t val;
} map_node_t;

typedef struct _map_t {
	set_t s; /* inherit the set_t */
    map_val_type_t val_type;
} map_t;

/*
 * Dynamically allocates a new map
 * destroy it with map_destroy()
 */
map_t * map_create();
void map_init(map_t *);
void map_free(map_t *);
void map_destroy(map_t *);

#define map_key_type(m)    (m)->s.type
#define map_key_cmp(m, kc) (m)->s.el_cmp = kc

/* for debugging */
/* void map_print(map_t * _this, FILE * stream); */

/*
 * Adds an element to the map. returns 0 if the key already exists
 */
int map_add(map_t *, ...); /* map_add(map, key, val); returns 1 if found (updated) 0 if not */
int map_get(map_t *, ...); /* map_get(map, key, &val) returns 1 if found 0 if not */
int map_has(map_t *, ...); /* map_has(map, key) returns 1 if found 0 if not */
int map_del(map_t *, ...); /* map_del(map, key) returns 1 if found 0 if not */

/*
 * set the key and value types by passing in string of the
 * following form: key->val
 * key can be: i,d,s or p
 * val can be: i,d or p
 * e.g.
 * map_set_types(m, "s->p")  string and pointer 
 * map_set_types(m, "p->i")  pointer and int
 */
void map_set_types(map_t *, const char * type_s);

#define map_count(m)    set_count(m)

/* Iteration methods */
#define map_reset(m)    set_reset(m)
#define map_valid(m)    set_valid(m)
#define map_next(m)     set_next(m)
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

