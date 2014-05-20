#include "lib/map.h"
#include <string.h>
#include <stdlib.h>

/* used for debugging #include <math.h> */

static rb_node_t * map_tree_search(map_t * this, const map_datum_t key);
static rb_node_t * map_tree_insert(map_t * this, const map_datum_t key, const map_datum_t val);
static rb_node_t * map_node_create(map_t * this, const map_datum_t key, const map_datum_t val);
static void map_tree_destroy(map_t * this, rb_node_t * root);
/*static void tree_print_walk(rb_node_t * root, FILE * stream);*/
/*static void print_tree(map_t * this, rb_node_t * root, FILE * stream);*/
/*static void map_tree_level(map_t * this, rb_node_t ** const nodes, int map_level, int level, rb_node_t * root, int start, int end); */

static int map_tree_height(map_t * this, rb_node_t * root);

static int map_key_cmp(map_t *, map_datum_t, map_datum_t);

/* helper functions and macros */

#define map_is_leaf(map, n) (n == &(map)->nil)

static map_datum_t map_get_va_key(map_t * this, va_list ap);
static map_datum_t map_get_va_val(map_t * this, va_list ap);
static void map_set_key_p(map_t * this, map_datum_t data, void * key);
static void map_set_val_p(map_t * this, map_datum_t data, void * val);

static void map_transplant(map_t * this, rb_node_t * node, rb_node_t * map_successor);
static rb_node_t * map_grandparent(map_t * this, rb_node_t * n);
static rb_node_t * map_uncle(map_t * this, rb_node_t * n);

static void map_rotate_left(map_t * this, rb_node_t * p);
static void map_rotate_right(map_t * this, rb_node_t * p);
static void map_update_root(map_t * this);

static rb_node_t * map_successor(map_t * this, rb_node_t * n);
static rb_node_t * map_predecessor(map_t * this, rb_node_t * n);

static void replace_node(rb_node_t * n1, const rb_node_t * n2);

/* functions for balancing */

static void map_insert_case1(map_t * this, rb_node_t * n);
static void map_insert_case2(map_t * this, rb_node_t * n);
static void map_insert_case3(map_t * this, rb_node_t * n);
static void map_insert_case4(map_t * this, rb_node_t * n);
static void map_insert_case5(map_t * this, rb_node_t * n);

static void map_delete_case1(map_t * this, rb_node_t * n);
static void map_delete_case2(map_t * this, rb_node_t * n);
static void map_delete_case3(map_t * this, rb_node_t * n);
static void map_delete_case4(map_t * this, rb_node_t * n);
static void map_delete_case5(map_t * this, rb_node_t * n);
static void map_delete_case6(map_t * this, rb_node_t * n);

map_t * map_create()
{
    map_t * this = malloc(sizeof(map_t));
    
    if (this == NULL)
        return NULL;
        
    map_init(this);
    
    return this;
}

void map_init(map_t * this)
{
    /* init the values */
    memset(&this->nil, 0, sizeof(rb_node_t));
    this->nil.color     = RB_COLOR_BLACK;

    this->root      = &this->nil;
    this->iter_p    = &this->nil;
    this->count     = 0;
    
    this->key_type  = MAP_KEY_TYPE_STRING;
    this->val_type  = MAP_VAL_TYPE_POINTER;
    this->key_cmp   = NULL;
}

void map_destroy(map_t * this)
{
    /* free the nodes */
    map_tree_destroy(this, this->root);
    free(this);
}

/*void map_print(map_t * this, FILE * stream)
{
    print_tree(this, this->root, stream);   
}*/

int map_add(map_t * this, ...)
{
    va_list ap; /* arg pointer */
    map_datum_t key,
                val;
    rb_node_t * n = &this->nil;

    /* grab the key and value*/    
    va_start(ap, this);

    key = map_get_va_key(this, ap);
    val = map_get_va_val(this, ap);
    
    va_end(ap);
    
    if (map_is_leaf(this, this->root))
        n = this->root = map_node_create(this, key, val);
    else
        n = map_tree_insert(this, key, val);
    
    if (map_is_leaf(this, n))
        return 1;
        
    /* let's balance the tree now */
    this->count++;
    
    map_insert_case1(this, n);
    map_update_root(this);
    
    return 0;
}

