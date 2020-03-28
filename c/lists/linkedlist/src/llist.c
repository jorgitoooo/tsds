#include <stdio.h>
#include <pthread.h>
#include <sys/types.h>

#include "../headers/llist.h"
#include "../headers/utils.h"

#define DEBUG 0

/* Helper functions */
static void _llist_init(llist_t *);
static void _llist_init_with_llorder(llist_t *, llorder_type_t);
static void _llist_insert_asc(llist_t *, llnode_t *);
static void _llist_insert_desc(llist_t *, llnode_t *);
static void _llist_insert_unordered(llist_t *, llnode_t *);
static void _llist_acquire_writers_lock(void);
static void _llist_release_writers_lock(void);
static void _llist_reverse(llist_t *);
static void _llist_reorder_llnodes_in_llist(llist_t *, llnode_t **);
static void _llist_sort(llist_t *, llorder_type_t);
static int _llist_asc_comparitor(void const *, void const *);
static int _llist_desc_comparitor(void const *, void const *);
static llnode_t * _llist_get_prev_llnode(llist_t *, int);
static llnode_t * _llist_extract_llnode(llist_t *, int);
static llnode_t * _llist_get_llnode_at(llist_t *, size_t);
static llnode_t ** _llist_make_llnode_array(llist_t *);

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
/* Creates a new llnode.
**/
{
  llnode_t * llnode = (llnode_t *) malloc(sizeof(llnode_t));
  if (llnode)
  {
    llnode->data = data;
    llnode->next = NULL;
  }
  return llnode;
}

void
llnode_free(llnode_t * llnode)
/* Frees linked list node if it exists.
**/
{
  if (llnode == NULL)
    return;

  llnode->data = 0;
  llnode->next = NULL;
  free(llnode);
  llnode = NULL;
}

llist_t *
llist_create(void)
/* Creates a new linked list with a default llorder_type_t
** of unordered.
**/
{
  llist_t * llist = (llist_t *) malloc(sizeof(llist_t));
  if (llist)
    _llist_init(llist);
  return llist;
}

llist_t *
llist_create_with_llorder(llorder_type_t order)
/* Creates a new linked list with the specified llorder_type_t.
**/
{
  llist_t * llist = (llist_t *) malloc(sizeof(llist_t));
  if (llist)
    _llist_init_with_llorder(llist, order);
  return llist;
}

void
llist_free(llist_t * llist)
/* Frees memory allocated for linked list.
**/
{
  /* Avoids unnecessary locking/unlocking 
     of mutexes */
  if (llist == NULL)
    return;

  pthread_mutex_lock(&mtx);
  pthread_mutex_lock(&w_mtx);
  if (llist)
  {
    llnode_t * cur = llist->head;
    while (cur)
    {
      llnode_t * llnode_to_free = cur;
      cur = cur->next;
      llnode_free(llnode_to_free);
    }
    llist->sz = 0;
    llist->head = NULL;
    llist->tail = NULL;
    free(llist);
    llist = NULL;
  }
  pthread_mutex_unlock(&w_mtx);
  pthread_mutex_unlock(&mtx);
}

void
llist_insert(llist_t * llist, llnode_t * llnode)
/* Inserts the llnode into the linked list in accordance
** to to the list's order type and increments the size
** of the linked list by one.
**/
{
  /* Avoids unnecessary locking/unlocking 
     of mutexes */
  if (llist == NULL)
    return;

  pthread_mutex_lock(&mtx);
  pthread_mutex_lock(&w_mtx);
  if (llist && llnode)
  {
    if (llist->order == ASC)
      _llist_insert_asc(llist, llnode);

    else if (llist->order == DESC)
      _llist_insert_desc(llist, llnode);

    else
      _llist_insert_unordered(llist, llnode);

    llist->sz++;
  }
  pthread_mutex_unlock(&w_mtx);
  pthread_mutex_unlock(&mtx);
}

void
llist_delete(llist_t * llist, int data)
/* Deletes first llnode in the linked list that contains 
** data and decrements linked list size by one.
**/
{
  /* Avoids unnecessary locking/unlocking 
     of mutexes */
  if (llist == NULL)
    return;

  pthread_mutex_lock(&mtx);
  pthread_mutex_lock(&w_mtx);
  if (llist)
  {
    llnode_t * extracted_llnode = _llist_extract_llnode(llist, data);

    if (extracted_llnode)
    {
      if (extracted_llnode->data != data) /* Internal error */
      {
        pthread_mutex_unlock(&mtx);
        pthread_mutex_unlock(&w_mtx);
        return;
      }

      llnode_free(extracted_llnode);
      llist->sz--;
    }
  }
  pthread_mutex_unlock(&mtx);
  pthread_mutex_unlock(&w_mtx);
}

