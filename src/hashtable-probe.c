#include "lib/hashtable-probe.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

int num_probes = 0, num_resize = 0;


#define hashtable_get_entry(ht, idx) ht->table + ((ht->entry_size * (idx)) % (ht->table_size * ht->entry_size))
#define hashtable_should_resize(ht) if ((float) ht->size / ht->table_size >= HASHTABLE_LOAD_FACTOR) hashtable_resize(ht);

static unsigned long prime_doubles[] = {
  3,
  7,
  13,
  31,
  61,
  127,
  251,
  509,
  1021,
  2039,
  4093,
  8191,
  16381,
  32749,
  65521,      /* For 1 << 16 */
  131071,
  262139,
  524287,
  1048573,
  2097143,
  4194301,
  8388593,
  16777213,
  33554393,
  67108859,
  134217689,
  268435399,
  536870909,
  1073741789,
  2147483647
    // 5,
//     11,
//     23,
//     47,
//     97,
//     197,
//     397,
//     797,
//     1597,
//     3203,
//     6421,
//     12853,
//     25717,
//     51437,
//     102877,
//     205759,
//     411527,
//     823117,
//     1646237,
//     3292489,
//     6584983,
//     13169977,
//     26339969,
//     52679969,
//     105359939,
//     210719881,
//     421439783,
//     842879579,
//     1685759167,
//     3371518343,
//     6743036717,
//     13486073473,
//     26972146961,
//     53944293929,
//     107888587883,
//     215777175787,
//     431554351609,
//     863108703229,
//     1726217406467,
//     3452434812973,
//     6904869625999,
//     13809739252051,
//     27619478504183,
//     55238957008387,
//     110477914016779,
//     220955828033581,
//     441911656067171,
//     883823312134381,
//     1767646624268779,
//     3535293248537579
};

static void * hashtable_probe(hashtable_t *, hashtable_entry_t * entry, int find);
static void hashtable_resize(hashtable_t *);

hashtable_t * hashtable_create(int entry_size, int (*entry_cmp)(void *, void *, void *), void * entry_cmp_state)
{
    hashtable_t * this = malloc(sizeof(hashtable_t));
    
    hashtable_init(this, entry_size, entry_cmp, entry_cmp_state); 
    return this;
}

void hashtable_init(hashtable_t * this, int entry_size, int (*entry_cmp)(void *, void *, void *), void * entry_cmp_state)
{
    this->size              = 0;
    this->prime_idx         = 0;
    this->entry_size        = entry_size;
    this->entry_cmp         = entry_cmp;
    this->entry_cmp_state   = entry_cmp_state;
    this->table_size        = prime_doubles[this->prime_idx];
    
    num_probes = 0;
    num_resize = 0;
    
    /* eager initialize the hashtable data, probably should lazy load instead */
    this->table = calloc(this->table_size, this->entry_size);
}

void * hashtable_lookup_entry(hashtable_t * this, void * entry, hashtable_lookup_t lu_type)
{
    unsigned long idx;
    hashtable_entry_t * ht_entry;

    /* should we resize? */
    hashtable_should_resize(this); /* only resizes if it needs to */
    
    ht_entry = hashtable_probe(this, entry, 0);

    /* no matter what, returned the occupied entry */
    if (ht_entry->is_occupied)
        return ht_entry;
        
    /* if we are looking for a node, and didn't find it, return NULL */
        
    switch (lu_type)
    {
        case HASHTABLE_LOOKUP_SEARCH:
            return NULL;
        case HASHTABLE_LOOKUP_INSERT:
            memcpy(ht_entry, entry, this->entry_size);
            ht_entry->is_occupied  = 1;
            this->size++;
            break;
        case HASHTABLE_LOOKUP_DELETE:
            ht_entry->is_occupied = 0;
            this->size--;
            break;
    }
    
    return ht_entry;
}

void hashtable_free(hashtable_t * this)
{
    free(this->table);
}

void hashtable_destroy(hashtable_t * this)
{
    hashtable_free(this);
    free(this);
}

void hashtable_print(hashtable_t * this)
{
    unsigned long i, occupied = 0;
    hashtable_entry_t * entry;
    const char * fmt = "{\n"
    "   is_occupied = %d\n"
    "   hash = %ld\n"
    "}\n";
    
    printf("table size = %ld\n", this->size);
    
    for (i = 0; i < this->table_size * this->entry_size; i += this->entry_size)
    {
        entry = (hashtable_entry_t *) ((char *)this->table + i);
        
        if (entry->is_occupied)
            occupied++;
        
        printf(fmt, entry->is_occupied, entry->hash);
    }
    
    printf("occupied size = %ld\n", occupied);
}

static void hashtable_resize(hashtable_t * this)
{
    unsigned long old_size = this->table_size * this->entry_size,
                  i;
    void * old_table = this->table;
    hashtable_entry_t * old_entry,
                      * new_entry;

    this->prime_idx++;
    //num_resize++;
    
    /* TODO - proper error handling */
    
    this->table_size = prime_doubles[this->prime_idx];
    this->table = calloc(this->table_size, this->entry_size);
    
    /* re index the entries */
    for (i = 0; i < old_size; i += this->entry_size)
    {
        old_entry = (hashtable_entry_t * ) ((char *)old_table + i);
        if (old_entry->is_occupied == 0)
            continue;
        
        new_entry = hashtable_probe(this, old_entry, 1);
        memcpy(new_entry, old_entry, this->entry_size);
    }
    
    free(old_table);
}

static void * hashtable_probe(hashtable_t * this, hashtable_entry_t * entry, int only_empty)
{
    int i = 0;
    unsigned long idx = entry->hash % this->table_size,
                  tmp_idx;
    hashtable_entry_t * ht_entry;

    while (1)
    {
        tmp_idx = (idx + i) % this->table_size; /* linear */
        //idx = tmp_idx;
        ht_entry = (hashtable_entry_t *) ((this->entry_size * tmp_idx) + (char *) this->table);

        if (ht_entry->is_occupied == 0)
            return ht_entry;

        /*
         * We are occupied, check if the hashes and values match,
         * but only do that if the only_empty parameter is not set.
         */
        if (!only_empty && ht_entry->hash == entry->hash && this->entry_cmp(ht_entry, entry, this->entry_cmp_state))
            return ht_entry;
            
        i++;
        //num_probes++;
    }
}
