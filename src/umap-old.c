#include "lib/umap.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "lib/hash.h"

#define umap_create_buckets(map, num) map->buckets = calloc(num, sizeof(bucket_t))

typedef union _umap_datum umap_datum_t;

static unsigned long hash_item_key(umap_key_type_t, umap_datum_t);

static umap_item_t * umap_item_create(umap_key_type_t, umap_datum_t, umap_datum_t);

static void umap_free_buckets(umap_t *);
static void umap_reallocate(umap_t *);
static void umap_rehash(umap_t *, bucket_t **, int);
static void umap_hash_item(umap_t *, umap_item_t *);
static void umap_attach_item_to_tail(umap_t *, umap_item_t *);

static int umap_key_eql(umap_t *, umap_datum_t, umap_datum_t);

void print_bucket_stats(umap_t *);
void print_buckets(umap_t *);

/* inline */ static umap_datum_t umap_get_va_key(umap_t *, va_list);
/* inline */ static umap_datum_t umap_get_va_val(umap_t *, va_list);


static unsigned long prime_doubles[] = {
    5,
    11,
    23,
    47,
    97,
    197,
    397,
    797,
    1597,
    3203,
    6421,
    12853,
    25717,
    51437,
    102877,
    205759,
    411527,
    823117,
    1646237,
    3292489,
    6584983,
    13169977,
    26339969,
    52679969,
    105359939,
    210719881,
    421439783,
    842879579,
    1685759167,
    3371518343,
    6743036717,
    13486073473,
    26972146961,
    53944293929,
    107888587883,
    215777175787,
    431554351609,
    863108703229,
    1726217406467,
    3452434812973,
    6904869625999,
    13809739252051,
    27619478504183,
    55238957008387,
    110477914016779,
    220955828033581,
    441911656067171,
    883823312134381,
    1767646624268779,
    3535293248537579
};


umap_t * umap_create()
{
    umap_t * this = (umap_t *) malloc(sizeof(umap_t));
    
    umap_init(this);
    
    return this;
}

void umap_init(umap_t * this)
{
    this->buckets   = NULL;
    this->size      = 0;
    this->max       = 0;
    this->key_type  = UMAP_KEY_TYPE_STRING;
    this->val_type  = UMAP_VAL_TYPE_DATA;
}

void umap_add(umap_t * this, ...)
{
    va_list ap; /* arg pointer */
    umap_datum_t key,
                      val;
    umap_item_t * mi;

    /* grab the key and value*/    
    va_start(ap, this);

    key = umap_get_va_key(this, ap);
    val = umap_get_va_val(this, ap);
    
    va_end(ap);
    
    /* create the map item */
    mi = umap_item_create(this->key_type, key, val);
    
    assert(mi); /* TODO - proper error handling */

    /* add to the hash table */
    umap_hash_item(this, mi);
    
    /*
     * Attach to the linked list
     * This NEEDS to be done after adding the item to the table
     * if the hash requires a reallocation, we loop through the 
     * internal linked list and re-insert all of the values.
     * To avoid adding mi twice in that case, we just add it to the internal
     * list after we hash.
     */
    //umap_attach_item_to_tail(this, mi);
    this->size++;
    
    return;
}

int umap_get(umap_t * this, ...)
{
    va_list ap; /* arg pointer */
    umap_datum_t key,
                 * ret;
    umap_item_t * mi;
    unsigned long hash;
    int idx;
    bucket_t * bucket;
        
    /* grab the key and data return pointer */
    va_start(ap, this);
    
    key = umap_get_va_key(this, ap);
    ret = va_arg(ap, umap_datum_t *);
    
    va_end(ap);

    hash    = hash_item_key(this->key_type, key);
    idx     = hash % this->max;
    
    //bucket = this->buckets + idx;
        
    /*list_foreach_p(bucket, mi)
    {
        if (umap_key_eql(this, mi->key, key))
        {
            if (ret)
            {
                *ret = mi->value;
            }

            return 1;
        }
    }*/
    
    return 0;
}

