#include "lib/hash.h"

#define HASH_INIT_VALUE 5381
#define hash_byte(hash, c) ((hash << 5) + hash) + c

unsigned long hash_str(char * str)
{
    unsigned long hash = HASH_INIT_VALUE;
    char c;
    
    while ((c = *str++))
        hash = hash_byte(hash, c);

    return hash;
}

unsigned long hash_bytes(void * vdata, int len)
{
    char * data = (char *) vdata;
    unsigned long hash = HASH_INIT_VALUE;
    char c;
    int i;
    
    for (i = 0; i < len; i++)
        hash = hash_byte(hash, *data++);
        
    return hash;
}
