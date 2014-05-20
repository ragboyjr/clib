#include "lib/vector.h"

#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <assert.h>
#include <stdio.h>

vector_t * vector_create(int item_size)
{
	vector_t * this = malloc(sizeof(vector_t));
    vector_init(this, item_size);
	return this;
}

void vector_init(vector_t * this, int item_size)
{
    this->data      = NULL,
	this->count     = 0,
	this->b_size    = 0,
	this->iter_idx  = 0,
	this->item_size = item_size,
	this->has_init  = 0;
}

static void vector_init_data(vector_t * this)
{
    this->b_size    = VECTOR_MIN_SIZE;
    this->data      = malloc(this->item_size * VECTOR_MIN_SIZE);
    this->has_init  = 1;
}

static void vector_check_resize(vector_t * this)
{
    if (this->count == this->b_size)
	{
		this->b_size *= VECTOR_GROWTH_FACTOR;
		this->data = realloc(this->data, this->item_size * this->b_size);
	}
}

void vector_print(vector_t * this)
{
    const char * fmt = "list {\n"
    "   size        = %d\n"
    "   b_size      = %d\n"
    "   has_init    = %d\n"
    "}\n";
    printf(fmt, this->count, this->b_size, this->has_init);
}              

void vector_free(vector_t * this)
{
    if (this->data) {
        free(this->data);
    }
}

void vector_destroy(vector_t * this)
{
	vector_free(this);
	free(this);
}

void vector_alloc(vector_t * this, int size)
{
	if (size < VECTOR_MIN_SIZE)
		return;
		
	this->b_size = size;
	this->data = realloc(this->data, this->item_size * this->b_size);
}

void * vector_push(vector_t * this)
{
    if (!this->has_init)
        vector_init_data(this);
    
	vector_check_resize(this);
		
	this->count++;
	return vector_top(this);
}

void * vector_top(vector_t * this)
{
    if (this->count == 0)
        return NULL;

    return vector_get(this, this->count - 1);
}

int vector_pop(vector_t * this)
{
	if (this->count == 0)
		return 0;

    this->count--;
    vector_check_resize(this);
    return 1;
}

void vector_del(vector_t * this, int idx)
{
    void * dst, * src;
    int size;

    if (this->count == 0)
        return;
    
    /* delete the current entry by shifting everything after it left one position */
    dst = vector_get(this, idx),
    src = vector_get(this, idx + 1);
    size = this->count * this->item_size - idx * this->item_size;
    
    memmove(dst, src, size);
    this->count--;
}
void * vector_get(vector_t * this, int idx)
{
     return (char *) this->data + idx * this->item_size;
}
