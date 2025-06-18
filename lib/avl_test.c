#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include "avl_tree.h"  // header for AVL implementation

/*
 * Test harness for AVL tree implementation
 * - Inserts a series of keys
 * - Prints in-order traversal
 * - Deletes selected keys
 * - Prints in-order again
 */

/* Container struct embedding avl_node */
struct test_node {
    int key;
    struct avl_node avl;
};

/* Compare and link helper: finds position to insert */
void avl_insert_key(struct avl_root *root, struct test_node *tn) {
    struct avl_node *parent = NULL;
    struct avl_node **link = &root->avl_node;
    struct test_node *iter;

    while (*link) {
        parent = *link;
        iter = container_of(parent, struct test_node, avl);
        if (tn->key < iter->key) {
            link = &parent->avl_left;
        } else {
            link = &parent->avl_right;
        }
    }
    /* link the new node */
    avl_link_node(&tn->avl, root, parent, (link == &parent->avl_left));
    avl_insert_color(&tn->avl, root);
}

/* In-order traversal */
void print_inorder(struct avl_root *root) {
    struct avl_node *n;
    for (n = avl_first(root); n; n = avl_next(n)) {
        struct test_node *tn = container_of(n, struct test_node, avl);
        printf("%d ", tn->key);
    }
    printf("\n");
}

int main(void) {
    struct avl_root tree = { .avl_node = NULL };
    int keys[] = { 50, 20, 70, 10, 30, 60, 80, 25, 35 };
    const int nkeys = sizeof(keys)/sizeof(*keys);
    struct test_node *nodes[nkeys];

    /* Allocate and insert nodes */
    for (int i = 0; i < nkeys; ++i) {
        nodes[i] = malloc(sizeof(*nodes[i]));
        if (!nodes[i]) {
            perror("malloc");
            return 1;
        }
        nodes[i]->key = keys[i];
        avl_insert_key(&tree, nodes[i]);
    }

    printf("In-order after inserts: ");
    print_inorder(&tree);

    /* Delete a few keys */
    int to_delete[] = { 20, 70, 25 };
    const int ndel = sizeof(to_delete)/sizeof(*to_delete);
    for (int i = 0; i < ndel; ++i) {
        /* search for node */
        struct avl_node *n = tree.avl_node;
        while (n) {
            struct test_node *tn = container_of(n, struct test_node, avl);
            if (to_delete[i] == tn->key) {
                avl_erase(n, &tree);
                free(tn);
                break;
            } else if (to_delete[i] < tn->key) {
                n = n->avl_left;
            } else {
                n = n->avl_right;
            }
        }
    }

    printf("In-order after deletes: ");
    print_inorder(&tree);

    /* Cleanup remaining nodes */
    struct avl_node *n = avl_first(&tree);
    while (n) {
        struct test_node *tn = container_of(n, struct test_node, avl);
        struct avl_node *next = avl_next(n);
        avl_erase(n, &tree);
        free(tn);
        n = next;
    }

    return 0;
}
