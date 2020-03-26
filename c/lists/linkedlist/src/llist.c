#include <stdio.h>
#include <pthread.h>
#include <sys/types.h>

#include "../headers/llist.h"
#include "../headers/utils.h"

/* Helper functions */
static void llist_init(llist_t *);
static void llist_init_with_order(llist_t *, order_type_t);
static void llist_insert_asc(llist_t *, llnode_t *);
static void llist_insert_desc(llist_t *, llnode_t *);
static void llist_insert_unordered(llist_t *, llnode_t *);
static void llist_acquire_writers_lock(void);
static void llist_release_writers_lock(void);
static void llist_reverse(llist_t * list);
static void llist_reorder_nodes_in_llist(llist_t * list, llnode_t * nodes[]);
static int llist_asc_comparitor(void const * lhs, void const * rhs);
static int llist_desc_comparitor(void const * lhs, void const * rhs);
static llnode_t * llist_get_prev_llnode(llist_t *, int);
static llnode_t * llist_extract_llnode(llist_t *, int);
static llnode_t * llist_get_llnode_at(llist_t * list, size_t idx);
static llnode_t ** llist_make_node_array(llist_t * list);

/* Mutexes */
static pthread_mutex_t mtx   = PTHREAD_MUTEX_INITIALIZER;  /* General mutex        */
static pthread_mutex_t w_mtx = PTHREAD_MUTEX_INITIALIZER;  /* Write specific mutex */

/* Reader singleton */
static struct {
  pthread_mutex_t mtx;
  pthread_cond_t cv; 
  pthread_t first_reader;
  uint64_t cnt;
  uint64_t entered;
} reader = {
  PTHREAD_MUTEX_INITIALIZER,
  PTHREAD_COND_INITIALIZER,
  ((pthread_t)-1),
  0,
  0
};

/*-----------------------------------*/
/* Function Definitions              */
/*-----------------------------------*/

llnode_t *
llnode_create(int data)
/* Creates a new linked list node.
**/
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
llnode_free(llnode_t * node)
/* Frees linked list node if it exists.
**/
{
  if (node == NULL)
    return;

  node->data = 0;
  node->next = NULL;
  free(node);
}

llist_t *
llist_create(void)
/* Creates a new linked list with a default order_type_t
** of unordered.
**/
{
  llist_t * list = (llist_t *) malloc(sizeof(llist_t));
  if (list)
    llist_init(list);
  return list;
}

llist_t *
llist_create_with_order(order_type_t order)
/* Creates a new linked list with the specified order_type_t.
**/
{
  llist_t * list = (llist_t *) malloc(sizeof(llist_t));
  if (list)
    llist_init_with_order(list, order);
  return list;
}