int map_get(map_t * this, ...)
{
    va_list ap; /* arg pointer */
    map_datum_t key;
    void * ret;
    rb_node_t * node;
                                         
    /* grab the key and data return pointer */
    va_start(ap, this);
    
    key = map_get_va_key(this, ap);
    ret = va_arg(ap, void *);
    
    va_end(ap);
    
    node = map_tree_search(this, key);
    
    if (!map_is_leaf(this, node))
    {
        if (ret)
            map_set_val_p(this, node->value, ret);
        
        return 1;
    }
    
    return 0;
}

int map_has(map_t * this, ...)
{
    va_list ap; /* arg pointer */
    map_datum_t key;
    rb_node_t * node;
                                         
    /* grab the key and data return pointer */
    va_start(ap, this);
    
    key = map_get_va_key(this, ap);
    
    va_end(ap);
    
    node = map_tree_search(this, key);
    
    if (!map_is_leaf(this, node))
        return 1;
    
    return 0;
}

int map_del(map_t * this, ...)
{
    va_list ap; /* arg pointer */
    map_datum_t key;
    rb_node_t * node,
              * child,
              * s = &this->nil; /* s = successor */
                                         
    /* grab the key and data return pointer */
    va_start(ap, this);
    
    key = map_get_va_key(this, ap);
    
    va_end(ap);
    
    node = map_tree_search(this, key);
    
    if (map_is_leaf(this, node))
        return 0;
    
    /* find the successor (which could be the in-order predecessor or in-order successor) */
    
    if (!map_is_leaf(this, node->right))
    {
        s = node->right;
        while (!map_is_leaf(this, s->left))
            s = s->left;
    }
    else if (!map_is_leaf(this, node->left))
    {
        s = node->left;
        while (!map_is_leaf(this, s->right))
            s = s->right;
    }
    
    if (!map_is_leaf(this, s))
    {
        /*printf("not leaf\n");*/
        /* swap the values */
        replace_node(node, s);
    }
    else    /* if this is the case */
        s = node;  /* n is it's own map_successor */
    
    /* printf("s = %d, s->p = %p, n = %p\n", * (int *) s->key, s->parent, n); */
    
    /* assumption: s points to the node that needs to be deleted */
    child = (map_is_leaf(this, s->right)) ? s->left : s->right;
        
    map_transplant(this, s, child);
    
    if (s->color == RB_COLOR_BLACK)
    {
        if (child->color == RB_COLOR_RED)
            child->color = RB_COLOR_BLACK;
        else
            map_delete_case1(this, child);
    }
    
    this->count--;
    
    free(s);
    
    return 1;
}

void map_reset(map_t * this)
{
    rb_node_t * n = this->root;
    
    if (!map_is_leaf(this, n))
    {
        while (!map_is_leaf(this, n->left))
            n = n->left;
    }
    
    this->iter_p = n;   /* the smallest node */
}

void map_next(map_t * this)
{
    this->iter_p = map_successor(this, this->iter_p);
}

void map_cur(map_t * this, ...)
{
    va_list ap; /* arg pointer */
    void * key_ret,
         * val_ret;
    
    /* map_valid should be called before making a call to this */
    if (map_is_leaf(this, this->iter_p))
        return;
                                             
    /* grab the key and data return pointer */
    va_start(ap, this);
    
    key_ret = va_arg(ap, void *);
    val_ret = va_arg(ap, void *);
    
    va_end(ap);
        
    map_set_key_p(this, this->iter_p->key, key_ret);
    map_set_val_p(this, this->iter_p->value, val_ret);
}

static void map_check_pointers(map_t * this, rb_node_t * n)
{
    if (!map_is_leaf(this, n->left))
        assert(n->left->parent == n);
    if (!map_is_leaf(this, n->right))
        assert(n->right->parent == n);
}

