#include "lib/set/common.h"

#include <assert.h>
#include <string.h>

int set_el_cmp(const rb_key_u key1, const rb_key_u key2, void * _arg)
{  
    set_t * this = _arg;

    if (this->el_cmp) {
        return this->el_cmp(key1.p, key2.p);
    }

    switch (this->type)
    {
        case SET_TYPE_INT:
            if (key1.i == key2.i)
                return 0;
            else if (key1.i < key2.i)
                return -1;
            else
                return 1;
        case SET_TYPE_DOUBLE:
            if (key1.d == key2.d)
                return 0;
            else if (key1.d < key2.d)
                return -1;
            else
                return 1;
        case SET_TYPE_STRING:
            return strcmp(key1.p, key2.p);
        default:
            /* you need to provide a custom key comparison for pointers */
            assert(0);
    }
}
void set_set_key_p(set_t * this, rb_key_u data, void * key)
{
    switch (this->type)
    {
        case SET_TYPE_INT:
            *((int *) key) = data.i;
            break;
        case SET_TYPE_DOUBLE:
            *((double *) key) = data.d;
            break;
        case SET_TYPE_STRING:
            *((char **) key) = data.p;
            break;
        case SET_TYPE_POINTER:
            *((void **) key) = data.p;
            break;
    }
}
rb_key_u set_get_va_key(set_t * this, va_list ap)
{
    rb_key_u key;
    
    switch (this->type)
    {
        case SET_TYPE_INT:
            key.i = va_arg(ap, int);
            break;
        case SET_TYPE_DOUBLE:
            key.d = va_arg(ap, double);
            break;
        case SET_TYPE_STRING:
        case SET_TYPE_POINTER:
            key.p = va_arg(ap, void *);
            break;
    }
    
    return key;
}
