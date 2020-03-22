#include <stdio.h>

#include "../headers/llist.h"
#include "../headers/utils.h"

#ifndef NULL
#define NULL (void *)0
#endif

/* Helper functions */
void insert_node_asc(llist_t * list, llnode_t * node);
void insert_node_desc(llist_t * list, llnode_t * node);
void insert_node_unordered(llist_t * list, llnode_t * node);
llnode_t * get_prev_node(llist_t * list, int data);
llnode_t * extract_node(llist_t * list, int data);


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
init(llist_t * list)
{
  list->head = NULL;
  list->tail = NULL;
  list->sz = 0;
}

void 
init_with_order(llist_t * list, order_type order)
{
  init(list);
  list->order = order;
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
      insert_node_unordered(list, node);

    list->sz++;
  }
}

void
delete_node(llist_t * list, int data)
// Deletes first node containing data
{
  if (list)
  {
    llnode_t * extracted_node = extract_node(list, data);

    if (extracted_node)
    {
      printf("Deleting node with data %d\n", extracted_node->data);
      list->sz--;
      free(extracted_node);
    }
  }
}

// TODO : Not done
llnode_t *
extract_node(llist_t * list, int data)
{
  llnode_t * prev_node = list->head;

  if (list->head == NULL)
    return NULL;

  if (list->head->data == data) /* Checks if HEAD is node to remove */
  {
    if (list->sz == 1)
      list->head = list->tail = NULL;

    else
      list->head = list->head->next;

    return prev_node;
  }

  while (prev_node->next && prev_node->next->data != data)
    prev_node = prev_node->next;

  llnode_t * extracted_node = prev_node->next;
  if (prev_node->next && prev_node->next->data == data)
    prev_node->next = prev_node->next->next;

  if (extracted_node == list->tail) /* Handles case where extracted node is TAIL */
    list->tail = prev_node;

  return extracted_node;
}

/*************************************/
/********* Helper Functions **********/ 
/*************************************/
void
insert_node_unordered(llist_t * list, llnode_t * node)
// Appends node to end of linked list.
// This function should not be called if list or node
// are NULL.
{
  if (list->sz == 0) /* list is empty */
    list->head = list->tail = node;

  else /* make tail -> node */
  {
    list->tail->next = node;
    list->tail = node;
  }
}

void
insert_node_asc(llist_t * list, llnode_t * node)
// Inserts node into linked list in ascending order.
// This function should not be called if list or node
// are NULL.
{
  int data = node->data;
  llnode_t * prev_node = get_prev_node(list, data);

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
}

void
insert_node_desc(llist_t * list, llnode_t * node)
// Inserts node into linked list in descending order
// This function should not be called if list or node
// are NULL.
{
  int data = node->data;
  llnode_t * prev_node = get_prev_node(list, data);

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
}

llnode_t *
get_prev_node(llist_t * list, int data)
{
  llnode_t * res = NULL;
  llnode_t * head = list->head;
  if (head)
  {
    if (list->order == ASC &&
        head->data <= data)
    {
      res = head;
      while (res->next && res->next->data <= data)
        res = res->next;
    }
    else if (list->order == DESC &&
             head->data >= data)
    {
      res = head;
      while (res->next && res->next->data >= data)
        res = res->next;
    }
    else if (list->order == NONE &&
             head->data != data)
    {
      res = head;
      while (res->next && res->next->data != data)
        res = res->next;
    }
  }

  return res;
}
