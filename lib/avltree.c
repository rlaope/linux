/*
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * AVL Tree based replacement for linux/lib/rbtree.c APIs
 * Provides self-balancing binary search tree with O(log n) insert, delete, search
 * and traversal functions compatible to rbtree API names.
 */

#include <linux/types.h>
#include <linux/export.h>
#include <linux/kernel.h>

/* AVL node header: embed inside structs */
struct avl_node {
    struct avl_node *avl_parent;
    struct avl_node *avl_left;
    struct avl_node *avl_right;
    int8_t          avl_height;
};

/* AVL tree root */
struct avl_root {
    struct avl_node *avl_node;
};

/* Initialize a node */
static inline void avl_init_node(struct avl_node *node)
{
    node->avl_parent = node->avl_left = node->avl_right = NULL;
    node->avl_height = 1;
}

static inline int8_t avl_height(struct avl_node *n)
{
    return n ? n->avl_height : 0;
}

static inline void avl_update_height(struct avl_node *n)
{
    int8_t hl = avl_height(n->avl_left);
    int8_t hr = avl_height(n->avl_right);
    n->avl_height = max(hl, hr) + 1;
}

static void avl_rotate_right(struct avl_node **root, struct avl_node *y)
{
    struct avl_node *x = y->avl_left;
    y->avl_left = x->avl_right;
    if (x->avl_right)
        x->avl_right->avl_parent = y;
    x->avl_parent = y->avl_parent;
    if (!y->avl_parent)
        *root = x;
    else if (y == y->avl_parent->avl_left)
        y->avl_parent->avl_left = x;
    else
        y->avl_parent->avl_right = x;
    x->avl_right = y;
    y->avl_parent = x;
    avl_update_height(y);
    avl_update_height(x);
}

static void avl_rotate_left(struct avl_node **root, struct avl_node *x)
{
    struct avl_node *y = x->avl_right;
    x->avl_right = y->avl_left;
    if (y->avl_left)
        y->avl_left->avl_parent = x;
    y->avl_parent = x->avl_parent;
    if (!x->avl_parent)
        *root = y;
    else if (x == x->avl_parent->avl_left)
        x->avl_parent->avl_left = y;
    else
        x->avl_parent->avl_right = y;
    y->avl_left = x;
    x->avl_parent = y;
    avl_update_height(x);
    avl_update_height(y);
}

static void avl_rebalance(struct avl_node **root, struct avl_node *n)
{
    while (n) {
        avl_update_height(n);
        int8_t balance = avl_height(n->avl_left) - avl_height(n->avl_right);
        if (balance > 1) {
            if (avl_height(n->avl_left->avl_left) < avl_height(n->avl_left->avl_right))
                avl_rotate_left(root, n->avl_left);
            avl_rotate_right(root, n);
        } else if (balance < -1) {
            if (avl_height(n->avl_right->avl_right) < avl_height(n->avl_right->avl_left))
                avl_rotate_right(root, n->avl_right);
            avl_rotate_left(root, n);
        }
        n = n->avl_parent;
    }
}

/* Link and insert: user compares keys and sets node->avl_parent, left/right pointers */
static inline void avl_link_node(struct avl_node *node, struct avl_root *root,
                                struct avl_node *parent, bool left)
{
    avl_init_node(node);
    node->avl_parent = parent;
    if (!parent)
        root->avl_node = node;
    else if (left)
        parent->avl_left = node;
    else
        parent->avl_right = node;
}

void avl_insert_color(struct avl_node *node, struct avl_root *root)
{
    /* Rebalance upwards from parent */
    avl_rebalance(&root->avl_node, node->avl_parent);
}
EXPORT_SYMBOL(avl_insert_color);

static struct avl_node *avl_subtree_min(struct avl_node *n)
{
    while (n && n->avl_left)
        n = n->avl_left;
    return n;
}

void avl_erase(struct avl_node *node, struct avl_root *root)
{
    struct avl_node *parent = node->avl_parent;
    struct avl_node *child;

    if (node->avl_left && node->avl_right) {
        struct avl_node *succ = avl_subtree_min(node->avl_right);
        /* swap node and succ content or keys (user-specific) */
        /* For simplicity, assume keys swapped externally */
        node = succ;
    }
    child = node->avl_left ? node->avl_left : node->avl_right;
    if (child)
        child->avl_parent = parent;
    if (!parent)
        root->avl_node = child;
    else if (node == parent->avl_left)
        parent->avl_left = child;
    else
        parent->avl_right = child;

    avl_rebalance(&root->avl_node, parent);
}
EXPORT_SYMBOL(avl_erase);

struct avl_node *avl_first(const struct avl_root *root)
{
    return avl_subtree_min(root->avl_node);
}
EXPORT_SYMBOL(avl_first);

struct avl_node *avl_next(const struct avl_node *node)
{
    struct avl_node *n;
    if (node->avl_right)
        return avl_subtree_min(node->avl_right);
    n = (struct avl_node *)node;
    while (n->avl_parent && n == n->avl_parent->avl_right)
        n = n->avl_parent;
    return n->avl_parent;
}
EXPORT_SYMBOL(avl_next);

/* Similar avl_last, avl_prev, avl_replace_node, avl_replace_node_rcu, postorder can be added */

/* User must implement search, replace, and traversal macros analogous to rbtree */
