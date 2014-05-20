#include "lib/umap.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "lib/hash.h"

typedef union _umap_datum umap_datum_t;

static unsigned long hash_entry_key(umap_key_type_t, umap_datum_t);

static void umap_entry_init(umap_entry_t *, umap_key_type_t, umap_datum_t, umap_datum_t);

/* static void umap_attach_item_to_tail(umap_t *, umap_item_t *); */

static int umap_key_eql(umap_t *, umap_datum_t, umap_datum_t);
static int umap_entry_eql(void * e1, void * e2, void *);

/* inline */ static umap_datum_t umap_get_va_key(umap_t *, va_list);
/* inline */ static umap_datum_t umap_get_va_val(umap_t *, va_list);


umap_t * umap_create()
{
    umap_t * this = (umap_t *) malloc(sizeof(umap_t));
    
    umap_init(this);
    
    return this;
}

void umap_init(umap_t * this)
{
    hashtable_init(&this->ht, sizeof(umap_entry_t), umap_entry_eql, this);
    this->key_type  = UMAP_KEY_TYPE_STRING;
    this->val_type  = UMAP_VAL_TYPE_DATA;
}

void umap_add(umap_t * this, ...)
{
    va_list ap; /* arg pointer */
    umap_datum_t key,
                 val;
    umap_entry_t mi, 
                 * new_entry;

    /* grab the key and value*/    
    va_start(ap, this);

    key = umap_get_va_key(this, ap);
    val = umap_get_va_val(this, ap);
    
    va_end(ap);
    
    umap_entry_init(&mi, this->key_type, key, val);
    new_entry = hashtable_lookup_entry(&this->ht, &mi, HASHTABLE_LOOKUP_INSERT);
    
    /*
     * Attach to the linked list
     */
    /* umap_attach_item_to_tail(this, mi); */
        
    return;
}

int umap_get(umap_t * this, ...)
{
    va_list ap; /* arg pointer */
    umap_datum_t key,
                 * ret;
                 
    umap_entry_t mi,
                 * ht_entry;
                        
    /* grab the key and data return pointer */
    va_start(ap, this);
    
    key = umap_get_va_key(this, ap);
    ret = va_arg(ap, umap_datum_t *);
    
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

void umap_free(umap_t * this)
{
    hashtable_free(&this->ht);

/*     umap_item_t * p,
                   * tmp;*/
    
    /* free the buckets */
/*    umap_free_buckets(this);*/
    
    /* free the linked list */
    /*p = this->head;
    
    while (p)
    {
        tmp = p;
        p = p->next;
        free(tmp);
    }*/
}
void umap_destroy(umap_t * this)
{
    umap_free(this);
    free(this);
}

/*
 * Attach a map item to the end of internal linked list
 */
/*static void umap_attach_item_to_tail(umap_t * this, umap_item_t * mi)
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


/* inline */ static umap_datum_t umap_get_va_key(umap_t * this, va_list ap)
{
    umap_datum_t key;
    
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
/* inline */ static umap_datum_t umap_get_va_val(umap_t * this, va_list ap)
{
    umap_datum_t val;
    
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

static int umap_key_eql(umap_t * this, umap_datum_t key1, umap_datum_t key2)
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

static unsigned long hash_entry_key(umap_key_type_t key_type, umap_datum_t key)
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

static int umap_entry_eql(void * e1, void * e2, void * this)
{
    return umap_key_eql(this, ((umap_entry_t * )e1)->key, ((umap_entry_t * )e2)->key);
}

static void umap_entry_init(umap_entry_t * mi, umap_key_type_t key_type, umap_datum_t key, umap_datum_t value)
{    
    mi->key         = key;
    mi->value       = value;
    mi->hash        = hash_entry_key(key_type, key);
    mi->is_occupied = 0;
}
