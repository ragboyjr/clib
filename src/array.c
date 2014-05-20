#include "lib/array.h"

#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <assert.h>
#include <stdio.h>

static void array_init_data(array_t * this)
{
    this->b_size    = ARRAY_MIN_SIZE;
    this->data      = malloc(sizeof(void *) * ARRAY_MIN_SIZE);
    this->has_init  = 1;
}

array_t * array_create()
{
	array_t * this = malloc(sizeof(array_t));
	array_init(this);
	return this;
}
void array_init(array_t * this)
{
    this->data      = NULL,
	this->count     = 0,
	this->b_size    = 0,
	this->iter_idx  = 0,
	this->has_init  = 0;
}

void array_print(array_t * this)
{
    const char * fmt = "list {\n"
    "   size        = %d\n"
    "   b_size      = %d\n"
    "   has_init    = %d\n"
    "}\n";
    printf(fmt, this->count, this->b_size, this->has_init);
}              

void array_destroy(array_t * this)
{
	if (this->data)
		free(this->data);
		
	free(this);
}

void array_alloc(array_t * this, int size)
{
	if (size < ARRAY_MIN_SIZE)
		return;
		
	this->b_size = size;
	this->data = realloc(this->data, sizeof(void *) * this->b_size);
}

void array_push(array_t * this, void * el)
{
    if (!this->has_init)
        array_init_data(this);
    
	if (this->count == this->b_size)
	{
		this->b_size *= ARRAY_GROWTH_FACTOR;
		this->data = realloc(this->data, sizeof(void *) * this->b_size);
	}
	
	this->data[this->count] = el;
	
	this->count++;
}

void * array_top(array_t * this)
{
    if (this->count == 0)
        return NULL;
    
    return this->data[this->count - 1];
}

void * array_pop(array_t * this)
{
	if (this->count == 0)
		return NULL;

    this->count--;
	return this->data[this->count];
}

void array_compact(array_t * this)
{
	int i = 0;
	int j = 0;
	for (i = 0; i < this->count; i++)
	{
		if (this->data[i])
			continue;
		
		// we found a null spot, so now let's try to close this NULL gap
		// j needs to point to the next valid value
		j = 1;
		while(!this->data[i + j] && i + j < this->count)
			j++;
		
		if (j == this->count)
		{
			// if we reached the end, then we don't have to worry about moving data
			/*
			 * [1][2][3][4][ ][ ][ ]
			 * [1][2][3][4]
			 */
			this->count -= j;
			break;
		}

		// close the gap of NULL data
		/*
		 * Close the gap of NULL data
		 *
		 *
		 * [1][2][ ][ ][5][6][7]
		 * [1][2][5][6][7]
		 */
		memmove(this->data + i, this->data + i + j, sizeof(void *) * (this->count - (i + j)));
		this->count -= j;
	}
	
	unsigned int old_b_size = this->b_size;
	
	if (this->count < ARRAY_MIN_SIZE)
		this->b_size = ARRAY_MIN_SIZE;
	else
		this->b_size = this->count;
		
	if (old_b_size == this->b_size)
		return;
		
	realloc(this->data, sizeof(void *) * this->b_size);
}
