#include "lib/uset.h"

/* DON'T USE THE USET YET */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "lib/hash.h"

typedef union _uset_datum uset_datum_t;

static unsigned long hash_entry_key(uset_key_type_t, uset_datum_t);

static void uset_entry_init(uset_entry_t *, uset_key_type_t, uset_datum_t, uset_datum_t);

/* static void uset_attach_item_to_tail(uset_t *, uset_item_t *); */

static int uset_key_eql(uset_t *, uset_datum_t, uset_datum_t);
static int uset_entry_eql(void * e1, void * e2, void *);

/* inline */ static uset_datum_t uset_get_va_key(uset_t *, va_list);
/* inline */ static uset_datum_t uset_get_va_val(uset_t *, va_list);


uset_t * uset_create()
{
    uset_t * this = (uset_t *) malloc(sizeof(uset_t));
    
    uset_init(this);
    
    return this;
}

void uset_init(uset_t * this)
{
    hashtable_init(&this->ht, sizeof(uset_entry_t), uset_entry_eql, this);
    this->key_type  = UMAP_KEY_TYPE_STRING;
    this->val_type  = UMAP_VAL_TYPE_DATA;
}

void uset_add(uset_t * this, ...)
{
    va_list ap; /* arg pointer */
    uset_datum_t key,
                      val;
    uset_entry_t mi, 
                 * new_entry;

    /* grab the key and value*/    
    va_start(ap, this);

    key = uset_get_va_key(this, ap);
    val = uset_get_va_val(this, ap);
    
    va_end(ap);
    
    uset_entry_init(&mi, this->key_type, key, val);
    new_entry = hashtable_lookup_entry(&this->ht, &mi, HASHTABLE_LOOKUP_INSERT);
    
    /*
     * Attach to the linked list
     */
    /* uset_attach_item_to_tail(this, mi); */
        
    return;
}

int uset_get(uset_t * this, ...)
{
    va_list ap; /* arg pointer */
    uset_datum_t key,
                 * ret;
                 
    uset_entry_t mi,
                 * ht_entry;
                        
    /* grab the key and data return pointer */
    va_start(ap, this);
    
    key = uset_get_va_key(this, ap);
    ret = va_arg(ap, uset_datum_t *);
    
    va_end(ap);

    mi.key  = key;
    mi.hash = hash_entry_key(this->key_type, key);
    mi.is_occupied = 0;
    
    ht_entry = hashtable_lookup_entry(&this->ht, &mi, HASHTABLE_LOOKUP_SEARCH);
    
    if (ht_entry == NULL)
        return 0;
    
    if (ret)
        *ret = ht_entry->value;
    
    return 1;
}

void uset_free(uset_t * this)
{
    hashtable_free(&this->ht);

/*     uset_item_t * p,
                   * tmp;*/
    
    /* free the buckets */
/*    uset_free_buckets(this);*/
    
    /* free the linked list */
    /*p = this->head;
    
    while (p)
    {
        tmp = p;
        p = p->next;
        free(tmp);
    }*/
}
void uset_destroy(uset_t * this)
{
    uset_free(this);
    free(this);
}

/*
 * Attach a map item to the end of internal linked list
 */
/*static void uset_attach_item_to_tail(uset_t * this, uset_item_t * mi)
{
    if (this->head == NULL)
    {
        this->head = this->tail = mi;
    }
    else
    {
        this->tail->next = mi;
        mi->prev = this->tail;
        this->tail = mi;
    }
}*/


/* inline */ static uset_datum_t uset_get_va_key(uset_t * this, va_list ap)
{
    uset_datum_t key;
    
    switch (this->key_type)
    {
        case UMAP_KEY_TYPE_INT:
            key.i = va_arg(ap, int);
            break;
        case UMAP_KEY_TYPE_DOUBLE:
            key.d = va_arg(ap, double);
            break;
        case UMAP_KEY_TYPE_STRING:
            key.p = va_arg(ap, void *);
            break;
    }
    
    return key;
}
/* inline */ static uset_datum_t uset_get_va_val(uset_t * this, va_list ap)
{
    uset_datum_t val;
    
    switch (this->val_type)
    {
        case UMAP_VAL_TYPE_INT:
            val.i = va_arg(ap, int);
            break;
        case UMAP_VAL_TYPE_DOUBLE:
            val.d = va_arg(ap, double);
            break;
        case UMAP_VAL_TYPE_DATA:
            val.p = va_arg(ap, void *);
            break;
    }
    
    return val;
}

static int uset_key_eql(uset_t * this, uset_datum_t key1, uset_datum_t key2)
{
    switch (this->key_type)
    {
        case UMAP_KEY_TYPE_INT:
            if (key1.i == key2.i)
                return 0;
            else if (key1.i < key2.i)
                return -1;
            else
                return 1;
        case UMAP_KEY_TYPE_DOUBLE:
            if (key1.d == key2.d)
                return 0;
            else if (key1.d < key2.d)
                return -1;
            else
                return 1;
        case UMAP_KEY_TYPE_STRING:
            return strcmp(key1.p, key2.p);
    }
}

static unsigned long hash_entry_key(uset_key_type_t key_type, uset_datum_t key)
{
    switch (key_type)
    {
        case UMAP_KEY_TYPE_INT:
            return hash_bytes(&key, sizeof(int));
        case UMAP_KEY_TYPE_DOUBLE:
            return hash_bytes(&key, sizeof(double));
        case UMAP_KEY_TYPE_STRING:
            return hash_str(key.p);
    }
}

static int uset_entry_eql(void * e1, void * e2, void * this)
{
    return uset_key_eql(this, ((uset_entry_t * )e1)->key, ((uset_entry_t * )e2)->key);
}

static void uset_entry_init(uset_entry_t * mi, uset_key_type_t key_type, uset_datum_t key, umap_datum_t value)
{    
    mi->key         = key;
    mi->value       = value;
    mi->hash        = hash_entry_key(key_type, key);
    mi->is_occupied = 0;
}