static rb_node_t * map_tree_search(map_t * this, const map_datum_t key)
{
    int cmp_res;
    rb_node_t * p = this->root;
    
    while (!map_is_leaf(this, p))
    {
        /* debugging to make sure the pointers are setup */
        map_check_pointers(this, p);
        
        cmp_res = map_key_cmp(this, p->key, key);
        
        if (cmp_res > 0)
            p = p->left;
        else if (cmp_res < 0)
            p = p->right;
        else
            return p;
    }
    
    return &this->nil;
}

static rb_node_t * map_tree_insert(map_t * this, const map_datum_t key, const map_datum_t value)
{
    int cmp_res;
    rb_node_t * n,
              * parent = NULL;
    
    n = this->root;
    
    while (!map_is_leaf(this, n))
    {
        cmp_res = map_key_cmp(this, n->key, key);

        parent = n;

        if (cmp_res > 0)
        {
            n = n->left;
        }
        else if (cmp_res < 0)
        {
            n = n->right;
        }
        else /* match */
        {
            /* update the node's value */
            n->value = value;
            return &this->nil;
        }
    }
    
    /* the node wasn't found, so we insert to the parent's left or right */
    n = map_node_create(this, key, value);
    n->parent = parent;
    
    if (cmp_res > 0)
        parent->left = n;
    else
        parent->right = n;
    
    return n;
}

static void map_tree_destroy(map_t * this, rb_node_t * root)
{
    if (!map_is_leaf(this, root))
    {
        map_tree_destroy(this, root->left);
        map_tree_destroy(this, root->right);
        free(root);
    }
}

/*static void tree_print_walk(rb_node_t * root, FILE * stream)
{
    if (!map_is_leaf(this, root))
    {
        tree_print_walk(root->left, stream);
        fprintf(stream, "key: %s, value: %s\n", (char *) root->key, (char *) root->value);
        tree_print_walk(root->right, stream);
    }
}*/

static int map_tree_height(map_t * this, rb_node_t * root)
{
    if (!map_is_leaf(this, root))
    {
        int maxl = map_tree_height(this, root->left) + 1;
        int maxr = map_tree_height(this, root->right) + 1;
        
        return (maxl > maxr) ? maxl : maxr;
    }
    
    return 0;
}

static void map_tree_level(map_t * this, rb_node_t ** const nodes, int map_level, int level, rb_node_t * root, int start, int end)
{
    int mid = (end - start) / 2 + start;
 
    if (!map_is_leaf(this, root))
    {
        if (map_level != level)
        {
            map_tree_level(this, nodes, map_level, level + 1, root->left, start, mid);
            map_tree_level(this, nodes, map_level, level + 1, root->right, mid + 1, end);   /* go to mid + 1 because I'm implicitly flooring mid */
        }
        else
        {
            /* by now start and end should be equal */
            if (start == end)
            {
                nodes[start] = root;
            }
        }
    }
}

/*static void print_tree(map_t * this, rb_node_t * root, FILE * stream)
{
    int height = map_tree_height(this, root);
    int padding_width = pow(2.0, height - 1);
    int current_offset = padding_width - 1;
    int spaces_per_unit = 3;
    int level_width;
    char buffer[8];
    char format[32] = {0};
    

    rb_node_t ** const nodes = malloc(sizeof(rb_node_t *) * padding_width);

    / * NULL out the pointers * /
    int j;
    for (j = 0; j < padding_width; j++)
        nodes[j] = NULL;

    int i = 0;
    while (i < height)
    {
        level_width = pow(2.0, i);
        map_tree_level(this, nodes, i, 0, root, 0, level_width - 1);

        sprintf(buffer, "%d", current_offset * spaces_per_unit);
        strcpy(format, "%-");
        strcat(format, buffer);
        strcat(format, "s");
        
        fprintf(stream, format, "");
        
        sprintf(buffer, "%d", padding_width * spaces_per_unit * 2); / * * 2 because I want the min-padding to be 2 not 1 * /
        strcpy(format, "%-");
        strcat(format, buffer);
        strcat(format, "s");
 
        int k;
        for (k = 0; k < level_width; k++)
        {
            if (nodes[k] == NULL)
            {
                sprintf(buffer, "xxx");
            }
            else
            { 
                sprintf(buffer, "%02d", *(int *)nodes[k]->key);
                buffer[2] = (nodes[k]->color == RB_COLOR_BLACK) ? 'B' : 'R';
                buffer[3] = '\0';
                nodes[k] = NULL;    / * set back to NULL to use for next iteration. * /
            }
            
            fprintf(stream, format, buffer);
        }
 
        i++;
        padding_width = pow(2.0, height - 1 - i);
        current_offset -= padding_width;
        fprintf(stream, "\n");
    }
 
    free(nodes);
}*/

