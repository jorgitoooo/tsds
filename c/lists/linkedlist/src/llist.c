#include <stdio.h>

#include "../headers/llist.h"
#include "../headers/utils.h"

/* Helper functions */
static void llist_init(llist_t *);
static void llist_init_with_order(llist_t *, order_type);
static void llnode_free(llnode_t *);
static void llist_insert_asc(llist_t *, llnode_t *);
static void llist_insert_desc(llist_t *, llnode_t *);
static void llist_insert_unordered(llist_t *, llnode_t *);
static llnode_t * llist_get_prev_llnode(llist_t *, int);
static llnode_t * llist_extract_llnode(llist_t *, int);
static llnode_t * llist_get_llnode_at(llist_t * list, size_t idx);

/*-----------------------------------*/
/* Function Definitions              */
/*-----------------------------------*/

llist_t *
llist_create(void)
// Creates a new linked list with a default order_type
// of unordered.
{
  llist_t * list = (llist_t *) malloc(sizeof(llist_t));
  if (list)
    llist_init(list);
  return list;
}

llist_t *
llist_create_with_order(order_type order)
// Creates a new linked list with the specified order_type.
{
  llist_t * list = (llist_t *) malloc(sizeof(llist_t));
  if (list)
    llist_init_with_order(list, order);
  return list;
}

llnode_t *
llnode_create(int data)
// Creates a new linked list node
{
  llnode_t * node = (llnode_t *) malloc(sizeof(llnode_t));
  if (node)
  {
    node->data = data;
    node->next = NULL;
  }
  return node;
}

void
llist_free(llist_t * list)
// Frees memory allocated for list
{
  if (list == NULL)
    return;

  llnode_t * cur = list->head;
  while (cur)
  {
    llnode_t * node_to_free = cur;
    cur = cur->next;
    llnode_free(node_to_free);
  }
  list->sz = 0;
  free(list);
}

void
llist_insert(llist_t * list, llnode_t * node)
// Inserts the linked list node referenced by node into 
// the linked list in accordance to to the list's order
// type and increments the size of the linked list by
// one.
{
  if (list && node)
  {
    if (list->order == ASC)
      llist_insert_asc(list, node);

    else if (list->order == DESC)
      llist_insert_desc(list, node);

    else
      llist_insert_unordered(list, node);

    list->sz++;
  }
}

void
llist_delete(llist_t * list, int data)
// Deletes first node in the linked list that contains 
// data and decrements linked list size by one.
{
  if (list)
  {
    llnode_t * extracted_node = llist_extract_llnode(list, data);

    if (extracted_node)
    {
      if (extracted_node->data != data) /* Internal error */
        return;

      llnode_free(extracted_node);
      list->sz--;
    }
  }
}

llnode_t * 
llist_at(llist_t * list, size_t idx)
// Returns the linked list node at position idx if idx 
// is within bounds and list is not NULL. Else, returns
// NULL.
{
  if (list
      && idx >= 0
      && idx < list->sz)
  {
    return llist_get_llnode_at(list, idx);
  }
  return NULL;
}

/*-----------------------------------*/
/* Helper Functions                  */ 
/*-----------------------------------*/

static void 
llist_init(llist_t * list)
// Initializes linked list and sets order_type to
// unordered. list should always be a valid pointer.
{
  list->head = NULL;
  list->tail = NULL;
  list->order = NONE;
  list->sz = 0;
}

static void 
llist_init_with_order(llist_t * list, order_type order)
// Initializes linked list and sets order_type to
// specified order. list should always be a valid
// pointer.
{
  llist_init(list);
  list->order = order;
}

static void
llnode_free(llnode_t * node)
// Frees linked list node if it exists.
{
  if (node == NULL)
    return;

  node->data = 0;
  node->next = NULL;
  free(node);
}


static void
llist_insert_unordered(llist_t * list, llnode_t * node)
// Appends linked list node to end of linked list.
// This function should not be called if list or
// node are NULL.
{
  if (list->sz == 0) /* list is empty */
    list->head = list->tail = node;

  else /* make tail -> node */
  {
    list->tail->next = node;
    list->tail = node;
  }
}

static void
llist_insert_asc(llist_t * list, llnode_t * node)
// Inserts node into linked list in ascending order.
// This function should not be called if list or node
// are NULL.
{
  int data = node->data;
  llnode_t * prev_node = llist_get_prev_llnode(list, data);

  /* Node to insert will be new HEAD */
  if (prev_node == NULL) 
  {
    node->next = list->head;
    list->head = node;

    /* Handles case where list is empty */
    if (list->sz == 0) 
      list->tail = node;
  }
  else
  {
    /* Node to insert will be new TAIL */
    if (prev_node->next == NULL) 
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

static void
llist_insert_desc(llist_t * list, llnode_t * node)
// Inserts node into linked list in descending order
// This function should not be called if list or node
// are NULL.
{
  int data = node->data;
  llnode_t * prev_node = llist_get_prev_llnode(list, data);

  /* Node to insert will be new HEAD */
  if (prev_node == NULL) 
  {
    node->next = list->head;
    list->head = node;

    /* Handles case where list is empty */
    if (list->sz == 0) 
      list->tail = node;
  }
  else
  {
    /* Node to insert will be new TAIL */
    if (prev_node->next == NULL) 
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

static llnode_t *
llist_get_prev_llnode(llist_t * list, int data)
// Returns the node that comes before node containing 
// data. If node with data doesn't exist or node
// containing data is HEAD node, it returns NULL.
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

static llnode_t *
llist_extract_llnode(llist_t * list, int data)
// Extracts node containing data from linked list
// and modifies previous node to point to extracted
// node's next node.
{
  llnode_t * prev_node = list->head;

  if (list->head == NULL)
    return NULL;

  /* Handles case where node to extract is HEAD */
  if (list->head->data == data)
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

  /* Handles case where node to extract is TAIL */
  if (extracted_node == list->tail)
    list->tail = prev_node;

  return extracted_node;
}

static llnode_t *
llist_get_llnode_at(llist_t * list, size_t idx)
// Returns node at position idx if it exists or
// returns NULL otherwise.
{
  size_t i = 0;
  llnode_t * cur = list->head;
  while (cur && i++ < idx)
    cur = cur->next;
  return cur;
}
