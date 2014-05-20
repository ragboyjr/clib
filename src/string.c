#include "lib/string.h"
#include "util/debug.h"

#include <stdlib.h>
#include <string.h>

#define should_init_buffer(s) if (!this->buf) string_init_buffer(s)

static void string_init_buffer(string_t *);

string_t * string_create()
{
    string_t * this = malloc(sizeof(string_t));
    string_init(this);    
    return this;
}

void string_init(string_t * this)
{
    this->buf       = NULL;
    this->size      = 0;
    this->buf_size  = 0;
}

void string_eager_init(string_t * this)
{
    should_init_buffer(this);
}

void string_set_size(string_t * this, int size)
{
    debug(DBG_INFO, "string set size\n");
    should_init_buffer(this);
    
    this->size = size;
    this->buf[this->size] = '\0';
}

void string_alloc(string_t * this, int size)
{
    if (size < STRING_MIN_SIZE)
        size = STRING_MIN_SIZE;
            
    this->buf_size = size;
    this->buf = realloc(this->buf, this->buf_size);
}

void string_free(string_t * this)
{
    if (this->buf)
        free(this->buf);
}

void string_destroy(string_t * this)
{
    string_free(this);
    free(this);
}

void string_cat(string_t * this, const char * restrict s)
{
    int s_len = strlen(s);
    
    should_init_buffer(this);

    // + 1 to include the NULL terminator
    if (this->size + s_len + 1 >= this->buf_size)
    {
        this->buf_size = (this->buf_size + s_len + 1) * STRING_GROWTH_FACTOR;
        this->buf = realloc(this->buf, this->buf_size);
    }
    
    strcat(this->buf, s);
    
    this->size += s_len;
}

void string_append_c(string_t * this, char c)
{
    char buf[] = {c,'\0'};
    string_cat(this, buf);
}

static void string_init_buffer(string_t * this)
{
    this->buf       = malloc(sizeof(char) * STRING_MIN_SIZE);
    this->buf[0]    = '\0';
    this->buf_size  = STRING_MIN_SIZE;
}
