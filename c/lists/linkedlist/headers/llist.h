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

llnode_t * llnode_create(int data);

llist_t * llist_create(void);
llist_t * llist_create_with_order(order_type order);
void llist_free(llist_t * list);
void llist_insert(llist_t * list, llnode_t * node);
void llist_delete(llist_t * list, int data);
llnode_t * llist_at(llist_t * list, size_t idx);

#endif /* LLIST_H */