static rb_node_t * map_node_create(map_t * this, const map_datum_t key, const map_datum_t value)
{
    rb_node_t * n = malloc(sizeof(rb_node_t));
    
    n->color    = RB_COLOR_RED;
    n->key      = key;
    n->value    = value;
    n->left     = &this->nil;
    n->right    = &this->nil;
    n->parent   = &this->nil;
    
    return n;
}

static void replace_node(rb_node_t * n1, const rb_node_t * n2)
{
    n1->value   = n2->value;
    n1->key     = n2->key;
}

static rb_node_t * map_grandparent(map_t * this, rb_node_t * n)
{
    if ((!map_is_leaf(this, n)) && (!map_is_leaf(this, n->parent)))
        return n->parent->parent;
    else
        return &this->nil;
}

static rb_node_t * map_uncle(map_t * this, rb_node_t * n)
{
    rb_node_t * g = map_grandparent(this, n);
    
    if (map_is_leaf(this, g))
        return g; /* No grandparent means no uncle */

    if (n->parent == g->left)
        return g->right;
    else
        return g->left;
}

rb_node_t * sibling(rb_node_t * n)
{
    if (n == n->parent->left)
        return n->parent->right;
    else
        return n->parent->left;
}

static void map_rotate_left(map_t * this, rb_node_t * p)
{
    rb_node_t * g = p->parent, * child_left = p->right->left;
    
    if (!map_is_leaf(this, g))
    {
        if (g->left == p)
            g->left = p->right;
        else
            g->right = p->right;
    }
    else
        this->root = p->right;
    
    p->right->parent = g;
    
    p->parent = p->right;
    p->right->left = p;
    p->right = child_left;
    
    if (!map_is_leaf(this, child_left))
        child_left->parent = p;
}

static void map_rotate_right(map_t * this, rb_node_t * p)
{
    rb_node_t * g = p->parent, * child_right = p->left->right;
    
    if (!map_is_leaf(this, g))
    {
        if (g->left == p)
            g->left = p->left;
        else
            g->right = p->left;
    }
    else
        this->root = p->left;

    p->left->parent = g;
    
    p->parent = p->left;
    p->left->right = p;
    p->left = child_right;
    
    if (!map_is_leaf(this, child_right))
        child_right->parent = p;
}

/*
 * Tree rotations may have shifted the root pointer location,
 * so let's just iterate back up the tree to the proper root.
 */
static void map_update_root(map_t * this)
{
    rb_node_t * n = this->root;
    
    int i = 0;
    
    while (!map_is_leaf(this, n->parent))
    {
        /* printf("Updating root: %d\n", i); */
        n = n->parent;
        i++;
    }
        
    this->root = n;
}

static void map_transplant(map_t * this, rb_node_t * node, rb_node_t * map_successor)
{
    if (map_is_leaf(this, node->parent))
    {   /*puts("map_transplant - root");*/
        this->root = map_successor;
    }
    else
    {
        if (node == node->parent->right)
        {
            node->parent->right = map_successor;
        }
        else
        {
            node->parent->left = map_successor;
        }
    }
    
    map_successor->parent = node->parent;
}

static rb_node_t * map_successor(map_t * this, rb_node_t * n)
{
    rb_node_t * suc = &this->nil;
    
    if (!map_is_leaf(this, n->right))
    {
        suc = n->right;
        
        while (!map_is_leaf(this, suc->left))
            suc = suc->left;
    }
    else if (!map_is_leaf(this, n->parent))
    {
        while (!map_is_leaf(this, n->parent) && n->parent->right == n)
            n = n->parent;
            
        /* either NULL or the map_successor */
        suc = n->parent;
    }
    
    return suc;
}

