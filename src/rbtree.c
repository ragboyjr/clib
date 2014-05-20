#include "lib/rbtree.h"

#include <string.h>
#include <stdlib.h>
#include <assert.h>


/* used for debugging */
#include <stdio.h>
#include <math.h>

static rb_node_t * rb_tree_node_create(rb_tree_t * this, const rb_key_u key);
static void rb_tree_destroy_tree(rb_tree_t * this, rb_node_t * root);
/*static void tree_print_walk(rb_node_t * root, FILE * stream);*/
/*static void print_tree(rb_tree_t * this, rb_node_t * root, FILE * stream);*/
/*static void rb_tree_level(rb_tree_t * this, rb_node_t ** const nodes, int map_level, int level, rb_node_t * root, int start, int end); */

static int rb_tree_height(rb_tree_t * this, rb_node_t * root);

/* helper functions and macros */

#define is_leaf(t, n) (n == &(t)->nil)

static void rb_tree_check_pointers(rb_tree_t *, rb_node_t *);

static void rb_tree_transplant(rb_tree_t * this, rb_node_t * node, rb_node_t * rb_tree_successor);
static rb_node_t * rb_tree_grandparent(rb_tree_t * this, rb_node_t * n);
static rb_node_t * rb_tree_uncle(rb_tree_t * this, rb_node_t * n);

static void rb_tree_rotl(rb_tree_t * this, rb_node_t * p);
static void rb_tree_rotr(rb_tree_t * this, rb_node_t * p);
static void rb_tree_update_root(rb_tree_t * this);

static rb_node_t * rb_tree_successor(rb_tree_t * this, rb_node_t * n);
static rb_node_t * rb_tree_predecessor(rb_tree_t * this, rb_node_t * n);

static void rb_tree_replace_node(rb_tree_t * this, rb_node_t * n1, const rb_node_t * n2);

/* functions for balancing */

static void rb_tree_insert_case1(rb_tree_t * this, rb_node_t * n);
static void rb_tree_insert_case2(rb_tree_t * this, rb_node_t * n);
static void rb_tree_insert_case3(rb_tree_t * this, rb_node_t * n);
static void rb_tree_insert_case4(rb_tree_t * this, rb_node_t * n);
static void rb_tree_insert_case5(rb_tree_t * this, rb_node_t * n);

static void rb_tree_delete_case1(rb_tree_t * this, rb_node_t * n);
static void rb_tree_delete_case2(rb_tree_t * this, rb_node_t * n);
static void rb_tree_delete_case3(rb_tree_t * this, rb_node_t * n);
static void rb_tree_delete_case4(rb_tree_t * this, rb_node_t * n);
static void rb_tree_delete_case5(rb_tree_t * this, rb_node_t * n);
static void rb_tree_delete_case6(rb_tree_t * this, rb_node_t * n);

void rb_tree_init(
    rb_tree_t * this,
    int node_size,
    int (* key_cmp)(const rb_key_u, const rb_key_u, void *),
    void * key_cmp_state
)
{  
    /* initialize the values */
    memset(&this->nil, 0, sizeof(rb_node_t));
    this->nil.color = RB_COLOR_BLACK;
    
    this->root          = &this->nil,
    this->iter_p        = &this->nil,
    this->count         = 0,
    this->node_size     = node_size,
    this->key_cmp       = key_cmp,
    this->key_cmp_state = key_cmp_state;
}

void rb_tree_free(rb_tree_t * this)
{
    /* free the nodes */
    rb_tree_destroy_tree(this, this->root);
}

rb_node_t * rb_tree_search(rb_tree_t * this, const rb_key_u key)
{
    int cmp_res;
    rb_node_t * p = this->root;
    
    while (!is_leaf(this, p))
    {
        /* debugging to make sure the pointers are setup correctly */
        rb_tree_check_pointers(this, p);
        
        cmp_res = this->key_cmp(p->key, key, this->key_cmp_state);
        
        if (cmp_res > 0)
            p = p->left;
        else if (cmp_res < 0)
            p = p->right;
        else
            return p;
    }
    
    return NULL;
}

