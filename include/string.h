#ifndef _LIB_STRING_H
#define _LIB_STRING_H

#define STRING_GROWTH_FACTOR 1.5
#define STRING_MIN_SIZE 16

typedef struct {
	char * buf;

	unsigned int size,
                 buf_size;
} string_t;

string_t *  string_create();
void        string_init(string_t *);
void        string_eager_init(string_t *);
void        string_set_size(string_t *, int size);
void        string_alloc(string_t *, int size);
void        string_free(string_t *);
void        string_destroy(string_t *);
void        string_cat(string_t *, const char * restrict s);
void        string_append_c(string_t *, char c);


#endif
