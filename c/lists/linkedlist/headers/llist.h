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
void llnode_free(llnode_t * node);

llist_t * llist_create(void);
llist_t * llist_create_with_order(order_type_t order);
void llist_free(llist_t * list);
void llist_insert(llist_t * list, llnode_t * node);
void llist_delete(llist_t * list, int data);
void llist_sort(llist_t * list, order_type_t order);
void llist_change_order(llist_t * list, order_type_t order);
llnode_t * llist_at(llist_t * list, size_t idx);
llnode_t * llist_get(llist_t * list, int data);

#endif /* LLIST_H */
