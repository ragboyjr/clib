#ifndef _LIB_UMAP_H
#define _LIB_UMAP_H

#include "lib/hashtable.h"
#include <stdarg.h>

/*
 * Unordered Map implemented by a hash table.
 */

#define umap_key_t_int(m)   m->key_type = UMAP_KEY_TYPE_INT
#define umap_key_t_dbl(m)   m->key_type = UMAP_KEY_TYPE_DOUBLE
#define umap_key_t_str(m)   m->key_type = UMAP_KEY_TYPE_STRING
#define umap_val_t_int(m)   m->val_type = UMAP_VAL_TYPE_INT
#define umap_val_t_dbl(m)   m->val_type = UMAP_VAL_TYPE_DOUBLE
#define umap_val_t_data(m)  m->val_type = UMAP_VAL_TYPE_DATA

union _umap_datum {
    void * p;
    int i;
    double d;
};

typedef enum {
    UMAP_KEY_TYPE_INT,
    UMAP_KEY_TYPE_DOUBLE,
    UMAP_KEY_TYPE_STRING
} umap_key_type_t;

typedef enum {
    UMAP_VAL_TYPE_INT,
    UMAP_VAL_TYPE_DOUBLE,
    UMAP_VAL_TYPE_DATA
} umap_val_type_t;

/* this needs to share the same structure as defined in hashtable.h */
typedef struct _umap_entry {
    int is_occupied;
    unsigned long hash;
    struct _umap_entry * left,
                       * right,
                       * next;
    union _umap_datum key,
                      value;
} umap_entry_t;

typedef struct {
    hashtable_t ht;
    
    umap_key_type_t key_type;
    umap_val_type_t val_type;
} umap_t;

umap_t * umap_create();
void umap_init(umap_t *);

/*
 * add values to the unordered map. The parameters are
 * umap_t, key, value. The key and value will be parsed as whatever is the current
 * types are defined in the umap
 */
void umap_add(umap_t *, ...);

/*
 * accepts the umap, key, then a pointer for the data to be stored.
 * i.e. if you're retrieving on integer that has a string key,
 * the call to the function would like so:
 *
 * int i; umap_get(map, key, &i);
 *
 * returns true if found or false if not found
 */

int umap_get(umap_t *, ...);

/*
 * the following get_* functions take the umap
 * as first parameter and the key as second parameter
 */
/* void *  umap_get_ref(umap_t *, ...);     returns a pointer to the data stored */
/* int     umap_get_i(umap_t *, ...);       returns the data as an int */
/* double  umap_get_d(umap_t *, ...);       returns the data as double */
/* void *  umap_get_p(umap_t *, ...);       returns the data as pointer */

void umap_free(umap_t *);
void umap_destroy(umap_t *);

#endif