void
llist_free(llist_t * list)
/* Frees memory allocated for linked list.
**/
{
  pthread_mutex_lock(&mtx);
  pthread_mutex_lock(&w_mtx);
  if (list)
  {
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
  pthread_mutex_unlock(&w_mtx);
  pthread_mutex_unlock(&mtx);
}

void
llist_insert(llist_t * list, llnode_t * node)
/* Inserts the linked list node referenced by node into 
** the linked list in accordance to to the list's order
** type and increments the size of the linked list by
** one.
**/
{
  pthread_mutex_lock(&mtx);
  pthread_mutex_lock(&w_mtx);
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
  pthread_mutex_unlock(&w_mtx);
  pthread_mutex_unlock(&mtx);
}

void
llist_delete(llist_t * list, int data)
/* Deletes first node in the linked list that contains 
** data and decrements linked list size by one.
**/
{
  pthread_mutex_lock(&mtx);
  pthread_mutex_lock(&w_mtx);
  if (list)
  {
    llnode_t * extracted_node = llist_extract_llnode(list, data);

    if (extracted_node)
    {
      if (extracted_node->data != data) /* Internal error */
      {
        pthread_mutex_unlock(&mtx);
        pthread_mutex_unlock(&w_mtx);
        return;
      }

      llnode_free(extracted_node);
      list->sz--;
    }
  }
  pthread_mutex_unlock(&mtx);
  pthread_mutex_unlock(&w_mtx);
}

void
llist_sort(llist_t * list, order_type_t order)
/* Sorts list in the order specified by order. Uses
** qsort and ascending/descending comparitors to sort.
**/
{
  if (list == NULL)
    return;
  llnode_t ** nodes = llist_make_node_array(list);

  if (order == ASC)
    qsort(nodes, list->sz, sizeof(llnode_t *), llist_asc_comparitor);
  else if (order == DESC)
    qsort(nodes, list->sz, sizeof(llnode_t *), llist_desc_comparitor);

  llist_reorder_nodes_in_llist(list, nodes);
  free(nodes);
}

llnode_t * 
llist_at(llist_t * list, size_t idx)
/* Returns the linked list node at position idx if idx 
** is within bounds and list is not NULL. Else, returns
** NULL.
**/
{
  llist_acquire_writers_lock();
  llnode_t * node = NULL;
  if (list
      && idx >= 0
      && idx < list->sz)
  {
    node = llist_get_llnode_at(list, idx);
  }
  llist_release_writers_lock();

  return node;
}

llnode_t *
llist_get(llist_t * list, int data)
/* Returns first linked list node containing data.
** Else returns NULL.
**/
{
  llist_acquire_writers_lock();
  llnode_t * node = NULL;
  if (list)
  {
    node = list->head;
    while (node && node->data != data)
      node = node->next;
  }
  llist_release_writers_lock();
  return node;
}

void
llist_change_order(llist_t * list, order_type_t order)
{
  pthread_mutex_lock(&mtx);
  pthread_mutex_lock(&w_mtx);
  if (list && list->order != order)
  {
    if (list->order == NONE)
      llist_sort(list, order);
    else
      llist_reverse(list);
    list->order = order;
  }
  pthread_mutex_unlock(&w_mtx);
  pthread_mutex_unlock(&mtx);
}
/*-----------------------------------*/
/* Helper Functions                  */ 
/*-----------------------------------*/

static void 
llist_init(llist_t * list)
/* Initializes linked list and sets order_type_t to
** unordered. list should always be a valid pointer.
**/
{
  list->head = NULL;
  list->tail = NULL;
  list->order = NONE;
  list->sz = 0;
}

static void 
llist_init_with_order(llist_t * list, order_type_t order)
/* Initializes linked list and sets order_type_t to
** specified order. list should always be a valid
** pointer.
**/
{
  llist_init(list);
  list->order = order;
}

static void
llist_insert_asc(llist_t * list, llnode_t * node)
/* Inserts node into linked list in ascending order.
** This function should not be called if list or node
** are NULL.
**/
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
/* Inserts node into linked list in descending order
** This function should not be called if list or node
** are NULL.
**/
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
llist_insert_unordered(llist_t * list, llnode_t * node)
/* Appends linked list node to end of linked list.
** This function should not be called if list or
** node are NULL.
**/
{
  if (list->sz == 0)
    list->head = list->tail = node;

  else /* make tail = node */
  {
    list->tail->next = node;
    list->tail = node;
  }
}

static void
llist_acquire_writers_lock(void)
/* The first thread to call this function acquires
** a lock on writers' mutex w_mtx and sets itself
** as the first reader in the reader structure.
**/
{
  pthread_mutex_lock(&mtx);
  pthread_mutex_lock(&reader.mtx);
  /* First reader locks writer mutex */
  if (!reader.entered)
  {
    reader.entered = 1;
    reader.first_reader = pthread_self();
    pthread_mutex_lock(&w_mtx);
  }
  reader.cnt++;
  pthread_mutex_unlock(&reader.mtx);
  pthread_mutex_unlock(&mtx);
}

static void
llist_release_writers_lock(void)
/* The second to last reader signals the first reader,
** which acquired lock on w_mtx, to release lock on
** w_mtx.
**/
{
  /* First reader releases writer mutex */
  pthread_mutex_lock(&reader.mtx);
  if (reader.first_reader == pthread_self())
  {
    while (reader.cnt != 1)
      pthread_cond_wait(&reader.cv, &reader.mtx);

    /* Only thread that locked w_mtx can unlock it */
    pthread_mutex_unlock(&w_mtx); 
    reader.cnt--;
    reader.entered = 0; /* No readers reading */
  }
  else
  {
    if (--reader.cnt == 1)
      pthread_cond_signal(&reader.cv);
  }
  pthread_mutex_unlock(&reader.mtx);
}

static void
llist_reverse(llist_t * list)
{
  llnode_t * new_prev;
  llnode_t * new_next;
  llnode_t * cur;

  cur = list->head;
  list->head = list->tail;
  list->tail = cur;

  new_next = NULL;
  new_prev = cur;
  while (cur)
  {
    new_prev = cur->next;
    cur->next = new_next;
    new_next = cur;
    cur = new_prev;
  }
}

static void
llist_reorder_nodes_in_llist(llist_t * list, llnode_t * nodes[])
/* Size of nodes should always be equal to list size.
**/
{
  size_t sz = list->sz;
  list->head = nodes[0];
  llnode_t * cur = list->head;
  size_t i;
  for (i = 1; i < sz && cur; i++)
  {
    cur->next = nodes[i];
    cur = nodes[i];
  }
  list->tail = cur;
  list->tail->next = NULL;
}

static int
llist_asc_comparitor(void const * lhs, void const * rhs)
{
  llnode_t * l_lhs = *((llnode_t **)lhs);
  llnode_t * l_rhs = *((llnode_t **)rhs);
  return l_lhs->data - l_rhs->data;
}

static int
llist_desc_comparitor(void const * lhs, void const * rhs)
{
  llnode_t * l_lhs = *((llnode_t **)lhs);
  llnode_t * l_rhs = *((llnode_t **)rhs);
  return l_rhs->data - l_lhs->data;
}

static llnode_t *
llist_get_prev_llnode(llist_t * list, int data)
/* Returns the node that comes before node containing 
** data. If node with data doesn't exist or node
** containing data is HEAD node, it returns NULL.
**/
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
/* Extracts node containing data from linked list
** and modifies previous node to point to extracted
** node's next node.
**/
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
/* Returns node at position idx if it exists or
** returns NULL otherwise.
**/
{
  size_t i = 0;
  llnode_t * cur = list->head;
  while (cur && i++ < idx)
    cur = cur->next;
  return cur;
}

static llnode_t **
llist_make_node_array(llist_t * list)
{
  llnode_t ** nodes = (llnode_t **) malloc(sizeof(llnode_t *) * list->sz);
  llnode_t * cur = list->head;
  int i;
  for (i = 0; i < list->sz && cur; i++)
  {
    nodes[i] = cur;
    cur = cur->next;
  }
  return nodes;
}
