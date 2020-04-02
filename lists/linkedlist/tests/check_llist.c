#include <time.h>
#include <stdio.h>
#include <check.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/types.h>

#include "../headers/llist.h"
#include "../headers/utils.h"

#define handle_error(err, msg)             \
  do {                                     \
    if (err != 0)                          \
      fprintf(stderr, "Error: %s\n", msg); \
  } while(0)

/*---------------------------------------*/
/* Typedefs                              */
/*---------------------------------------*/
typedef void * (*tsds_func_t)(void *);
typedef struct tsds_llist_arg tsds_llarg_t;

/*---------------------------------------*/
/* Globals                               */
/*---------------------------------------*/
llist_t * llist;

/*---------------------------------------*/
/* Helper function declarations          */
/*---------------------------------------*/
int tsds_llist_has_llorder(llorder_type_t order);
void tsds_ck_assert_llist_array_eq(llnode_t * llnode,
                                   int const * const data,
                                   int const sz);
void tsds_create_nthreads(pthread_t * threads, 
                          tsds_func_t func, 
                          tsds_llarg_t * llargs, 
                          int const num_threads);
void tsds_join_nthreads(pthread_t * threads,
                        int const num_threads);
void tsds_spin(unsigned int seed);
void * tsds_llist_insert(void * arg);
void * tsds_llist_at(void * arg);

/*---------------------------------------*/
/* Test fixtures                         */
/*---------------------------------------*/
void
setup(void)
{
  llist = llist_create();
}

void
teardown(void)
{
  llist_free(llist);
}

/*---------------------------------------*/
/* Helper function definitions           */
/*---------------------------------------*/
struct tsds_llist_arg
{
  llist_t * llist;
  llnode_t * llnode;
  int data;
  int idx;
};

void *
tsds_llist_insert(void * arg)
{
  tsds_llarg_t * llarg = (tsds_llarg_t *)arg;
  tsds_spin(llarg->data);
  llist_insert(llarg->llist,
               llnode_create(llarg->data));
  return 0;
}

void *
tsds_llist_at(void * arg)
{
  tsds_llarg_t * llarg = (tsds_llarg_t *)arg;
  tsds_spin(llarg->data);
  return (void *)llist_at(llarg->llist, llarg->idx);
}

void *
tsds_llist_get(void * arg)
{
  tsds_llarg_t * llarg = (tsds_llarg_t *)arg;
  tsds_spin(llarg->data);
  return (void *)llist_get(llarg->llist, llarg->data);
}

void
tsds_spin(unsigned int seed)
{
  unsigned int const ITERS = 1000000;
  seed = (unsigned int)(time(NULL)/(seed*ITERS + 1));
  int start = rand_r(&seed);

  int i;
  for (i = start; i < ITERS; i++);
}

void
tsds_join_nthreads(pthread_t * threads,
                   int const num_threads)
{
  int i, r;
  for (i = 0; i < num_threads; i++)
  {
    r = pthread_join(threads[i], NULL);
    handle_error(r, "pthread_join");
  }
}

void
tsds_create_nthreads(pthread_t * threads, 
                     tsds_func_t func, 
                     tsds_llarg_t * llargs, 
                     int const num_threads)
{
  int i, r;
  for (i = 0; i < num_threads; i++)
  {
    r = pthread_create(&threads[i], NULL,
                       &(*func),
                       (void *)&llargs[i]);
    handle_error(r, "pthread_create");
  }

}

void
tsds_ck_assert_llist_array_eq(llnode_t * llnode,
                              int const * const data,
                              int const sz)
{
  int i;
  for (i = 0;
       i < sz && llnode;
       i++, llnode = llnode->next)
  {
    ck_assert_int_eq(llnode->data, data[i]);
  }
}

int 
tsds_llist_has_llorder(llorder_type_t order)
{
  int has_order = 1;
  if (llist == NULL)
    return has_order;
  if (llist->order != order)
    return !has_order;

  llnode_t * cur = llist->head;
  switch (order)
  {
    case ASC:
      while (cur && cur->next)
      {
        if (cur->data > cur->next->data)
        {
          has_order = 0;
          break;
        }
        cur = cur->next;
      }
      break;
    case DESC:
      while (cur && cur->next)
      {
        if (cur->data < cur->next->data)
        {
          has_order = 0;
          break;
        }
        cur = cur->next;
      }
      break;
    case NONE:
    default:
      break;
  }
  return has_order;
}

