#include "lib/refc.h"

#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <stdio.h>
#include <execinfo.h>

/*
 * Reference Counting is handled by allocated an integer along with 
 * the memory being allocated. The small int holds the refcount
 */

#define ptr_to_refc(p) (int *) (((char *) (p)) - sizeof(int))

void * refc_alloc(size_t size)
{
    void * ptr;
    int * refc;
    size += sizeof(int); /* add size for refcount */
    
    ptr = malloc(size);
    
    if (!ptr) {
        return NULL;
    }
    
    /* I have a feeling this might NOT work depending on the endianess of the machine. */
    *((int *) ptr) = 1; /* set refcount to 1 */    
    
    /* the pointer returned points to the actual memory, not including our extra int */
    return ((char *) ptr) + sizeof(int);
}

void refc_free(void * ptr)
{
    /* this pointer is at the memory, we need to back up to the int */
    int * refc = ptr_to_refc(ptr);
    
    /* decrement the refcount */
    *refc -= 1;
    
    if (*refc > 0) {
        return; /* refcount is greater than 0, do nothing */
    }
    
    if (*refc < 0)
    {
         void* callstack[128];
         int i, frames = backtrace(callstack, 128);
         char** strs = backtrace_symbols(callstack, frames);
         for (i = 0; i < frames; ++i) {
             printf("%s\n", strs[i]);
         }
         free(strs);
         assert(0);
    }
    
    assert(*refc == 0);
    
    /* free the memory now */
    puts("refc freeing memory");
    free(refc);
}

void refc_incr(void * ptr)
{
    int * refc = ptr_to_refc(ptr);
    *refc += 1;
}

char * refc_strdup(const char * s)
{
    int size = strlen(s);
    char * new_s = refc_alloc(size);
    memcpy(new_s, s, size);
    new_s[size] = '\0'; /* null terminiate */
    
    return new_s;
}
