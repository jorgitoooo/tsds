#ifndef LLIST_H
#define LLIST_H

#include <stdlib.h>

typedef struct llist_t llist_t;
typedef struct llnode_t llnode_t;
typedef enum { ASC, DESC, NONE } order_type_t;

struct llist_t
{
  llnode_t * head;        /* First element        */
  llnode_t * tail;        /* Last element         */
  order_type_t order;     /* Element ordering     */
  size_t sz;              /* Size of linked list  */
};

struct llnode_t
{
  int data;
  llnode_t * next;
};

llnode_t * llnode_create(int data);
void llnode_free(llnode_t * llnode);

llist_t * llist_create(void);
llist_t * llist_create_with_order(order_type_t order);
void llist_free(llist_t * llist);
void llist_insert(llist_t * llist, llnode_t * llnode);
void llist_delete(llist_t * llist, int data);
void llist_sort(llist_t * llist, order_type_t order);
void llist_change_order(llist_t * llist, order_type_t order);
llnode_t * llist_at(llist_t * llist, size_t idx);
llnode_t * llist_get(llist_t * llist, int data);

#endif /* LLIST_H */