/*---------------------------------------*/
/* Unit Tests                            */
/*---------------------------------------*/
START_TEST(test_llist_size)
/* Tests for correct incrementing 
** and decrementing of linked list
** size after insertion and deletion
** operations.
**/
{ 
  llnode_t * n0 = llnode_create(0);
  llnode_t * n1 = llnode_create(1);
  llnode_t * n2 = llnode_create(4);
  llnode_t * n3 = llnode_create(-8);

  ck_assert_uint_eq(llist->sz, 0);

  llist_insert(llist, n0);
  llist_insert(llist, n1);
  ck_assert_uint_eq(llist->sz, 2);
  ck_assert_ptr_eq(
      llist_get(llist, n0->data), 
      n0);
  ck_assert_ptr_eq(
      llist_get(llist, n1->data), 
      n1);

  llist_insert(llist, n2);
  llist_insert(llist, n3);
  ck_assert_uint_eq(llist->sz, 4);
  ck_assert_ptr_eq(
      llist_get(llist, n2->data), 
      n2);
  ck_assert_ptr_eq(
      llist_get(llist, n3->data), 
      n3);

  llist_delete(llist, 1);
  ck_assert_uint_eq(llist->sz, 3);
  ck_assert_ptr_eq(
      llist_get(llist, n0->data), 
      n0);
  ck_assert_ptr_eq(
      llist_get(llist, n2->data), 
      n2);
  ck_assert_ptr_eq(
      llist_get(llist, n3->data), 
      n3);

  llist_delete(llist, 1);
  ck_assert_uint_eq(llist->sz, 3);
  ck_assert_ptr_eq(
      llist_get(llist, n0->data), 
      n0);
  ck_assert_ptr_eq(
      llist_get(llist, n2->data), 
      n2);
  ck_assert_ptr_eq(
      llist_get(llist, n3->data), 
      n3);

  llist_delete(llist, 4);
  ck_assert_uint_eq(llist->sz, 2);
  ck_assert_ptr_eq(
      llist_get(llist, n0->data), 
      n0);
  ck_assert_ptr_eq(
      llist_get(llist, n3->data), 
      n3);

  llist_delete(llist, 2);
  ck_assert_uint_eq(llist->sz, 2);
  ck_assert_ptr_eq(
      llist_get(llist, n0->data), 
      n0);
  ck_assert_ptr_eq(
      llist_get(llist, n3->data), 
      n3);

  llist_delete(llist, -8);
  ck_assert_uint_eq(llist->sz, 1);
  ck_assert_ptr_eq(
      llist_get(llist, n0->data), 
      n0);

  llist_delete(llist, 0);
  ck_assert_uint_eq(llist->sz, 0);

  llist_delete(llist, 1);
  ck_assert_uint_eq(llist->sz, 0);

  llist_delete(llist, 4);
  ck_assert_uint_eq(llist->sz, 0);
}
END_TEST

START_TEST(test_llist_create_with_llorder)
/* Tests that llist_create_with_llorder() creates
** a new llist with the specified order.
**/
{
  int const NUM_ORDERS = 3; /* [ASC, DESC, NONE */

  llist_free(llist);

  int order, data;
  for (order = data = 0; 
       order < NUM_ORDERS;
       order++, data = 0)
  {
    llist = llist_create_with_llorder(order);
    llist_insert(llist, llnode_create(data++));
    llist_insert(llist, llnode_create(data++));
    llist_insert(llist, llnode_create(data++));

    ck_assert_int_eq(tsds_llist_has_llorder(order), 1);

    if (order < NUM_ORDERS - 1)
      llist_free(llist);
  }
}

