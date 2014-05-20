#ifndef _LIB_SET_COMMON_H
#define _LIB_SET_COMMON_H

#include "lib/set.h"

rb_key_u    set_get_va_key(set_t * this, va_list ap);
void        set_set_key_p(set_t * this, rb_key_u data, void * key);
int         set_el_cmp(const rb_key_u, const rb_key_u, void *);

#endif
