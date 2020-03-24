#include <stdio.h>

#include "../headers/llist.h"
#include "../headers/utils.h"

/* Helper functions */
static void init_llist(llist_t *);
static void init_llist_with_order(llist_t *, order_type);
static void free_llnode(llnode_t *);
static void insert_llnode_asc(llist_t *, llnode_t *);
static void insert_llnode_desc(llist_t *, llnode_t *);
static void insert_llnode_unordered(llist_t *, llnode_t *);
static llnode_t * get_prev_llnode(llist_t *, int);
static llnode_t * extract_llnode(llist_t *, int);


/*-----------------------------------*/
/* Function Definitions              */
/*-----------------------------------*/
llist_t *
create_llist(void)
{
  llist_t * list = (llist_t *) malloc(sizeof(llist_t));
  if (list)
    init_llist(list);
  return list;
}

llist_t *
create_llist_with_order(order_type order)
{
  llist_t * list = (llist_t *) malloc(sizeof(llist_t));
  if (list)
    init_llist_with_order(list, order);
  return list;
}

llnode_t *
create_llnode(int data)
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
free_llist(llist_t * list)
{
  if (list == NULL)
    return;

  llnode_t * cur = list->head;
  while (cur)
  {
    llnode_t * node_to_free = cur;
    cur = cur->next;
    free_llnode(node_to_free);
  }
  list->sz = 0;
  free(list);
}

void
insert_llnode(llist_t * list, llnode_t * node)
{
  if (list && node)
  {
    if (list->order == ASC)
      insert_llnode_asc(list, node);

    else if (list->order == DESC)
      insert_llnode_desc(list, node);

    else
      insert_llnode_unordered(list, node);

    list->sz++;
  }
}

void
delete_llnode(llist_t * list, int data)
// Deletes first node containing data
{
  if (list)
  {
    llnode_t * extracted_node = extract_llnode(list, data);

    if (extracted_node)
    {
      if (extracted_node->data != data) /* Internal error */
        return;

      //printf("Deleting node with data %d\n", extracted_node->data);
      free_llnode(extracted_node);
      list->sz--;
    }
  }
}

/*-----------------------------------*/
/* Helper Functions                  */ 
/*-----------------------------------*/
static void 
init_llist(llist_t * list)
{
  list->head = NULL;
  list->tail = NULL;
  list->order = NONE;
  list->sz = 0;
}

static void 
init_llist_with_order(llist_t * list, order_type order)
{
  init_llist(list);
  list->order = order;
}

static void
free_llnode(llnode_t * node)
{
  if (node == NULL)
    return;

  node->data = 0;
  node->next = NULL;
  free(node);
}


static void
insert_llnode_unordered(llist_t * list, llnode_t * node)
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

static void
insert_llnode_asc(llist_t * list, llnode_t * node)
// Inserts node into linked list in ascending order.
// This function should not be called if list or node
// are NULL.
{
  int data = node->data;
  llnode_t * prev_node = get_prev_llnode(list, data);

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

static void
insert_llnode_desc(llist_t * list, llnode_t * node)
// Inserts node into linked list in descending order
// This function should not be called if list or node
// are NULL.
{
  int data = node->data;
  llnode_t * prev_node = get_prev_llnode(list, data);

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

static llnode_t *
get_prev_llnode(llist_t * list, int data)
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
extract_llnode(llist_t * list, int data)
{
  llnode_t * prev_node = list->head;

  if (list->head == NULL)
    return NULL;

  if (list->head->data == data) /* Handles case where node to extract is HEAD */
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

  if (extracted_node == list->tail) /* Handles case where node to extract is TAIL */
    list->tail = prev_node;

  return extracted_node;
}