START_TEST(test_llist_head_tail)
/* Tests that head and tail pointers
** are correctly assigned and maintained
** throughout the life of the llist_t
** structure.
**/
{
  llnode_t * n_0   = llnode_create(0);
  llnode_t * n_3   = llnode_create(3);
  llnode_t * n_295 = llnode_create(295);
  llnode_t * n_38  = llnode_create(38);

  llist_insert(llist, n_0);
  llist_insert(llist, n_3);
  llist_insert(llist, n_295);
  llist_insert(llist, n_38);
  /* [0, 3, 295, 38]  */

  ck_assert_ptr_eq(llist->head, n_0);
  ck_assert_ptr_eq(llist->tail, n_38);
  /* [0, 3, 295, 38]  */
  /*  |          |    */
  /*  head       tail */

  llist_delete(llist, n_0->data);
  ck_assert_ptr_eq(llist->head, n_3);
  ck_assert_ptr_eq(llist->tail, n_38);
  /* [3, 295, 38]  */
  /*  |       |    */
  /*  head    tail */

  llist_delete(llist, n_295->data);
  ck_assert_ptr_eq(llist->head, n_3);
  ck_assert_ptr_eq(llist->tail, n_38);
  /* [3,  38]   */
  /*  |    |    */
  /*  head tail */

  llist_delete(llist, n_38->data);
  ck_assert_ptr_eq(llist->head, n_3);
  ck_assert_ptr_eq(llist->tail, n_3);
  /*     [3]     */
  /*      |      */
  /* head - tail */

  llist_delete(llist, n_3->data);
  ck_assert_ptr_null(llist->head);
  ck_assert_ptr_null(llist->tail);
  /*     [ ]     */
  /*      |      */
  /* head - tail */
}
END_TEST 

START_TEST(test_llist_at)
/* Tests that the llist_at(...) function
** returns either the correct llnode residing
** at the specified index or NULL if index
** is out of bounds.
**/
{
  int const NUM_LLNODES = 30;
  llnode_t * llnodes[NUM_LLNODES];

  /* Changed order to ascending */
  llist->order = ASC;

  int i;
  for (i = 0; i < NUM_LLNODES; i++)
  {
    llnodes[i] = llnode_create(i);
    llist_insert(llist, llnodes[i]);
  }

  for (i = 0; i < NUM_LLNODES; i++)
    ck_assert_ptr_eq(llist_at(llist, i), llnodes[i]);

  /* Edge cases:
  **  1. llist->sz = 1
  **  2. llist->sz = 0
  **  3. llist = NULL
  ***/
  llist_free(llist);
  llist = llist_create();
  llist_insert(llist, llnode_create(2)); /* [2] */

  /* Edge case 1 */
  ck_assert_int_eq(llist->sz, 1);
  ck_assert_ptr_nonnull(llist_at(llist,0));
  ck_assert_int_eq(llist_at(llist,0)->data, 2);
  ck_assert_ptr_null(llist_at(llist, -1));
  ck_assert_ptr_null(llist_at(llist, 1));

  /* Edge case 2 */
  llist_delete(llist, 2); /* [] */
  ck_assert_int_eq(llist->sz, 0);
  ck_assert_ptr_null(llist_at(llist, 0));

  /* Edge case 3 */
  llist_free(llist);
  llist = NULL;
  ck_assert_ptr_null(llist);
  ck_assert_ptr_null(llist_at(llist, 0));
}
END_TEST

START_TEST(test_llist_get)
/* Tests that the llist_get(...) function
** returns either the correct llnode containing
** specified data or NULL if no llnodes contain
** the data.
**/
{
  int const NUM_LLNODES = 30;
  llnode_t * llnodes[NUM_LLNODES];

  /* Changed order to ascending */
  llist->order = ASC;

  int i;
  for (i = 0; i < NUM_LLNODES; i++)
  {
    llnodes[i] = llnode_create(i);
    llist_insert(llist, llnodes[i]);
  }

  for (i = 0; i < NUM_LLNODES; i++)
  {
    int data = llnodes[i]->data;
    ck_assert_ptr_eq(llist_get(llist, data), 
                     llnodes[i]);
    ck_assert_ptr_eq(llist_get(llist, data)->next,
                     llnodes[i]->next);
    ck_assert_int_eq(llist_get(llist, data)->data,
                     llnodes[i]->data);
  }

  /* Edge cases:
  **  1. llist->sz = 1
  **  2. llist->sz = 0
  **  3. llist = NULL
  ***/
  llist_free(llist);
  llist = llist_create();
  llist_insert(llist, llnode_create(0)); /* [0] */

  /* Edge case 1 */
  ck_assert_int_eq(llist->sz, 1);
  ck_assert_ptr_nonnull(llist_get(llist,0));
  ck_assert_ptr_null(llist_get(llist, -1));
  ck_assert_ptr_null(llist_get(llist, 1));

  /* Edge case 2 */
  llist_delete(llist, 0); /* [] */
  ck_assert_int_eq(llist->sz, 0);
  ck_assert_ptr_null(llist_get(llist, 0));

  /* Edge case 3 */
  llist_free(llist);
  llist = NULL;
  ck_assert_ptr_null(llist);
  ck_assert_ptr_null(llist_get(llist, 0));
}
END_TEST

