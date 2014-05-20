#ifndef _LIB_REFC_H
#define _LIB_REFC_H

#include <stddef.h>

/* simple api for reference counting */

void *  refc_alloc(size_t);
void    refc_free(void *);
void    refc_incr(void *);
char *  refc_strdup(const char *);

#endif