void
llist_sort(llist_t * llist, llorder_type_t order)
/* Sorts llist in the order specified by order. Does
** not modify the order of the llist itself.
**/
{
  /* Avoids unnecessary locking/unlocking 
     of mutexes */
  if (llist == NULL 
      || order == NONE)
    return;

  pthread_mutex_lock(&mtx);
  pthread_mutex_lock(&w_mtx);

  if (llist)
    _llist_sort(llist, order);

  pthread_mutex_unlock(&w_mtx);
  pthread_mutex_unlock(&mtx);
}

void
llist_change_llorder(llist_t * llist, llorder_type_t order)
/* Modifies the order of llist and reorders elements
** accordingly.
**/
{
  /* Avoids unnecessary locking/unlocking 
     of mutexes */
  if (llist == NULL)
    return;

  pthread_mutex_lock(&mtx);
  pthread_mutex_lock(&w_mtx);

  // DEBUG
  if (DEBUG)
    puts("llist_change_llorder(llist_t * llist, llorder_type_t order)");

  if (llist && llist->order != order)
  {
    /* Implies that order goes from
       NONE -> ASC | DESC */
    if (llist->order == NONE)
      _llist_sort(llist, order);

    /* Implies that order goes from
       ASC  -> DESC
            or
       DESC -> ASC */
    else if (order != NONE)
      _llist_reverse(llist);

    llist->order = order;
  }
  pthread_mutex_unlock(&w_mtx);
  pthread_mutex_unlock(&mtx);
}

llnode_t * 
llist_at(llist_t * llist, size_t idx)
/* Returns the llnode at position idx if idx is within
** bounds and llist is not NULL. Else, returns NULL.
**/
{
  /* Avoids unnecessary locking/unlocking 
     of mutexes */
  if (llist == NULL)
    return NULL;

  llnode_t * llnode = NULL;
  _llist_acquire_writers_lock();
  if (llist
      && idx >= 0
      && idx < llist->sz)
  {
    llnode = _llist_get_llnode_at(llist, idx);
  }
  _llist_release_writers_lock();

  return llnode;
}

llnode_t *
llist_get(llist_t * llist, int data)
/* Returns first llnode containing data. Else returns
** NULL.
**/
{
  /* Avoids unnecessary locking/unlocking 
     of mutexes */
  if (llist == NULL)
    return NULL;

  _llist_acquire_writers_lock();
  llnode_t * llnode = NULL;
  if (llist)
  {
    llnode = llist->head;
    while (llnode && llnode->data != data)
      llnode = llnode->next;
  }
  _llist_release_writers_lock();
  return llnode;
}

/*-----------------------------------*/
/* Helper Functions                  */ 
/*-----------------------------------*/

static void 
_llist_init(llist_t * llist)
/* Initializes linked list and sets llorder_type_t to
** unordered. llist should always be a valid pointer.
**/
{
  llist->head = NULL;
  llist->tail = NULL;
  llist->order = NONE;
  llist->sz = 0;
}

static void 
_llist_init_with_llorder(llist_t * llist, llorder_type_t order)
/* Initializes linked list and sets llorder_type_t to
** specified order. llist should always be a valid
** pointer.
**/
{
  _llist_init(llist);
  llist->order = order;
}

static void
_llist_insert_asc(llist_t * llist, llnode_t * llnode)
/* Inserts llnode into linked list in ascending order.
** This function should not be called if llist or llnode
** are NULL.
**/
{
  int data = llnode->data;
  llnode_t * prev_llnode = _llist_get_prev_llnode(llist, data);

  /* Node to insert will be new HEAD */
  if (prev_llnode == NULL) 
  {
    llnode->next = llist->head;
    llist->head = llnode;

    /* Handles case where llist is empty */
    if (llist->sz == 0) 
      llist->tail = llnode;
  }
  else
  {
    /* Node to insert will be new TAIL */
    if (prev_llnode->next == NULL) 
    {
      prev_llnode->next = llnode;
      llist->tail = llnode;
    }
    else
    {
      llnode->next = prev_llnode->next;
      prev_llnode->next = llnode;
    }
  }
}

static void
_llist_insert_desc(llist_t * llist, llnode_t * llnode)
/* Inserts llnode into linked list in descending order
** This function should not be called if llist or llnode
** are NULL.
**/
{
  int data = llnode->data;
  llnode_t * prev_llnode = _llist_get_prev_llnode(llist, data);

  /* Node to insert will be new HEAD */
  if (prev_llnode == NULL) 
  {
    llnode->next = llist->head;
    llist->head = llnode;

    /* Handles case where llist is empty */
    if (llist->sz == 0) 
      llist->tail = llnode;
  }
  else
  {
    /* Node to insert will be new TAIL */
    if (prev_llnode->next == NULL) 
    {
      prev_llnode->next = llnode;
      llist->tail = llnode;
    }
    else
    {
      llnode->next = prev_llnode->next;
      prev_llnode->next = llnode;
    }
  }
}

static void
_llist_insert_unordered(llist_t * llist, llnode_t * llnode)
/* Appends llnode to end of linked list. This function
** should not be called if llist or llnode are NULL.
**/
{
  if (llist->sz == 0)
    llist->head = llist->tail = llnode;
  else 
  {
    llist->tail->next = llnode;
    llist->tail = llnode;
  }
}

