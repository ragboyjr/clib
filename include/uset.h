#ifndef _LIB_USET_H
#define _LIB_USET_H

#include "lib/hashtable.h"
#include <stdarg.h>

/*
 * Unordered Map implemented by a hash table.
 */

#define uset_key_t_int(m)   m->key_type = USET_KEY_TYPE_INT
#define uset_key_t_dbl(m)   m->key_type = USET_KEY_TYPE_DOUBLE
#define uset_key_t_str(m)   m->key_type = USET_KEY_TYPE_STRING
#define uset_val_t_int(m)   m->val_type = USET_VAL_TYPE_INT
#define uset_val_t_dbl(m)   m->val_type = USET_VAL_TYPE_DOUBLE
#define uset_val_t_data(m)  m->val_type = USET_VAL_TYPE_DATA

union _uset_datum {
    void * p;
    int i;
    double d;
};

typedef enum {
    USET_KEY_TYPE_INT,
    USET_KEY_TYPE_DOUBLE,
    USET_KEY_TYPE_STRING
} uset_key_type_t;

typedef enum {
    USET_VAL_TYPE_INT,
    USET_VAL_TYPE_DOUBLE,
    USET_VAL_TYPE_DATA
} uset_val_type_t;

/* this needs to share the same structure as defined in hashtable.h */
typedef struct _uset_entry {
    int is_occupied;
    unsigned long hash;
    struct _uset_entry * left,
                       * right,
                       * next;
    union _uset_datum key;
} uset_entry_t;

typedef struct {
    hashtable_t ht;
    
    uset_key_type_t key_type;
    uset_val_type_t val_type;
} uset_t;

uset_t * uset_create();
void uset_init(uset_t *);

/*
 * add values to the unordered map. The parameters are
 * uset_t, key, value. The key and value will be parsed as whatever is the current
 * types are defined in the uset
 */
void uset_add(uset_t *, ...);

/*
 * accepts the uset, key, then a pointer for the data to be stored.
 * i.e. if you're retrieving on integer that has a string key,
 * the call to the function would like so:
 *
 * int i; uset_get(map, key, &i);
 *
 * returns true if found or false if not found
 */

int uset_get(uset_t *, ...);

/*
 * the following get_* functions take the uset
 * as first parameter and the key as second parameter
 */
/* void *  uset_get_ref(uset_t *, ...);     returns a pointer to the data stored */
/* int     uset_get_i(uset_t *, ...);       returns the data as an int */
/* double  uset_get_d(uset_t *, ...);       returns the data as double */
/* void *  uset_get_p(uset_t *, ...);       returns the data as pointer */

void uset_free(uset_t *);
void uset_destroy(uset_t *);

#endif
