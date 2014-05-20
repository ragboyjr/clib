#include "lib/hashtable.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <assert.h>

#define hashtable_get_entry(ht, idx) ht->table + ((ht->entry_size * (idx)) % (ht->table_size * ht->entry_size))
#define hashtable_should_resize(ht) if ((float) ht->size / ht->table_size >= HASHTABLE_LOAD_FACTOR) hashtable_resize(ht);

static unsigned long prime_doubles[] = {
    1,
    3,
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

static void * hashtable_probe(hashtable_t *, hashtable_entry_t * entry,hashtable_lookup_t lu_type);
static void hashtable_resize(hashtable_t *);

static int hashtable_entry_in_table(hashtable_t * this, hashtable_entry_t * entry, void * old_table)
{
    return (old_table <= entry && entry < (old_table + this->table_size * this->entry_size));
}

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
    
    /* eager initialize the hashtable data, probably should lazy load instead */
    this->table = calloc(this->table_size, this->entry_size);
}

void * hashtable_lookup_entry(hashtable_t * this, void * entry, hashtable_lookup_t lu_type)
{
    unsigned long idx;
    hashtable_entry_t * ht_entry;

    /* should we resize? */
    hashtable_should_resize(this); /* only resizes if it needs to */
    
    ht_entry = hashtable_probe(this, entry, lu_type);

    /* not found... */
    if (!ht_entry)
        return NULL;

    /* no matter what, returned the occupied entry, something was found */
    if (ht_entry->is_occupied)
        return ht_entry;
        
    /* if we are looking for a node, and didn't find it, return NULL */
        
    switch (lu_type)
    {
        case HASHTABLE_LOOKUP_INSERT:
            memcpy(ht_entry, entry, this->entry_size);
            ht_entry->is_occupied  = 1;
            ht_entry->next = NULL;
            this->size++;
            break;
        case HASHTABLE_LOOKUP_DELETE:
            ht_entry->is_occupied = 0;
            this->size--;
            break;
        default:
            return NULL;
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
    
    for (i = 0; i < this->table_size * this->entry_size; i += this->entry_size)
    {
        entry = (hashtable_entry_t *) ((char *)this->table + i);
        
        if (entry->is_occupied)
            occupied++;
        
        //printf(fmt, entry->is_occupied, entry->hash);
    }
    
    //printf("occupied size = %ld\n", occupied);
}

static void hashtable_resize(hashtable_t * this)
{
    unsigned long old_size = this->table_size * this->entry_size,
                  i;
    int use_entry;
    void * old_table = this->table;
    hashtable_entry_t * old_entry,
                      * new_entry,
                      * tmp;
    this->prime_idx++;
    
    /* TODO - proper error handling */
    
    this->table_size = prime_doubles[this->prime_idx];
    this->table = calloc(this->table_size, this->entry_size);

    /* re index the entries */
    for (i = 0; i < old_size; i += this->entry_size)
    {
        old_entry = (hashtable_entry_t * ) ((char *)old_table + i);
        
        if (old_entry->is_occupied == 0)
            continue;

        
        while (old_entry)
        {
            new_entry = hashtable_probe(this, old_entry, HASHTABLE_LOOKUP_INSERT);
            
            memcpy(new_entry, old_entry, this->entry_size);
            
            new_entry->next = NULL;
            
            tmp = old_entry;
            old_entry = old_entry->next;
            
            if (!hashtable_entry_in_table(this, tmp, old_table))
            {
                free(tmp);
            }
        }
    }
    
    free(old_table);
}

static void * hashtable_probe(hashtable_t * this, hashtable_entry_t * entry, hashtable_lookup_t lu_type)
{
    int i = 0;
    unsigned long idx = entry->hash % this->table_size,
                  tmp_idx;
    hashtable_entry_t * ht_entry,
                      * p,
                      * prev = NULL,
                      * new_entry = NULL;

    ht_entry = (hashtable_entry_t *) ((this->entry_size * idx) + (char *) this->table);

    p = ht_entry;

    while (p)
    {
        if (p->is_occupied == 0)
            return p;
     
        if (p->hash == entry->hash && this->entry_cmp(p, entry, this->entry_cmp_state))
            return p;
     
        prev = p;
        
        p = p->next;    // iterate to next element
    }

    // if we've made it this far, then there is no match. return new element
    if (lu_type != HASHTABLE_LOOKUP_INSERT)
        return NULL;
    
    new_entry = calloc(1, this->entry_size);
        
    prev->next = new_entry;
    
    return new_entry;
}
