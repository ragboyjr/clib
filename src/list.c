#include "lib/list.h"

#include <assert.h>
#include <stdlib.h>
#include <errno.h>

#include <stdio.h>

list_t * list_create()
{
    list_t * this = (list_t *) malloc(sizeof(list_t));
    
    if (this == NULL)
    {
        return NULL;
    }
    
    list_init(this);
    
    return this;
}

void list_init(list_t * this)
{
    assert(this);
    
    this->size      = 0;
    this->head      = NULL;
    this->tail      = NULL;
    this->cur       = NULL;
}

static int list_push(list_t * this, union _list_datum data)
{
    list_item_t * li = (list_item_t *) malloc(sizeof(list_item_t));
    
    if (li == NULL)
    {
        return ENOMEM;
    }
    
    li->next    = NULL;
    li->prev    = NULL;
    li->data    = data;
    
    if (this->head == NULL)
    {
        this->head = this->tail = li;
    }
    else
    {
        this->tail->next = li;
        li->prev = this->tail;
        this->tail = li;
    }
    
    this->size++;
    
    return 0;
}

int list_push_d(list_t * this, double d)
{
    union _list_datum data; data.d = d;
    return list_push(this, data);
}
int list_push_i(list_t * this, int i)
{
    union _list_datum data; data.i = i;
    return list_push(this, data);
}
int list_push_p(list_t * this, void * p)
{
    union _list_datum data; data.p = p;
    return list_push(this, data);
}


/* list top */

void * list_top(list_t * this)
{
    if (!this->tail)
        return NULL;    
    return &this->tail->data;
}
double list_top_d(list_t * this)
{
    if (!this->tail)
        return (double) 0;  
    return this->tail->data.d;
}
int list_top_i(list_t * this)
{
    if (!this->tail)
        return (int) 0;  
    return this->tail->data.i;
}
void * list_top_p(list_t * this)
{
    if (!this->tail)
        return NULL;    
    return this->tail->data.p;
}

int list_pop(list_t * this)
{
    list_item_t * prev;

    if (this->size == 0)
        return 0;
        
    /* we know there is a tail pointer */
    if (this->tail == this->head)   /* only 1 element */
    {
        free(this->tail);
        this->tail = this->head = NULL;
        this->size--;
        
        return 1;
    }
    
    prev = this->tail->prev;
    assert(prev);   /* this should never fail */
    
    free(this->tail);
    this->tail = prev;
    prev->next = NULL;
    this->size--;
    
    return 1;
}

int list_shift(list_t * this)
{
    list_item_t * next;

    if (this->size == 0)
        return 0;
        
    /* we know there is a tail pointer */
    if (this->tail == this->head)   /* only 1 element */
    {
        free(this->head);
        this->tail = this->head = NULL;
        this->size--;
        
        return 1;
    }
    
    
    next = this->head->next;
    assert(next);   /* this should never fail */
    
    free(this->head);
    this->head          = next;
    this->head->prev    = NULL;
    this->size--;
    
    return 1;
}

list_t * list_join(list_t * this, list_t * ll_add)
{
    if (this->size > 0 && ll_add->size > 0)
    {
        this->tail->next = ll_add->head;
        ll_add->head->prev = this->tail;
        
        this->tail = ll_add->tail;
        this->size += ll_add->size;
        
        ll_add->size    = 0;
        ll_add->head    = NULL;
        ll_add->tail    = NULL;
        ll_add->cur     = NULL;
    }
    
    return this;
}

void * list_get(list_t * this)
{
    if (!this->cur) return NULL;
    return &this->cur->data;
}
double list_get_d(list_t * this)
{
    if (!this->cur) return 0.0;
    return this->cur->data.d;
}
int list_get_i(list_t * this)
{
    if (!this->cur) return 0;
    return this->cur->data.i;
}
void * list_get_p(list_t * this)
{
    if (!this->cur) return NULL;
    return this->cur->data.p;
}

int list_del(list_t * this)
{
    list_item_t * tmp = NULL;
    
    if (this->cur != NULL)
    {
        if (this->cur != this->head)
        {
            this->cur->prev->next = this->cur->next;
            tmp = this->cur->prev;
        }
        else
        {
            this->head = this->head->next;
        }

        if (this->cur != this->tail)
        {
            this->cur->next->prev = this->cur->prev;
        }
        else
        {
            this->tail = this->tail->prev;
        }

        free(this->cur);
        this->cur = tmp;
        this->size--;
        return 1;
    }
    
    return 0;
}

void list_destroy(list_t * this)
{
    list_free(this);
    
    /* destroy the list */
    free(this);
}

void list_free(list_t * this)
{
    list_item_t * next,
                * cur;
    
    cur = this->head;
    
    /* destroy each list item */
    
    while (cur != NULL)
    {
        next = cur->next;
        free(cur);
        cur = next;
    }
}

void list_print(list_t * this)
{
    const char * fmt = "list {\n"
    "   head = %#010x\n"
    "   tail = %#010x\n"
    "   cur  = %#010x\n"
    "   size = %d\n"
    "}\n";
    printf(fmt, this->head, this->tail, this->cur, this->size);
}