void umap_free(umap_t * this)
{
    umap_item_t * p,
                * tmp;
    
    /* free the buckets */
    umap_free_buckets(this);
    
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
    //umap_free(this);
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

/*
 * Actually perform the insertion into the table 
 */
static void umap_hash_item(umap_t * this, umap_item_t * mi)
{
    int idx;

    if (this->size == this->max)
    {
        /* reallocate and rehash the table */
        umap_reallocate(this);
    }

    assert(this->size < this->max);
    
    idx = mi->hash % this->max;
    array_add(this->buckets[idx], mi);
    //list_push_p((this->buckets + idx), mi);
}

static void umap_reallocate(umap_t * this)
{
    bucket_t ** tmp_buckets;
    int i;
    int new_max = (this->max) ? this->max * UMAP_GROWTH_FACTOR : UMAP_INITIAL_SIZE;

    /* create the new buckets */
    tmp_buckets = calloc(new_max, sizeof(bucket_t *));
    
    for (i = 0; i < new_max; i++)
        tmp_buckets[i] = array_create();
    
    umap_rehash(this, tmp_buckets, new_max);
    
    /* free the buckets */
    umap_free_buckets(this);
    
    this->max = new_max;
    this->buckets = tmp_buckets;
}

static void umap_rehash(umap_t * this, bucket_t ** tmp_buckets, int new_max)
{
    int idx, i, j;
    umap_item_t * mi;
    bucket_t * bucket;
    
    for (i = 0; i < this->max; i++)
    {
        bucket = this->buckets[i];
        for (j = 0; j < bucket->size; j++)
        {
            mi = bucket->data[j];
            idx = mi->hash % new_max;
            array_add(tmp_buckets[idx], mi);
        }
    }
    
    // mi = this->head;
//     while (mi)
//     {
//         idx = mi->hash % this->max; /* re calculate each items index */
//         list_push_p(this->buckets + idx, mi); /* push onto the bucket */
//         
//         mi = mi->next;
//     }
}

static void umap_free_buckets(umap_t * this)
{
    int i;
    
    /* free the buckets */
    if (this->buckets)
    {
        for (i = 0; i < this->max; i++)
            array_destroy(this->buckets[i]);
            
        free(this->buckets);
    }
}

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
            return key1.i == key2.i;
        case UMAP_KEY_TYPE_DOUBLE:
            return key1.d == key2.d;
        case UMAP_KEY_TYPE_STRING:
            return strcmp(key1.p, key2.p) == 0;
    }
}

static unsigned long hash_item_key(umap_key_type_t key_type, umap_datum_t key)
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

void print_bucket_stats(umap_t * this)
{
    
}

void print_buckets(umap_t * this)
{
    int i;
    
    puts("printing buckets");
    for (i = 0; i < this->max; i++)
    {
        /*list_print(this->buckets + i);*/
        array_print(this->buckets[i]);
    }
}

/* umap_item functions */
static umap_item_t * get_malloced_node()
{
    return (umap_item_t *) malloc(sizeof(umap_item_t));
}

static umap_item_t * get_new_node()
{
    static int cur_idx = 0;
    static umap_item_t * nodes = NULL;
    umap_item_t * ret;
    
    if (nodes == NULL)
        nodes = malloc(sizeof(umap_item_t) * 10000);
    
    ret = nodes + cur_idx;
    cur_idx++;
    return ret;
}

static umap_item_t * umap_item_create(umap_key_type_t key_type, umap_datum_t key, umap_datum_t value)
{
    umap_item_t * mi = get_malloced_node();
    
    if (mi == NULL)
        return mi;
    
    mi->key     = key;
    mi->value   = value;
    mi->hash    = hash_item_key(key_type, key);
    
    return mi;
}