static rb_node_t * map_predecessor(map_t * this, rb_node_t * n)
{
    rb_node_t * pre = &this->nil;
    
    if (!map_is_leaf(this, n->left))
    {
        pre = n->left;
        
        while (!map_is_leaf(this, pre->right))
            pre = pre->right;
    }
    else if (!map_is_leaf(this, n->parent))
    {
        while (!map_is_leaf(this, n->parent) && n->parent->left == n)
            n = n->parent;
            
        /* either NULL or the map_predecessor */
        pre = n->parent;
    }
    
    return pre;
}

static void map_insert_case1(map_t * this, rb_node_t * n)
{
    if (map_is_leaf(this, n->parent))
        n->color = RB_COLOR_BLACK;
    else
        map_insert_case2(this, n);
}

static void map_insert_case2(map_t * this, rb_node_t * n)
{
    if (n->parent->color == RB_COLOR_BLACK)
        return; /* Tree is still valid */
    else
        map_insert_case3(this, n);
}

static void map_insert_case3(map_t * this, rb_node_t * n)
{
    rb_node_t * u = map_uncle(this, n), *g;
 
    if (!map_is_leaf(this, u) && u->color == RB_COLOR_RED)
    {
        n->parent->color = RB_COLOR_BLACK;
        u->color = RB_COLOR_BLACK;
        g = map_grandparent(this, n);
        g->color = RB_COLOR_RED;
        map_insert_case1(this, g);
    }
    else
    {
        map_insert_case4(this, n);
    }
}

static void map_insert_case4(map_t * this, rb_node_t * n)
{
    rb_node_t * g = map_grandparent(this, n);
 
    if ((n == n->parent->right) && (n->parent == g->left))
    {
        map_rotate_left(this, n->parent);
        n = n->left; 
    }
    else if ((n == n->parent->left) && (n->parent == g->right))
    {
        map_rotate_right(this, n->parent);
        n = n->right; 
    }

    map_insert_case5(this, n);
}

static void map_insert_case5(map_t * this, rb_node_t * n)
{
    /*puts("-----");print_tree(this->root, stdout);puts("--");*/
    rb_node_t * g = map_grandparent(this, n);
 
    n->parent->color = RB_COLOR_BLACK;
    g->color = RB_COLOR_RED;
    if (n == n->parent->left)
        map_rotate_right(this, g);
    else
        map_rotate_left(this, g);
}

/* DELETE CASES */

static void map_delete_case1(map_t * this, rb_node_t * n)
{
    /*puts("delcase1");*/
    if (!map_is_leaf(this, n->parent))
        map_delete_case2(this, n);
}

static void map_delete_case2(map_t * this, rb_node_t * n)
{
    rb_node_t * s = sibling(n);
    /*printf("delcase2 - ");*/

    if (s->color == RB_COLOR_RED)
    {
        n->parent->color = RB_COLOR_RED;
        s->color = RB_COLOR_BLACK;
        /* printf("rotate"); */
        if (n == n->parent->left)
            map_rotate_left(this, n->parent);
        else
            map_rotate_right(this, n->parent);
    }
    /* printf("\n"); */
    map_delete_case3(this, n);
}

static void map_delete_case3(map_t * this, rb_node_t * n)
{
    rb_node_t * s = sibling(n);
    /*puts("delcase3");*/

    if (n->parent->color == RB_COLOR_BLACK
        && s->color == RB_COLOR_BLACK
        && s->left->color == RB_COLOR_BLACK
        && s->right->color == RB_COLOR_BLACK)
    {
        s->color = RB_COLOR_RED;
        map_delete_case1(this, n->parent);
    }
    else
    {
        map_delete_case4(this, n);
    }
}

static void map_delete_case4(map_t * this, rb_node_t * n)
{
    /* puts("delcase4"); */
    rb_node_t * s = sibling(n);
 
    if (n->parent->color == RB_COLOR_RED
        && s->color == RB_COLOR_BLACK
        && s->left->color == RB_COLOR_BLACK
        && s->right->color == RB_COLOR_BLACK)
    {
        s->color = RB_COLOR_RED;
        n->parent->color = RB_COLOR_BLACK;
    }
    else
        map_delete_case5(this, n);
}

