#include "../headers/utils.h"

void print_subtree_inorder(node_t *n)
{
  if(n)
  {
    print_subtree_inorder(n->left);
    printf(" %d ", n->key);
    print_subtree_inorder(n->right);
  }
}

void print_subtree_postorder(node_t *n)
{
  if(n)
  {
    print_subtree_postorder(n->left);
    print_subtree_postorder(n->right);
    printf(" %d ", n->key);
  }
}

void print_subtree_preorder(node_t *n)
{
  if(n)
  {
    printf(" %d ", n->key);
    print_subtree_preorder(n->left);
    print_subtree_preorder(n->right);
  }
}

void print(bintree_t *bt, traversal order)
{
  if(bt)
  {
    if(order == PRE)
    {
      printf("preorder : ");
      print_subtree_preorder(bt->head);
    }
    if(order == IN)
    {
      printf("inorder  : ");
      print_subtree_inorder(bt->head);
    }
    if(order == POST)
    {
      printf("postorder: ");
      print_subtree_postorder(bt->head);
    }
    printf("\n");
  }
}


void print_node(node_t *n)
{
  if(n)
    printf("node: key(%d), value(\"%s\")\n", n->key, n->value);
  else
    printf("node: NULL\n");
}
