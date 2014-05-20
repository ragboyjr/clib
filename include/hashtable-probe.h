#ifndef _LIB_HASHTABLE_H
#define _LIB_HASHTABLE_H

/*
 * The hashtable is an internal use only data structure used to implement the 
 * umap and uset.
 */

#ifndef HASHTABLE_LOAD_FACTOR
#define HASHTABLE_LOAD_FACTOR .8
#endif

#ifndef HASHTABLE_PROBE_LINEAR
#define HASHTABLE_PROBE_LINEAR 1
#endif

/* the umap and uset entries need to share the same
 * memory layout as this struct or BAD things will
 * happen
 */
typedef struct {
    int is_occupied;
    unsigned long hash;
} hashtable_entry_t;

typedef enum {
    HASHTABLE_LOOKUP_INSERT,
    HASHTABLE_LOOKUP_SEARCH,
    HASHTABLE_LOOKUP_DELETE
} hashtable_lookup_t;

typedef struct {
    /* array of hash entries */
    void * table,
         * entry_cmp_state; /* arbitrary data to pass along to the entry_cmp func */
    
    unsigned long table_size, /* size of the allocated table */
                  size; /* number of entries in hash table */
    
    int prime_idx,  /* index into the prime doubles array */
        mod,
        mask,
        entry_size; /* size of the entries for the hash table */
    int (*entry_cmp)(void *, void *, void *); /* entry, entry, state */

} hashtable_t;

hashtable_t * hashtable_create(int /* entry size */, int (*)(void *, void *, void *) /* entry comp */, void * /* entry cmp state */);
void hashtable_init(hashtable_t *, int /* entry size */, int (*)(void *, void *, void *) /* entry comp */, void * /* entry cmp state */);

void * hashtable_lookup_entry(hashtable_t *, void * /* entry */, hashtable_lookup_t);

void hashtable_print(hashtable_t *);

void hashtable_free(hashtable_t *);
void hashtable_destroy(hashtable_t *);

#endif