START_TEST(test_llist_sort)
/* Tests the llist_sort(...) function works properly.
** Ensures that elements are sorted in the specified
** order.
**/
{
  int const data_asc[] = {1,2,4,8,16,32,64};
  int const data_desc[] = {64,32,16,8,4,2,1};
  int const data_unordered[] = {16,2,8,32,1,64,4};
  int const NUM_LLNODES = 7;
  llnode_t llnodes[NUM_LLNODES];
  
  int i;
  for (i = 0; i < NUM_LLNODES; i++)
    llist_insert(llist, 
                 llnode_create(data_unordered[i]));

  llorder_type_t llorder = llist->order;

  /* Sort in NONE order */
  llist_sort(llist, NONE);
  ck_assert_int_eq(llist->order, llorder);
  ck_assert_ptr_nonnull(llist->head->next);
  ck_assert_ptr_null(llist->tail->next);
  tsds_ck_assert_llist_array_eq(llist->head,
                                data_unordered,
                                NUM_LLNODES);

  /* Sort in ASC order */
  llist_sort(llist, ASC);
  ck_assert_int_eq(llist->order, llorder);
  ck_assert_ptr_nonnull(llist->head->next);
  ck_assert_ptr_null(llist->tail->next);
  tsds_ck_assert_llist_array_eq(llist->head,
                                data_asc,
                                NUM_LLNODES);

  /* Sort in DESC order */
  llist_sort(llist, DESC);
  ck_assert_int_eq(llist->order, llorder);
  ck_assert_ptr_nonnull(llist->head->next);
  ck_assert_ptr_null(llist->tail->next);
  tsds_ck_assert_llist_array_eq(llist->head,
                                data_desc,
                                NUM_LLNODES);

  /* Edge cases:
  **  1. llist->sz = 2
  **  2. llist->sz = 1
  **  3. llist->sz = 0
  **  4. llist = NULL
  ***/
  llist_free(llist);
  llist = llist_create();

  /* Edge case 1: */
  llist_insert(llist, llnode_create(4));
  llist_insert(llist, llnode_create(2)); /* [4,2] */

  ck_assert_int_eq(llist->sz, 2);
  ck_assert_ptr_nonnull(llist_at(llist, 0));
  ck_assert_ptr_nonnull(llist_at(llist, 1));
  ck_assert_int_eq(llist_at(llist, 0)->data, 4);
  ck_assert_int_eq(llist_at(llist, 1)->data, 2);
  ck_assert_ptr_eq(llist->head, llist_get(llist, 4));
  ck_assert_ptr_eq(llist->tail, llist_get(llist, 2));

  llist_sort(llist, ASC); /* [2,4] */

  ck_assert_ptr_nonnull(llist_at(llist, 0));
  ck_assert_ptr_nonnull(llist_at(llist, 1));
  ck_assert_int_eq(llist_at(llist, 0)->data, 2);
  ck_assert_int_eq(llist_at(llist, 1)->data, 4);
  ck_assert_ptr_eq(llist->head, llist_get(llist, 2));
  ck_assert_ptr_eq(llist->tail, llist_get(llist, 4));

  /* Edge case 2: */
  llist_delete(llist, 2); /* [4] */

  ck_assert_int_eq(llist->sz, 1);
  ck_assert_ptr_nonnull(llist_at(llist, 0));

  llist_sort(llist, DESC); /* [4] */
  ck_assert_int_eq(llist_at(llist, 0)->data, 4);
  ck_assert_ptr_eq(llist->head, llist_get(llist, 4));
  ck_assert_ptr_eq(llist->tail, llist_get(llist, 4));

  /* Edge case 3: */
  llist_delete(llist, 4); /* [] */
  
  ck_assert_int_eq(llist->sz, 0);
  ck_assert_ptr_null(llist_at(llist, 0));

  llist_sort(llist, ASC);
  ck_assert_ptr_null(llist_at(llist, 0));
  ck_assert_ptr_null(llist->head);
  ck_assert_ptr_null(llist->tail);

  /* Edge case 4: */
  llist_free(llist);
  llist = NULL;

  ck_assert_ptr_null(llist);

  llist_sort(llist, DESC);
  ck_assert_ptr_null(llist_at(llist, 0));
}
END_TEST

