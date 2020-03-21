#include "../headers/llist.h"

#ifndef NULL
#define NULL (void *)0
#endif

/* Helper functions */
void insert(llist_t * list, llnode_t * node);
void insert_node_asc(llist_t * list, llnode_t * node);
void insert_node_desc(llist_t * list, llnode_t * node);
llnode_t * get_prev_of_node_with_data(llist_t * list, int data);


/*************************************/
/******* Function Definitions ********/ 
/*************************************/
llnode_t *
create_node(int data)
{
  llnode_t * n0 = (llnode_t *) malloc(sizeof(llnode_t));
  if (n0)
    n0->data = data;
  return n0;
}

void 
init(llist_t * list, order_type order)
{
  list->head = NULL;
  list->tail = NULL;
  list->order = order;
  list->sz = 0;
}

void
insert_node(llist_t * list, llnode_t * node)
{
  if (list && node)
  {
    if (list->order == ASC)
      insert_node_asc(list, node);

    else if (list->order == DESC)
      insert_node_desc(list, node);

    else
      insert(list, node);
  }
}

void
delete_node_with_data(llist_t * list, int data)
// Deletes first node containing data
{
/*
    llnode_t * prev_node = get_prev_of_node_with_data(list, data);
    llnode_t * del_node = get_node_with_data(list, data);
    llnode_t * next_node;

    if (del_node)
    // Edge cases:
    // 1. del_node = HEAD
    // 2. del_node = TAIL
    {
      next_node = del_node->next;

      if (prev_node == NULL) / Node to be deleted is HEAD /
      {
        
      }
      else if (next_node == NULL) / Node to be deleted is TAIL /
      {

      }
    }
*/
}

/*************************************/
/********* Helper Functions **********/ 
/*************************************/
void
insert(llist_t * list, llnode_t * node)
{
  if (list->sz == 0) /* list is empty */
    list->head = list->tail = node;

  else /* make tail -> node */
  {
    list->tail->next = node;
    list->tail = node;
  }
  list->sz++;
}

void
insert_node_asc(llist_t * list, llnode_t * node)
{
  int data = node->data;
  llnode_t * prev_node = get_prev_of_node_with_data(list, data);

  if (prev_node == NULL) /* Node to insert will be new HEAD */
  {
    node->next = list->head;
    list->head = node;
    if (list->sz == 0) /* Handles case where list is empty */
      list->tail = node;
  }
  else
  {
    if (prev_node->next == NULL) /* Node to insert will be new TAIL */
    {
      prev_node->next = node;
      list->tail = node;
    }
    else
    {
      node->next = prev_node->next;
      prev_node->next = node;
    }
  }
  list->sz++;
}

void
insert_node_desc(llist_t * list, llnode_t * node)
{
  int data = node->data;
  llnode_t * prev_node = get_prev_of_node_with_data(list, data);

  if (prev_node == NULL) /* Node to insert will be new HEAD */
  {
    node->next = list->head;
    list->head = node;
    if (list->sz == 0) /* Handles case where list is empty */
      list->tail = node;
  }
  else
  {
    if (prev_node->next == NULL) /* Node to insert will be new TAIL */
    {
      prev_node->next = node;
      list->tail = node;
    }
    else
    {
      node->next = prev_node->next;
      prev_node->next = node;
    }
  }
  list->sz++;
}

llnode_t *
get_prev_of_node_with_data(llist_t * list, int data)
{
  llnode_t * res = NULL;
  if (list->order == ASC)
  {
    res = list->head;
    if (res == NULL || res->data > data)
      return NULL;

    while (res->next && res->next->data <= data)
      res = res->next;
  }
  else if (list->order == DESC)
  {
    res = list->head;
    if (res == NULL || res->data < data)
      return NULL;

    while (res->next && res->next->data >= data)
      res = res->next;
  }
  return res;
}

