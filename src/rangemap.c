#include "lib/rangemap.h"
#include <stdlib.h>

/* we have two different comparisons depending on if we are inserting a range or if
   we are looking for one value in a range */
static int range_cmp(const void * _arg1, const void * _arg2)
{
    const range_t * r1 = _arg1,
                  * r2 = _arg2;
                  
    if (r1->min == r2->min && r1->max == r2->max) {
        return 0; /* ranges are equal */
    }
    else if (r1->min < r2->min) {
        return -1;
    }
    else if (r1->min > r2->min) {
        return 1;
    } /* the mins are equal at this point */
    else if (r1->max < r2->max) {
        return -1;
    }
    else if (r1->max > r2->max) {
        return 1;
    }
    
    assert(0); /* i should never reach this code */
}

static int range_single_cmp(const void * _arg1, const void * _arg2)
{
    const range_t * r1 = _arg1,
                  * r2 = _arg2;
    int val = r2->min; /* r2 is just used to store a single val, max isn't used */
    
    /* val is in range */
    if (r1->min <= val && val <= r1->max) {
        return 0;
    }
    else if (r1->min > val) {
        return 1;
    }
    else if (r1->max < val) {
        return -1;
    }
    
    assert(0); /* i should never reach this code */
}

rangemap_t * rangemap_create()
{
    rangemap_t * this = malloc(sizeof(rangemap_t));
    rangemap_init(this);
    return this;
}
void rangemap_init(rangemap_t * this)
{
    map_init(&this->range_map);
    map_key_t_ptr(&this->range_map);
    vector_init(&this->ranges, sizeof(range_t));
}
void rangemap_free(rangemap_t * this)
{
    map_free(&this->range_map);
    vector_free(&this->ranges);
}
void rangemap_destroy(rangemap_t * this)
{
    rangemap_free(this);
    free(this);
}

int rangemap_add(rangemap_t * this, int min, int max, ...)
{
    /* create a new range */
    va_list ap;
    int ret;
    range_t * range = vector_push(&this->ranges);
    
    range->min = min;
    range->max = max;
    
    va_start(ap, max);
    
    /* make sure we are using the range comparison */
    map_key_cmp(&this->range_map, range_cmp);
    
    switch (this->range_map.val_type)
    {
        case MAP_VAL_TYPE_INT:
            ret = map_add(&this->range_map, range, va_arg(ap, int));
            break; 
        case MAP_VAL_TYPE_DOUBLE:
            ret = map_add(&this->range_map, range, va_arg(ap, double));
            break;
        case MAP_VAL_TYPE_POINTER:
            ret = map_add(&this->range_map, range, va_arg(ap, void *));
            break;
    }
    
    /* if true, then the range already exists, so we can pop this range off */
    if (ret) {
        vector_pop(&this->ranges);
    }
    
    va_end(ap);
    
    return ret;
}

int rangemap_get(rangemap_t * this, int val, ...)
{
    va_list ap;
    int ret;
    range_t range;
    range.min = val;
        
    /* make sure we are using the range comparison */
    map_key_cmp(&this->range_map, range_single_cmp);
    
    va_start(ap, val);
    ret = map_get(&this->range_map, &range, va_arg(ap, void *));
    va_end(ap);
    
    return ret;
}

int rangemap_has(rangemap_t * this, int val)
{
    int ret;
    range_t range; range.min = val;
        
    /* make sure we are using the range comparison */
    map_key_cmp(&this->range_map, range_single_cmp);
    
    return map_has(&this->range_map, &range);    
}