START_TEST(test_llist_change_llorder)
/* Tests the llist_change_llorder(...) function works
** properly. Ensures that elements are ordered in
** the specified order.
**/
{
  int const data_asc[] = {1,2,4,8,16,32,64};
  int const data_desc[] = {64,32,16,8,4,2,1};
  int const data_unordered[] = {16,2,8,32,1,64,4};
  int const NUM_LLNODES = 7;
  llorder_type_t prev_order;

  int i;
  for (i = 0; i < NUM_LLNODES; i++)
    llist_insert(llist, 
                 llnode_create(data_unordered[i]));

  /* Change order to NONE to NONE */
  prev_order = llist->order;
  llist_change_llorder(llist, NONE);
  ck_assert_int_eq(llist->order, NONE);
  ck_assert_int_eq(llist->order, prev_order);
  ck_assert_ptr_nonnull(llist->head->next);
  ck_assert_ptr_null(llist->tail->next);
  tsds_ck_assert_llist_array_eq(llist->head,
                                data_unordered,
                                NUM_LLNODES);

  /* Change order from NONE to DESC */
  prev_order = llist->order;
  llist_change_llorder(llist, DESC);
  ck_assert_int_eq(llist->order, DESC);
  ck_assert_int_ne(llist->order, prev_order);
  ck_assert_ptr_nonnull(llist->head->next);
  ck_assert_ptr_null(llist->tail->next);
  tsds_ck_assert_llist_array_eq(llist->head,
                                data_desc,
                                NUM_LLNODES);

  /* Change order from DESC to ASC */
  prev_order = llist->order;
  llist_change_llorder(llist, ASC);
  ck_assert_int_eq(llist->order, ASC);
  ck_assert_int_ne(llist->order, prev_order);
  ck_assert_ptr_nonnull(llist->head->next);
  ck_assert_ptr_null(llist->tail->next);
  tsds_ck_assert_llist_array_eq(llist->head,
                                data_asc,
                                NUM_LLNODES);

  /* Change order from ASC to NONE */
  prev_order = llist->order;
  llist_change_llorder(llist, NONE);
  ck_assert_int_eq(llist->order, NONE);
  ck_assert_int_ne(llist->order, prev_order);
  ck_assert_ptr_nonnull(llist->head->next);
  ck_assert_ptr_null(llist->tail->next);
  tsds_ck_assert_llist_array_eq(llist->head,
                                data_asc,
                                NUM_LLNODES);
  /* Edge cases:
  **  1. llist->sz = 2
  **  2. llist->sz = 1
  **  3. llist->sz = 0
  **  4. llist = NULL
  ***/
  llist_free(llist);
  llist = llist_create();

  /* Edge case 1: */
  llist_insert(llist, llnode_create(4));
  llist_insert(llist, llnode_create(2)); /* [4,2] */

  ck_assert_int_eq(llist->sz, 2);
  ck_assert_ptr_nonnull(llist_at(llist, 0));
  ck_assert_ptr_nonnull(llist_at(llist, 1));
  ck_assert_int_eq(llist_at(llist, 0)->data, 4);
  ck_assert_int_eq(llist_at(llist, 1)->data, 2);
  ck_assert_ptr_eq(llist->head, llist_get(llist, 4));
  ck_assert_ptr_eq(llist->tail, llist_get(llist, 2));

  llist_change_llorder(llist, ASC); /* [2,4] */

  ck_assert_ptr_nonnull(llist_at(llist, 0));
  ck_assert_ptr_nonnull(llist_at(llist, 1));
  ck_assert_int_eq(llist_at(llist, 0)->data, 2);
  ck_assert_int_eq(llist_at(llist, 1)->data, 4);
  ck_assert_ptr_eq(llist->head, llist_get(llist, 2));
  ck_assert_ptr_eq(llist->tail, llist_get(llist, 4));

  /* Edge case 2: */
  llist_delete(llist, 2); /* [4] */

  ck_assert_int_eq(llist->sz, 1);
  ck_assert_ptr_nonnull(llist_at(llist, 0));

  llist_change_llorder(llist, DESC); /* [4] */
  ck_assert_int_eq(llist_at(llist, 0)->data, 4);
  ck_assert_ptr_eq(llist->head, llist_get(llist, 4));
  ck_assert_ptr_eq(llist->tail, llist_get(llist, 4));

  /* Edge case 3: */
  llist_delete(llist, 4); /* [] */
  
  ck_assert_int_eq(llist->sz, 0);
  ck_assert_ptr_null(llist_at(llist, 0));

  llist_change_llorder(llist, ASC);
  ck_assert_ptr_null(llist_at(llist, 0));
  ck_assert_ptr_null(llist->head);
  ck_assert_ptr_null(llist->tail);

  /* Edge case 4: */
  llist_free(llist);
  llist = NULL;

  ck_assert_ptr_null(llist);

  llist_change_llorder(llist, DESC);
  ck_assert_ptr_null(llist_at(llist, 0));

}
END_TEST

