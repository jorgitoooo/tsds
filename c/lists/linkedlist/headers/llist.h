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

void init(llist_t * list);
void init_with_order(llist_t * list, order_type order);
void insert_node(llist_t * list, llnode_t * node);
void delete_node(llist_t * list, int data);

llnode_t * create_node(int data);

#endif /* LLIST_H */
