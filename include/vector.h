#ifndef _LIB_VECTOR_H
#define _LIB_VECTOR_H

#include <assert.h>

#define VECTOR_GROWTH_FACTOR 1.5
#define VECTOR_MIN_SIZE 4

typedef struct {
	void * data;
	unsigned int count,
                 b_size, /* buffer size */
                 iter_idx;
    unsigned short item_size;
	int has_init;
} vector_t;

vector_t * vector_create(int /* item_size */);
void vector_init(vector_t *, int);
void vector_free(vector_t *);
void vector_destroy(vector_t *);
void vector_alloc(vector_t *, int size);

#define vector_count(v) (v)->count

#define vector_add(v, el) vector_push(v, el)

void * vector_push(vector_t *);
int    vector_pop(vector_t *);
void * vector_top(vector_t *);

void vector_del(vector_t *, int /* idx */);
void * vector_get(vector_t *, int);

void vector_print(vector_t *);

#define vector_reset(v) (v)->iter_idx = 0
#define vector_valid(v) ((v)->iter_idx < vector_count(v))
#define vector_next(v)  (v)->iter_idx++

#define vector_foreach(v, el) assert(v);\
    for (vector_reset(v); el = vector_get(v, (v)->iter_idx),  vector_valid(v); vector_next(v))

#endif