START_TEST(test_mt_llist_insert)
/* Tests the thread-safety of the llist_insert(...)
** function. Insertion is done in both ascending and
** descending order.
**/
{
  int num;
  int const NUM_LLNODES = 15;
  pthread_t threads[NUM_LLNODES];
  tsds_llarg_t llargs[NUM_LLNODES];
  llnode_t * cur;

  int i;
  for (i = 0; i < NUM_LLNODES; i++)
  {
    llargs[i].llist = llist;
    llargs[i].data = i;
  }

  llist->order = ASC;
  tsds_create_nthreads(threads, 
                       tsds_llist_insert, 
                       llargs, 
                       NUM_LLNODES);
  tsds_join_nthreads(threads, NUM_LLNODES);
  ck_assert_uint_eq(llist->sz, NUM_LLNODES);

  num = 0;
  cur = llist->head;
  /* Check that all llnodes have been inserted and
     are present and in ascending order. */
  while (cur)
  {
    ck_assert_int_eq(cur->data, num++);
    cur = cur->next;
  }
  
  llist_free(llist);
  llist = llist_create();

  for (i = 0; i < NUM_LLNODES; i++)
  {
    llargs[i].llist = llist;
    llargs[i].data = i;
  }

  llist->order = DESC;
  tsds_create_nthreads(threads, 
                       tsds_llist_insert, 
                       llargs, 
                       NUM_LLNODES);
  tsds_join_nthreads(threads, NUM_LLNODES);
  ck_assert_uint_eq(llist->sz, NUM_LLNODES);

  num = NUM_LLNODES - 1;
  cur = llist->head;
  /* Check that all llnodes have been inserted and
     are present and in descending order. */
  while (cur)
  {
    ck_assert_int_eq(cur->data, num--);
    cur = cur->next;
  }
}
END_TEST

START_TEST(test_mt_llist_at)
/* Tests the thread-safety of llist_at() function.
** Inserts NUM_LLNODES nodes into llist and then
** spawns NUM_LLNODES number of threads. Each thread
** retrieves a unique node in parallel.
**/
{
  int const NUM_LLNODES = 100;

  pthread_t threads[NUM_LLNODES];
  tsds_llarg_t llargs[NUM_LLNODES];

  llist_change_llorder(llist, ASC);

  int i, r;
  /* Insert llnodes [0, NUM_LLNODES) in llist */
  for (i = 0; i < NUM_LLNODES; i++)
  {
    llargs[i].llnode = llnode_create(i);
    llist_insert(llist, llargs[i].llnode);
  }

  /* Index into llist and get node */
  for (i = 0; i < NUM_LLNODES; i++)
  {
    llargs[i].llist = llist;
    llargs[i].idx = i;
  }
  tsds_create_nthreads(threads,
                       tsds_llist_at,
                       llargs,
                       NUM_LLNODES);

  /* join threads and assert that the correct
     llnodes have been retreived */
  for (i = 0; i < NUM_LLNODES; i++)
  {
    void * llnode;
    r = pthread_join(threads[i], &llnode);
    handle_error(r, "pthread_join");

    ck_assert_ptr_nonnull((llnode_t *)llnode);
    ck_assert_ptr_eq(llargs[i].llnode, (llnode_t *)llnode);
  }
}
END_TEST