rb_node_t * rb_tree_insert(rb_tree_t * this, const rb_key_u key, int * was_inserted)
{
    int cmp_res;
    rb_node_t * n,
              * parent = NULL;
    
    assert(was_inserted);
    *was_inserted = 0;
    
    if (is_leaf(this, this->root)) {
        /* insert the root */
        n = this->root = rb_tree_node_create(this, key);
    }
    else
    {
        n = this->root;
        
        while (!is_leaf(this, n))
        {
            cmp_res = this->key_cmp(n->key, key, this->key_cmp_state);

            parent = n;

            if (cmp_res > 0)
                n = n->left;
            else if (cmp_res < 0)
                n = n->right;
            else /* match */
                return n;
        }
    
        /* the node wasn't found, so we insert to the parent's left or right */
        n = rb_tree_node_create(this, key);
        n->parent = parent;
    
        if (cmp_res > 0)
            parent->left = n;
        else
            parent->right = n;
    }
    
    if (is_leaf(this, n)) {
        assert(0);
    }
        
    /* let's balance the tree now */
    this->count++;
    
    rb_tree_insert_case1(this, n);
    rb_tree_update_root(this);

    *was_inserted = 1;

    return n;
}

rb_node_t * rb_tree_delete(rb_tree_t * this, const rb_key_u key)
{
    rb_node_t * n,
              * s = &this->nil,
              * child;
    
    n = rb_tree_search(this, key);
    
    if (!n) {
        return NULL;
    }
    
    /* find the successor (which could be the in-order predecessor or in-order successor) */
    
    if (!is_leaf(this, n->right))
    {
        s = n->right;
        while (!is_leaf(this, s->left))
            s = s->left;
    }
    else if (!is_leaf(this, n->left))
    {
        s = n->left;
        while (!is_leaf(this, s->right))
            s = s->right;
    }

    if (!is_leaf(this, s)) {
        /* puts("not leaf"); */
        /* swap the values */
        rb_tree_replace_node(this, n, s);
    }
    else {   /* if this is the case */
        s = n;  /* n is it's own rb_tree_successor */
    }
    
    /* printf("s = %d, s->p = %p, n = %p\n", s->key.i, s->parent, n); */
    
    /* assumption: s points to the node that needs to be deleted */
    child = (is_leaf(this, s->right)) ? s->left : s->right;
        
    rb_tree_transplant(this, s, child);
    
    if (s->color == RB_COLOR_BLACK)
    {
        if (child->color == RB_COLOR_RED)
            child->color = RB_COLOR_BLACK;
        else
            rb_tree_delete_case1(this, child);
    }
    
    this->count--;
    
    /* return s to be freed */
    return s;
}


/*void map_print(rb_tree_t * this, FILE * stream)
{
    print_tree(this, this->root, stream);   
}*/

void rb_tree_reset(rb_tree_t * this)
{
    rb_node_t * n = this->root;
    
    if (!is_leaf(this, n))
    {
        while (!is_leaf(this, n->left))
            n = n->left;
    }
    
    this->iter_p = n;   /* the smallest node */
}

void rb_tree_next(rb_tree_t * this)
{
    this->iter_p = rb_tree_successor(this, this->iter_p);
}

static void rb_tree_check_pointers(rb_tree_t * this, rb_node_t * n)
{
    if (!is_leaf(this, n->left))
        assert(n->left->parent == n);
    if (!is_leaf(this, n->right))
        assert(n->right->parent == n);
}

static void rb_tree_destroy_tree(rb_tree_t * this, rb_node_t * root)
{
    if (!is_leaf(this, root))
    {
        rb_tree_destroy_tree(this, root->left);
        rb_tree_destroy_tree(this, root->right);
        free(root);
    }
}

/*static void tree_print_walk(rb_node_t * root, FILE * stream)
{
    if (!is_leaf(this, root))
    {
        tree_print_walk(root->left, stream);
        fprintf(stream, "key: %s, value: %s\n", (char *) root->key, (char *) root->value);
        tree_print_walk(root->right, stream);
    }
}*/

static int rb_tree_height(rb_tree_t * this, rb_node_t * root)
{
    if (!is_leaf(this, root))
    {
        int maxl = rb_tree_height(this, root->left) + 1;
        int maxr = rb_tree_height(this, root->right) + 1;
        
        return (maxl > maxr) ? maxl : maxr;
    }
    
    return 0;
}