static void
_llist_acquire_writers_lock(void)
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
_llist_release_writers_lock(void)
/* The second to last reader signals the first reader,
** which acquired lock on w_mtx, to release lock on
** w_mtx.
**/
{
  // DEBUG
  if (DEBUG)
    puts("_llist_release_writers_lock(void)");

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
_llist_reverse(llist_t * llist)
{
  llnode_t * new_prev;
  llnode_t * new_next;
  llnode_t * cur;

  cur = llist->head;
  llist->head = llist->tail;
  llist->tail = cur;

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
_llist_reorder_llnodes_in_llist(llist_t * llist, llnode_t * llnodes[])
/* Size of llnodes should always be equal to llist size.
**/
{
  size_t sz = llist->sz;
  llist->head = llnodes[0];
  llnode_t * cur = llist->head;
  size_t i;
  for (i = 1; i < sz && cur; i++)
  {
    cur->next = llnodes[i];
    cur = llnodes[i];
  }
  llist->tail = cur;
  llist->tail->next = NULL;
}

static void
_llist_sort(llist_t * llist, llorder_type_t order)
/* Sorts llist in the order specified by order. Uses
** qsort and ascending/descending comparitors to sort.
** Does not modify the order of the llist itself.
**/
{
  // DEBUG
  if (DEBUG)
    puts("_llist_sort(llist_t * llist, llorder_type_t order)");

  if (llist->sz == 0)
    return;

  llnode_t ** llnodes = _llist_make_llnode_array(llist);

  if (order == ASC)
    qsort(llnodes, llist->sz, sizeof(llnode_t *), _llist_asc_comparitor);
  else if (order == DESC)
    qsort(llnodes, llist->sz, sizeof(llnode_t *), _llist_desc_comparitor);

  _llist_reorder_llnodes_in_llist(llist, llnodes);
  free(llnodes);
}

static int
_llist_asc_comparitor(void const * lhs, void const * rhs)
{
  llnode_t * l_lhs = *((llnode_t **)lhs);
  llnode_t * l_rhs = *((llnode_t **)rhs);
  return l_lhs->data - l_rhs->data;
}

static int
_llist_desc_comparitor(void const * lhs, void const * rhs)
{
  llnode_t * l_lhs = *((llnode_t **)lhs);
  llnode_t * l_rhs = *((llnode_t **)rhs);
  return l_rhs->data - l_lhs->data;
}

static llnode_t *
_llist_get_prev_llnode(llist_t * llist, int data)
/* Returns the llnode that comes before llnode containing 
** data. If llnode with data doesn't exist or llnode
** containing data is HEAD llnode, it returns NULL.
**/
{
  llnode_t * res = NULL;
  llnode_t * head = llist->head;
  if (head)
  {
    if (llist->order == ASC &&
        head->data <= data)
    {
      res = head;
      while (res->next && res->next->data <= data)
        res = res->next;
    }
    else if (llist->order == DESC &&
             head->data >= data)
    {
      res = head;
      while (res->next && res->next->data >= data)
        res = res->next;
    }
    else if (llist->order == NONE &&
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
_llist_extract_llnode(llist_t * llist, int data)
/* Extracts llnode containing data from linked list
** and modifies previous llnode to point to extracted
** llnode's next llnode.
**/
{
  llnode_t * prev_llnode = llist->head;

  if (llist->head == NULL)
    return NULL;

  /* Handles case where llnode to extract is HEAD */
  if (llist->head->data == data)
  {
    if (llist->sz == 1)
      llist->head = llist->tail = NULL;

    else
      llist->head = llist->head->next;

    return prev_llnode;
  }

  while (prev_llnode->next && prev_llnode->next->data != data)
    prev_llnode = prev_llnode->next;

  llnode_t * extracted_llnode = prev_llnode->next;
  if (prev_llnode->next && prev_llnode->next->data == data)
    prev_llnode->next = prev_llnode->next->next;

  /* Handles case where llnode to extract is TAIL */
  if (extracted_llnode == llist->tail)
    llist->tail = prev_llnode;

  return extracted_llnode;
}

static llnode_t *
_llist_get_llnode_at(llist_t * llist, size_t idx)
/* Returns node at position idx if it exists or
** returns NULL otherwise.
**/
{
  size_t i = 0;
  llnode_t * cur = llist->head;
  while (cur && i++ < idx)
    cur = cur->next;
  return cur;
}

static llnode_t **
_llist_make_llnode_array(llist_t * llist)
{
  // DEBUG
  if (DEBUG)
    puts("_llist_make_llnode_array(llist_t * llist)");

  llnode_t ** llnodes = (llnode_t **) malloc(sizeof(llnode_t *) * llist->sz);
  llnode_t * cur = llist->head;
  int i;
  for (i = 0; i < llist->sz && cur; i++)
  {
    llnodes[i] = cur;
    cur = cur->next;
  }
  return llnodes;
}
