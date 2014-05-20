#include "lib/set.h"
#include "lib/set/common.h"
#include <stdlib.h>

set_t * set_create()
{
    set_t * this = malloc(sizeof(set_t));
    
    if (this == NULL)
        return NULL;
        
    set_init(this);
    
    return this;
}

void set_init(set_t * this)
{
    /* init the values */
    rb_tree_init(&this->t, sizeof(rb_node_t), set_el_cmp, this);
    
    this->type = SET_TYPE_STRING;
}

void set_free(set_t * this)
{
    /* free the nodes */
    rb_tree_free(&this->t);
}

void set_destroy(set_t * this)
{
    set_free(this);
    free(this);
}

int set_add(set_t * this, ...)
{
    va_list ap; /* arg pointer */
    rb_key_u key;
    rb_node_t * n = NULL;
    int was_inserted;

    /* grab the key and value*/    
    va_start(ap, this);
    key = set_get_va_key(this, ap);
    va_end(ap);
    
    n = rb_tree_insert(&this->t, key, &was_inserted);
    
    return !was_inserted; /* return if node was found and updated, 0 if inserted */
}

int set_has(set_t * this, ...)
{
    va_list ap; /* arg pointer */
    rb_key_u key;

    /* grab the key and data return pointer */
    va_start(ap, this);
    key = set_get_va_key(this, ap);
    va_end(ap);
    
    return rb_tree_search(&this->t, key) ? 1 : 0;
}

int set_del(set_t * this, ...)
{
    va_list ap; /* arg pointer */
    rb_key_u key;
    rb_node_t * n;
                                         
    /* grab the key and data return pointer */
    va_start(ap, this);
    key = set_get_va_key(this, ap);
    va_end(ap);
    
    n = rb_tree_delete(&this->t, key);
    
    if (!n) {
        return 0;
    }
    
    /* free n */
    free(n);
    
    return 1;
}

void set_cur(set_t * this, ...)
{
    va_list ap; /* arg pointer */
    void * key_ret;
    
    /* map_valid should be called before making a call to this */
    if (!set_valid(this)) {
        return;
    }
                                                 
    /* grab the key return pointer */
    va_start(ap, this);
    key_ret = va_arg(ap, void *);    
    va_end(ap);
        
    set_set_key_p(this, this->t.iter_p->key, key_ret);
}