static void rb_tree_level(rb_tree_t * this, rb_node_t ** const nodes, int map_level, int level, rb_node_t * root, int start, int end)
{
    int mid = (end - start) / 2 + start;
 
    if (!is_leaf(this, root))
    {
        if (map_level != level)
        {
            rb_tree_level(this, nodes, map_level, level + 1, root->left, start, mid);
            rb_tree_level(this, nodes, map_level, level + 1, root->right, mid + 1, end);   /* go to mid + 1 because I'm implicitly flooring mid */
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

/*static void print_tree(rb_tree_t * this, rb_node_t * root, FILE * stream)
{
    int height = rb_tree_height(this, root);
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
        rb_tree_level(this, nodes, i, 0, root, 0, level_width - 1);

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

static rb_node_t * rb_tree_node_create(rb_tree_t * this, const rb_key_u key)
{
    rb_node_t * n = malloc(this->node_size);
    
    n->color    = RB_COLOR_RED;
    n->key      = key;
    n->left     = &this->nil;
    n->right    = &this->nil;
    n->parent   = &this->nil;
    
    return n;
}

static void rb_tree_replace_node(rb_tree_t * this, rb_node_t * n1, const rb_node_t * n2)
{
    /* double check that this works like it should */
    
    /* we need to typecast these to char because we need to add bytes to the pointer values
       and only a char is defined as one byte */
    char * c_n1 = (char *) n1,
         * c_n2 = (char *) n2;
    
    /* because every node that uses the rb_node will have the rb_node as the first
       member of the struct, we can assume, that everything after the rb_node data
       belongs to the encapsulating struct, we replace the rb_node's data AND the encapsulating
       struct's data */
    
    assert(n1);
    assert(n2);
    
    /* replace the keys */
    n1->key = n2->key;
    
    assert(this->node_size >= sizeof(rb_node_t));
    
    /* swap any extra data over */
    memcpy(c_n1 + sizeof(rb_node_t), c_n2 + sizeof(rb_node_t), this->node_size - sizeof(rb_node_t));
}

static rb_node_t * rb_tree_grandparent(rb_tree_t * this, rb_node_t * n)
{
    if ((!is_leaf(this, n)) && (!is_leaf(this, n->parent)))
        return n->parent->parent;
    else
        return &this->nil;
}

static rb_node_t * rb_tree_uncle(rb_tree_t * this, rb_node_t * n)
{
    rb_node_t * g = rb_tree_grandparent(this, n);
    
    if (is_leaf(this, g))
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

static void rb_tree_rotl(rb_tree_t * this, rb_node_t * p)
{
    rb_node_t * g = p->parent, * child_left = p->right->left;
    
    if (!is_leaf(this, g))
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
    
    if (!is_leaf(this, child_left))
        child_left->parent = p;
}

static void rb_tree_rotr(rb_tree_t * this, rb_node_t * p)
{
    rb_node_t * g = p->parent, * child_right = p->left->right;
    
    if (!is_leaf(this, g))
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
    
    if (!is_leaf(this, child_right))
        child_right->parent = p;
}

/*
 * Tree rotations may have shifted the root pointer location,
 * so let's just iterate back up the tree to the proper root.
 */
static void rb_tree_update_root(rb_tree_t * this)
{
    rb_node_t * n = this->root;
    
    int i = 0;
    
    while (!is_leaf(this, n->parent))
    {
        /* printf("Updating root: %d\n", i); */
        n = n->parent;
        i++;
    }
        
    this->root = n;
}

static void rb_tree_transplant(rb_tree_t * this, rb_node_t * node, rb_node_t * rb_tree_successor)
{
    if (is_leaf(this, node->parent))
    {   /*puts("rb_tree_transplant - root");*/
        this->root = rb_tree_successor;
    }
    else
    {
        if (node == node->parent->right)
        {
            node->parent->right = rb_tree_successor;
        }
        else
        {
            node->parent->left = rb_tree_successor;
        }
    }
    
    rb_tree_successor->parent = node->parent;
}

static rb_node_t * rb_tree_successor(rb_tree_t * this, rb_node_t * n)
{
    rb_node_t * suc = &this->nil;
    
    if (!is_leaf(this, n->right))
    {
        suc = n->right;
        
        while (!is_leaf(this, suc->left))
            suc = suc->left;
    }
    else if (!is_leaf(this, n->parent))
    {
        while (!is_leaf(this, n->parent) && n->parent->right == n)
            n = n->parent;
            
        /* either NULL or the rb_tree_successor */
        suc = n->parent;
    }
    
    return suc;
}

static rb_node_t * rb_tree_predecessor(rb_tree_t * this, rb_node_t * n)
{
    rb_node_t * pre = &this->nil;
    
    if (!is_leaf(this, n->left))
    {
        pre = n->left;
        
        while (!is_leaf(this, pre->right))
            pre = pre->right;
    }
    else if (!is_leaf(this, n->parent))
    {
        while (!is_leaf(this, n->parent) && n->parent->left == n)
            n = n->parent;
            
        /* either NULL or the rb_tree_predecessor */
        pre = n->parent;
    }
    
    return pre;
}

static void rb_tree_insert_case1(rb_tree_t * this, rb_node_t * n)
{
    if (is_leaf(this, n->parent))
        n->color = RB_COLOR_BLACK;
    else
        rb_tree_insert_case2(this, n);
}

static void rb_tree_insert_case2(rb_tree_t * this, rb_node_t * n)
{
    if (n->parent->color == RB_COLOR_BLACK)
        return; /* Tree is still valid */
    else
        rb_tree_insert_case3(this, n);
}

static void rb_tree_insert_case3(rb_tree_t * this, rb_node_t * n)
{
    rb_node_t * u = rb_tree_uncle(this, n), *g;
 
    if (!is_leaf(this, u) && u->color == RB_COLOR_RED)
    {
        n->parent->color = RB_COLOR_BLACK;
        u->color = RB_COLOR_BLACK;
        g = rb_tree_grandparent(this, n);
        g->color = RB_COLOR_RED;
        rb_tree_insert_case1(this, g);
    }
    else
    {
        rb_tree_insert_case4(this, n);
    }
}

static void rb_tree_insert_case4(rb_tree_t * this, rb_node_t * n)
{
    rb_node_t * g = rb_tree_grandparent(this, n);
 
    if ((n == n->parent->right) && (n->parent == g->left))
    {
        rb_tree_rotl(this, n->parent);
        n = n->left; 
    }
    else if ((n == n->parent->left) && (n->parent == g->right))
    {
        rb_tree_rotr(this, n->parent);
        n = n->right; 
    }

    rb_tree_insert_case5(this, n);
}

static void rb_tree_insert_case5(rb_tree_t * this, rb_node_t * n)
{
    /*puts("-----");print_tree(this->root, stdout);puts("--");*/
    rb_node_t * g = rb_tree_grandparent(this, n);
 
    n->parent->color = RB_COLOR_BLACK;
    g->color = RB_COLOR_RED;
    if (n == n->parent->left)
        rb_tree_rotr(this, g);
    else
        rb_tree_rotl(this, g);
}

/* DELETE CASES */

static void rb_tree_delete_case1(rb_tree_t * this, rb_node_t * n)
{
    /*puts("delcase1");*/
    if (!is_leaf(this, n->parent))
        rb_tree_delete_case2(this, n);
}

static void rb_tree_delete_case2(rb_tree_t * this, rb_node_t * n)
{
    rb_node_t * s = sibling(n);
    /*printf("delcase2 - ");*/

    if (s->color == RB_COLOR_RED)
    {
        n->parent->color = RB_COLOR_RED;
        s->color = RB_COLOR_BLACK;
        /* printf("rotate"); */
        if (n == n->parent->left)
            rb_tree_rotl(this, n->parent);
        else
            rb_tree_rotr(this, n->parent);
    }
    /* printf("\n"); */
    rb_tree_delete_case3(this, n);
}

static void rb_tree_delete_case3(rb_tree_t * this, rb_node_t * n)
{
    rb_node_t * s = sibling(n);
    /*puts("delcase3");*/

    if (n->parent->color == RB_COLOR_BLACK
        && s->color == RB_COLOR_BLACK
        && s->left->color == RB_COLOR_BLACK
        && s->right->color == RB_COLOR_BLACK)
    {
        s->color = RB_COLOR_RED;
        rb_tree_delete_case1(this, n->parent);
    }
    else
    {
        rb_tree_delete_case4(this, n);
    }
}

static void rb_tree_delete_case4(rb_tree_t * this, rb_node_t * n)
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
        rb_tree_delete_case5(this, n);
}

static void rb_tree_delete_case5(rb_tree_t * this, rb_node_t * n)
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
            rb_tree_rotr(this, s);
        }
        else if (n == n->parent->right
                && s->left->color == RB_COLOR_BLACK
                && s->right->color == RB_COLOR_RED)
        {/* printf("r"); */
            s->color = RB_COLOR_RED;
            s->right->color = RB_COLOR_BLACK;
            rb_tree_rotl(this, s);
        }
    }
    /* printf("\n"); */
    rb_tree_delete_case6(this, n);
}

static void rb_tree_delete_case6(rb_tree_t * this, rb_node_t * n)
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
        rb_tree_rotl(this, n->parent);
    }
    else
    {
        /*printf("r\n");*/
        s->left->color = RB_COLOR_BLACK;
        rb_tree_rotr(this, n->parent);
    }
}