START_TEST(test_mt_llist_get)
/* Tests thread-safety of llist_get() function.
** Inserts NUM_LLNODES nodes into llist and then
** spawns NUM_LLNODES number of threads. Half of
** the threads spawned will retrieve a unique node
** while the rest will insert a node.
**/
{
  int const NUM_ORDERS = 3; /* [ASC, DESC, NONE] */
  int const NUM_LLNODES = 100;
  pthread_t threads[NUM_LLNODES];
  tsds_llarg_t llargs[NUM_LLNODES];

  int i;
  for (i = 0; i < NUM_LLNODES; i++) 
    llist_insert(llist, llnode_create(i)); 

  ck_assert_uint_eq(llist->sz, NUM_LLNODES);

  int order;
  for (order = 0; order < NUM_ORDERS; order++)
  {
    llist_change_llorder(llist, order);
    ck_assert_int_eq(tsds_llist_has_llorder(order), 1);

    int j, r;
    for (j = 0; j < NUM_LLNODES; j++)
    {
      llargs[j].llist = llist;
      llargs[j].data = j;
      llargs[j].idx = j;

      if (j % 2 == 0)
      {
        r = pthread_create(&threads[j],
                           NULL,
                           &tsds_llist_get,
                           (void *)&llargs[j]);
        handle_error(r, "pthread_create");
      }
      else
      {
        r = pthread_create(&threads[j],
                           NULL,
                           &tsds_llist_insert,
                           (void *)&llargs[j]);
        handle_error(r, "pthread_create");
      }
    }

    for (j = 0; j < NUM_LLNODES; j++)
    {
      if (j % 2 == 0)
      {
        void * vptr;
        r = pthread_join(threads[j],&vptr);
        handle_error(r, "pthread_join");
        
        llnode_t * llnode = (llnode_t *) vptr;
        ck_assert_ptr_nonnull(llnode);
        ck_assert_int_eq(llargs[j].data, llnode->data);
      }
      else
      {
        r = pthread_join(threads[j], NULL);
        handle_error(r, "pthread_join");
      }
    }
  }
  ck_assert_uint_eq(llist->sz, 
                    NUM_LLNODES + (NUM_ORDERS*(NUM_LLNODES/2)));
}
END_TEST

Suite * 
llist_suite(void)
{
  Suite * suite;
  TCase * tc_core;

  suite = suite_create("Linked List");

  tc_core = tcase_create("Core");
  tcase_add_checked_fixture(tc_core, setup, teardown);
  tcase_set_timeout(tc_core, 0.0); /* Disables timeout */

  /* Single threaded tests */
  tcase_add_test(tc_core, test_llist_size);
  tcase_add_test(tc_core, test_llist_create_with_llorder);
  tcase_add_test(tc_core, test_llist_head_tail);
  tcase_add_test(tc_core, test_llist_at);
  tcase_add_test(tc_core, test_llist_get);
  tcase_add_test(tc_core, test_llist_sort);
  tcase_add_test(tc_core, test_llist_change_llorder);

  /* Multithreaded tests */
  tcase_add_test(tc_core, test_mt_llist_insert);
  tcase_add_test(tc_core, test_mt_llist_at);
  tcase_add_test(tc_core, test_mt_llist_get);

  suite_add_tcase(suite, tc_core);

  return suite;
}

int
main(int argc, char* argv[]) 
{
  int num_tests_failed;

  Suite * suite;
  SRunner *suite_runner;

  suite = llist_suite();
  suite_runner = srunner_create(suite);

  /* Prevents test suite from forking to catch segfault bug */
  //srunner_set_fork_status(suite_runner, CK_NOFORK);

  srunner_run_all(suite_runner, CK_NORMAL);
  num_tests_failed = srunner_ntests_failed(suite_runner);
  srunner_free(suite_runner);

  return (num_tests_failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}
