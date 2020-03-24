#ifndef LLIST_H
#define LLIST_H

#include <stdlib.h>

typedef struct llist_t llist_t;
typedef struct llnode_t llnode_t;
typedef enum { NONE, DESC, ASC } order_type;

struct llist_t
{
  llnode_t * head;
  llnode_t * tail;
  order_type order;
  size_t sz;
};

struct llnode_t
{
  int data;
  llnode_t * next;
};

llist_t * create_llist(void);
llist_t * create_llist_with_order(order_type order);
void free_llist(llist_t * list);

llnode_t * create_llnode(int data);
void insert_llnode(llist_t * list, llnode_t * node);
void delete_llnode(llist_t * list, int data);

#endif /* LLIST_H */
