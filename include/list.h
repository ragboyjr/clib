#ifndef _LIB_LIST_H
#define _LIB_LIST_H

#include <assert.h>

/* a union for making the list datums more versatile */
union _list_datum {
    void * p;
    int i;
    double d;
};

struct _list_item {
    union _list_datum data;
    struct _list_item * next;
    struct _list_item * prev; 
};

typedef struct _list_item list_item_t;

/* macros for accessing items in the list */

typedef struct {
    list_item_t * head,
                * tail,
                * cur;  /* cur is used for iteration */
    int size;
} list_t;

/*
 * Creates a new doubly linked list
 *
 *		list_t * ll = list_create();
 *
 */
list_t * list_create();

/*
 * Initializes a doubly linked list.
 * Same as list_create, except, list_init
 * expects a created link list
 *
 *		list_t ll;
 *		list_init(&ll);
 */
void list_init(list_t *);
 
int list_push_d(list_t *, double);
int list_push_i(list_t *, int);
int list_push_p(list_t *, void *);

/*
 * Grabs the last element in the list, used to implement a stack
 *
 *		list_t * ll;
 *		void * data = list_top(ll);
 */

void *	list_top(list_t *);	/* returns a pointer to the datum (can be used for setting) */
double	list_top_d(list_t *);
int		list_top_i(list_t *);
void *	list_top_p(list_t *);

/*
 * Removes the last element from the list. O(1), used for a stack implementation
 *
 *		list_t * ll;
 *		int success = list_pop(ll);
 */
int list_pop(list_t *);

/*
 * removes first item from list
 *
 *		list_t * ll;
 *		int data = list_shift(ll);
 */
int list_shift(list_t *);

/*
 * Joins the second list with the first, the second list is appended
 * to the first. Returns a pointer to the first list. Calling list_join
 * will render the second list useless, so only call _this if you no longer need
 * the second list
 *
 *		list_t * ll_0, ll_1
 *		list_t * joined = list_join(ll_0, ll_1);
 */
list_t * list_join(list_t *, list_t *);

/*
 * advance the internal pointer
 */
#define list_next(ll) (assert((ll)->cur), (ll)->cur = (ll)->cur->next)
#define list_valid(ll) ((ll)->cur != NULL)

/* retrieve current list item */
void *	list_get(list_t *);	/* returns pointer to datum (can be used for setting) */
double	list_get_d(list_t *);
int		list_get_i(list_t *);
void *	list_get_p(list_t *);

/*
 * Removes the list item the iterator is pointing to.
 *
 *		list_t * ll;
 *		void * data;
 *		while (list_next(ll, &data)) {
 *			// the following is sudo code
 *			if (data == SOME_VALUE) {
 *				// the current list item will be freed up from the list, and the
 *				// iterator now points to the element before current.
 *				list_del(ll);
 *			}
 *		}		
 */
int list_del(list_t *);

/*
 * Resets the iteration pointer
 *
 *		list_t * ll;
 *		list_reset(ll);
 *		// now we can iterate trough again
 *
 */
#define list_reset(ll) ((ll)->cur = (ll)->head)

void list_print(list_t *);

/*
 * Frees up the memory for the list and destroys the list itself
 */
void list_destroy(list_t *);
void list_free(list_t *);   /* only frees up the memory associated */

#define list_foreach(ll, data) assert(ll);\
    for (list_reset(ll); data = list_get(ll), list_valid(ll); list_next(ll))
    
#define list_foreach_d(ll, data) assert(ll);\
    for (list_reset(ll); data = list_get_d(ll), list_valid(ll); list_next(ll))
    
#define list_foreach_i(ll, data) assert(ll);\
    for (list_reset(ll); data = list_get_i(ll), list_valid(ll); list_next(ll))

#define list_foreach_p(ll, data) assert(ll);\
    for (list_reset(ll); data = list_get_p(ll), list_valid(ll); list_next(ll))

#endif
