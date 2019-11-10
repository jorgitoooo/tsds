#ifndef _UTILS_H_
#define _UTILS_H_

#include <stdio.h>
#include "bintree.h"

typedef enum traversal { PRE, POST, IN } traversal;

// Print ops
void print_node(node_t *n);
void print(bintree_t *bt, traversal order);
void print_subtree_inorder(node_t *n);
void print_subtree_postorder(node_t *n);
void print_subtree_preorder(node_t *n);

#endif
