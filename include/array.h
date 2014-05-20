#ifndef _LIB_ARRAY_H
#define _LIB_ARRAY_H

#include <assert.h>

#define ARRAY_GROWTH_FACTOR 1.5
#define ARRAY_MIN_SIZE 4

typedef struct {
	void ** data;
	unsigned int count,
                 b_size, /* buffer size */
                 iter_idx;
	int has_init;
} array_t;

array_t * array_create();
void      array_init(array_t *);
void array_destroy(array_t * _this);
void array_alloc(array_t * _this, int size);

#define array_count(a) (a)->count

#define array_add(a, el) array_push(a, el)
void    array_push(array_t *, void *);
void *  array_pop(array_t *);
void *  array_top(array_t *);

#define array_set(a, i, el) (a)->data[i] = el
#define array_get(a, i)     (a)->data[i]
#define array_del(a, i)     (a)->data[i] = NULL

void array_compact(array_t * _this);
void array_print(array_t *);


#define array_reset(a) (a)->iter_idx = 0
#define array_valid(a) ((a)->iter_idx < array_count(a))
#define array_next(a)  (a)->iter_idx++

#define array_foreach(a, el) assert(a);\
    for (array_reset(a); el = array_get(a, (a)->iter_idx),  array_valid(a); array_next(a))

#endif
