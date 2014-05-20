#ifndef _LIB_RANGEMAP_H
#define _LIB_RANGEMAP_H

#include "lib/map.h"
#include "lib/vector.h"

/* simple data structure that allows you to store data as ranges */

typedef struct _range_t {
    int min, max;
} range_t;

typedef struct _rangemap_t {
    map_t range_map;
    vector_t ranges;
} rangemap_t;

#define rangemap_type_int(m)   map_val_t_int((map_t *) m)
#define rangemap_type_dbl(m)   map_val_t_dbl((map_t *) m)
#define rangemap_type_ptr(m)   map_val_t_ptr((map_t *) m)

rangemap_t *    rangemap_create();
void            rangemap_init(rangemap_t *);
void            rangemap_free(rangemap_t *);
void            rangemap_destroy(rangemap_t *);

int rangemap_add(rangemap_t *, int, int, ...); /* rangemap_add(map, min, max , val); returns 1 if found (updated) 0 if not */
int rangemap_get(rangemap_t *, int, ...); /* rangemap_get(map, key, &val) returns 1 if found 0 if not */
int rangemap_has(rangemap_t *, int); /* rangemap_has(map, key) returns 1 if found 0 if not */

#define rangemap_count(rm)    map_count(rm)

/* Iteration methods */
#define rangemap_reset(m)           map_reset(m)
#define rangemap_valid(m)           map_valid(m)
#define rangemap_next(m)            map_next(m)
/*#define rangemap_cur(m, args...)    map_cur(m, __VA_ARGS__)*/

#define rangemap_foreach(rm, keyp, valp) map_foreach(rm, keyp, valp)

#endif