static void map_delete_case5(map_t * this, rb_node_t * n)
{
    rb_node_t * s = sibling(n);
    /* printf("delcase5 - ");//printf("n->p - %p", n->parent); */
    if  (s->color == RB_COLOR_BLACK)
    {
        if (n == n->parent->left
            && s->right->color == RB_COLOR_BLACK
            && s->left->color == RB_COLOR_RED)
        {/* printf("l"); */
            s->color = RB_COLOR_RED;
            s->left->color = RB_COLOR_BLACK;
            map_rotate_right(this, s);
        }
        else if (n == n->parent->right
                && s->left->color == RB_COLOR_BLACK
                && s->right->color == RB_COLOR_RED)
        {/* printf("r"); */
            s->color = RB_COLOR_RED;
            s->right->color = RB_COLOR_BLACK;
            map_rotate_left(this, s);
        }
    }
    /* printf("\n"); */
    map_delete_case6(this, n);
}

static void map_delete_case6(map_t * this, rb_node_t * n)
{
    /* printf("delcase6 - "); */
    /* printf("n->p - %p", n->parent); */
    rb_node_t * s = sibling(n);
 
    s->color = n->parent->color;
    n->parent->color = RB_COLOR_BLACK;

    if (n == n->parent->left)
    {
        /*printf("l\n");*/
        s->right->color = RB_COLOR_BLACK;
        map_rotate_left(this, n->parent);
    }
    else
    {
        /*printf("r\n");*/
        s->left->color = RB_COLOR_BLACK;
        map_rotate_right(this, n->parent);
    }
}

static int map_key_cmp(map_t * this, map_datum_t key1, map_datum_t key2)
{
    if (this->key_cmp)
        return this->key_cmp(key1.p, key2.p);

    switch (this->key_type)
    {
        case MAP_KEY_TYPE_INT:
            if (key1.i == key2.i)
                return 0;
            else if (key1.i < key2.i)
                return -1;
            else
                return 1;
        case MAP_KEY_TYPE_DOUBLE:
            if (key1.d == key2.d)
                return 0;
            else if (key1.d < key2.d)
                return -1;
            else
                return 1;
        case MAP_KEY_TYPE_STRING:
            return strcmp(key1.p, key2.p);
    }
}
static void map_set_key_p(map_t * this, map_datum_t data, void * key)
{
    switch (this->key_type)
    {
        case MAP_KEY_TYPE_INT:
            *((int *) key) = data.i;
            break;
        case MAP_KEY_TYPE_DOUBLE:
            *((double *) key) = data.d;
            break;
        case MAP_KEY_TYPE_STRING:
            *((char **) key) = data.p;
            break;
    }
}

static void map_set_val_p(map_t * this, map_datum_t data, void * val)
{
    switch (this->key_type)
    {
        case MAP_VAL_TYPE_INT:
            *((int *) val) = data.i;
            break;
        case MAP_VAL_TYPE_DOUBLE:
            *((double *) val) = data.d;
            break;
        case MAP_VAL_TYPE_POINTER:
            *((void **) val) = data.p;
            break;
    }
}

/* inline */ static map_datum_t map_get_va_key(map_t * this, va_list ap)
{
    map_datum_t key;
    
    switch (this->key_type)
    {
        case MAP_KEY_TYPE_INT:
            key.i = va_arg(ap, int);
            break;
        case MAP_KEY_TYPE_DOUBLE:
            key.d = va_arg(ap, double);
            break;
        case MAP_KEY_TYPE_STRING:
            key.p = va_arg(ap, void *);
            break;
    }
    
    return key;
}
/* inline */ static map_datum_t map_get_va_val(map_t * this, va_list ap)
{
    map_datum_t val;
    
    switch (this->val_type)
    {
        case MAP_VAL_TYPE_INT:
            val.i = va_arg(ap, int);
            break;
        case MAP_VAL_TYPE_DOUBLE:
            val.d = va_arg(ap, double);
            break;
        case MAP_VAL_TYPE_POINTER:
            val.p = va_arg(ap, void *);
            break;
    }
    
    return val;
}
