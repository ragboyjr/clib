#include "lib/map.h"
#include "lib/set/common.h"
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <stdio.h>

/* helper functions and macros */

static map_datum_t map_get_va_val(map_t * this, va_list ap);
static void map_set_val_p(map_t * this, map_datum_t data, void * val);

map_t * map_create()
{
    map_t * this = malloc(sizeof(map_t));
    
    if (this == NULL)
        return NULL;
        
    map_init(this);
    
    return this;
}

void map_init(map_t * this)
{
    /* init the values */
    set_init(&this->s);
    this->s.t.node_size = sizeof(map_node_t);    
    this->val_type  = MAP_VAL_TYPE_POINTER;
}

void map_free(map_t * this)
{
    set_free(&this->s);
}

void map_destroy(map_t * this)
{
    /* free the nodes */
    map_free(this);
    free(this);
}

void map_set_types(map_t * this, const char * type_s)
{
    char key_c, val_c;
    int num_scanned;
    
    num_scanned = sscanf(type_s, "%c->%c", &key_c, &val_c);
    
    assert(num_scanned == 2);
    
    switch (key_c)
    {
        case 'i':
            map_key_t_int(this);
            break;
        case 'd':
            map_key_t_dbl(this);
            break;
        case 's':
            map_key_t_str(this);
            break;
        case 'p':
            map_key_t_ptr(this);
            break;
        default:
            assert(0);
    }
    
    switch (val_c)
    {
        case 'i':
            map_val_t_int(this);
            break;
        case 'd':
            map_val_t_dbl(this);
            break;
        case 'p':
            map_val_t_ptr(this);
            break;
        default:
            assert(0);
    }
}

/*void map_print(map_t * this, FILE * stream)
{
    print_tree(this, this->root, stream);   
}*/

int map_add(map_t * this, ...)
{
    va_list ap; /* arg pointer */
    rb_key_u key;
    map_datum_t val;
    map_node_t * n = NULL;
    int was_inserted;

    /* grab the key and value*/    
    va_start(ap, this);

    key = set_get_va_key(&this->s, ap);
    val = map_get_va_val(this, ap);
    
    va_end(ap);
    
    n = (map_node_t *) rb_tree_insert(&this->s.t, key, &was_inserted);
    
    n->val = val;
    
    return !was_inserted; /* return if node was found and updated, 0 if inserted */
}

int map_get(map_t * this, ...)
{
    va_list ap; /* arg pointer */
    rb_key_u key;
    void * ret;
    map_node_t * n;
                                         
    /* grab the key and data return pointer */
    va_start(ap, this);
    
    key = set_get_va_key(&this->s, ap);
    ret = va_arg(ap, void *);
    
    va_end(ap);
    
    n = (map_node_t *) rb_tree_search(&this->s.t, key);
    
    if (!n) {
        return 0;
    }
    
    /* node was found */
    if (ret) {
        map_set_val_p(this, n->val, ret);
    }
    
    return 1;
}

int map_has(map_t * this, ...)
{
    va_list ap; /* arg pointer */
    rb_key_u key;

    /* grab the key and data return pointer */
    va_start(ap, this);
    
    key = set_get_va_key(&this->s, ap);
    
    va_end(ap);
    
    return rb_tree_search(&this->s.t, key) ? 1 : 0;
}

int map_del(map_t * this, ...)
{
    va_list ap; /* arg pointer */
    rb_key_u key;
    map_node_t * n;
                                         
    /* grab the key and data return pointer */
    va_start(ap, this);
    
    key = set_get_va_key(&this->s, ap);
    
    va_end(ap);
    
    n = (map_node_t *) rb_tree_delete(&this->s.t, key);
    
    if (!n) {
        return 0;
    }
    
    /* free n */
    free(n);
    
    return 1;
}

void map_cur(map_t * this, ...)
{
    va_list ap; /* arg pointer */
    void * key_ret,
         * val_ret;
    
    /* map_valid should be called before making a call to this */
    if (!map_valid(this)) {
        return;
    }
                                                 
    /* grab the key and data return pointer */
    va_start(ap, this);
    
    key_ret = va_arg(ap, void *);
    val_ret = va_arg(ap, void *);
    
    va_end(ap);
        
    set_set_key_p(&this->s, this->s.t.iter_p->key, key_ret);
    map_set_val_p(this, ((map_node_t *) this->s.t.iter_p)->val, val_ret);
}

static void map_set_val_p(map_t * this, map_datum_t data, void * val)
{
    switch (this->val_type)
    {
        case MAP_VAL_TYPE_INT:
            *((int *) val) = data.i;
            break;
        case MAP_VAL_TYPE_DOUBLE:
            *((double *) val) = data.d;
            break;
        case MAP_VAL_TYPE_POINTER:
            *((void **) val) = data.p;
            break;
    }
}
static map_datum_t map_get_va_val(map_t * this, va_list ap)
{
    map_datum_t val;
    
    switch (this->val_type)
    {
        case MAP_VAL_TYPE_INT:
            val.i = va_arg(ap, int);
            break;
        case MAP_VAL_TYPE_DOUBLE:
            val.d = va_arg(ap, double);
            break;
        case MAP_VAL_TYPE_POINTER:
            val.p = va_arg(ap, void *);
            break;
    }
    
    return val;
}
