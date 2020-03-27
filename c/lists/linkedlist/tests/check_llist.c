#include <time.h>
#include <stdio.h>
#include <check.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/types.h>

#include "../headers/llist.h"
#include "../headers/utils.h"

/* Globals */
llist_t * llist;

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
/* Helper functions                      */
/*---------------------------------------*/
/* May need to duplicate api with functions
** that can be passed to pthread_create and
** call actual funcs being tested from w/i
** these tests dups.
**/
typedef struct {
  llist_t * llist;
  int num;
} tsds_ck_llist_arg_t;

void *
tsds_ck_llist_insert(void * _arg)
{
  tsds_ck_llist_arg_t * arg = (tsds_ck_llist_arg_t *)_arg;

  int i;
  unsigned int seed;

  seed = (unsigned int)(time(NULL)/(arg->num + 1));
  for (i = rand_r(&seed); i < 1000000; i++);

  llist_insert(arg->llist, llnode_create(arg->num));
  return 0;
}

void
tsds_ck_join_threads(pthread_t * threads, int const NUM_THREADS)
{
  int i, r;
  for (i = 0; i < NUM_THREADS; i++)
  {
    r = pthread_join(threads[i], NULL);
    if (r != 0)
      fprintf(stderr, "Error: pthread_join\n");
  }
}

void
tsds_ck_llist_insert_llnodes(pthread_t * threads, int const NUM_LLNODES)
{
  tsds_ck_llist_arg_t args[NUM_LLNODES];

  int i, r;
  for (i = 0; i < NUM_LLNODES; i++)
  {
    args[i].llist = llist;
    args[i].num = i;

    r = pthread_create(&threads[i], NULL,
                       &tsds_ck_llist_insert,
                       (void *)&args[i]);
    if (r != 0)
      fprintf(stderr, "Error: pthread_create\n");
  }

  tsds_ck_join_threads(threads, NUM_LLNODES);
}

void
tsds_ck_compare_llnode_data(llnode_t * llnode,
                            int const * const data,
                            int const sz)
{
  int i;
  for (i = 0; i < sz && llnode; i++, llnode = llnode->next)
    ck_assert_int_eq(llnode->data, data[i]);
}

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

  llist_insert(llist, n2);
  llist_insert(llist, n3);
  ck_assert_uint_eq(llist->sz, 4);

  llist_delete(llist, 1);
  ck_assert_uint_eq(llist->sz, 3);

  llist_delete(llist, 1);
  ck_assert_uint_eq(llist->sz, 3);

  llist_delete(llist, 4);
  ck_assert_uint_eq(llist->sz, 2);

  llist_delete(llist, 2);
  ck_assert_uint_eq(llist->sz, 2);

  llist_delete(llist, -8);
  ck_assert_uint_eq(llist->sz, 1);

  llist_delete(llist, 0);
  ck_assert_uint_eq(llist->sz, 0);

  llist_delete(llist, 1);
  ck_assert_uint_eq(llist->sz, 0);

  llist_delete(llist, 4);
  ck_assert_uint_eq(llist->sz, 0);
}
END_TEST

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

START_TEST(test_llist_index_into_llist)
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

  ck_assert_ptr_null(llist_at(llist, -1));
  ck_assert_ptr_null(llist_at(llist, NUM_LLNODES));
}
END_TEST

START_TEST(test_llist_get_element)
/* Tests that the llist_get(...) function
** returns either the correct llnode containing
** specified data or NULL if no llnodes contain
** the data.
** TODO: Add edge cases.
**        1. llist->sz is 1
**        2. llist is null
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

  ck_assert_ptr_null(llist_get(llist, -1));
  ck_assert_ptr_null(llist_get(llist, NUM_LLNODES));
}
END_TEST

START_TEST(test_llist_sort)
/* Tests the llist_sort(...) function works properly.
** Ensures that elements are sorted in the specified
** order.
** TODO: Add edge cases.
**        1. llist->sz is 0 or 1
**        2. llist is null
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

  order_type_t llorder = llist->order;

  /* Sort in NONE order */
  llist_sort(llist, NONE);
  ck_assert_int_eq(llist->order, llorder);
  ck_assert_ptr_nonnull(llist->head->next);
  ck_assert_ptr_null(llist->tail->next);
  tsds_ck_compare_llnode_data(llist->head,
                      data_unordered,
                      NUM_LLNODES);

  /* Sort in ASC order */
  llist_sort(llist, ASC);
  ck_assert_int_eq(llist->order, llorder);
  ck_assert_ptr_nonnull(llist->head->next);
  ck_assert_ptr_null(llist->tail->next);
  tsds_ck_compare_llnode_data(llist->head,
                      data_asc,
                      NUM_LLNODES);

  /* Sort in DESC order */
  llist_sort(llist, DESC);
  ck_assert_int_eq(llist->order, llorder);
  ck_assert_ptr_nonnull(llist->head->next);
  ck_assert_ptr_null(llist->tail->next);
  tsds_ck_compare_llnode_data(llist->head,
                      data_desc,
                      NUM_LLNODES);
}
END_TEST

START_TEST(test_llist_change_order)
/* Tests the llist_change_order(...) function works
** properly. Ensures that elements are ordered in
** the specified order.
** TODO: Add edge cases.
**        1. llist->sz is 0 or 1
**        2. llist is null
**/
{
  int const data_asc[] = {1,2,4,8,16,32,64};
  int const data_desc[] = {64,32,16,8,4,2,1};
  int const data_unordered[] = {16,2,8,32,1,64,4};
  int const NUM_LLNODES = 7;
  llnode_t llnodes[NUM_LLNODES];
  order_type_t prev_order;

  int i;
  for (i = 0; i < NUM_LLNODES; i++)
    llist_insert(llist, 
                 llnode_create(data_unordered[i]));

  /* Change order to NONE to NONE */
  prev_order = llist->order;
  llist_change_order(llist, NONE);
  ck_assert_int_eq(llist->order, NONE);
  ck_assert_int_eq(llist->order, prev_order);
  ck_assert_ptr_nonnull(llist->head->next);
  ck_assert_ptr_null(llist->tail->next);
  tsds_ck_compare_llnode_data(llist->head,
                              data_unordered,
                              NUM_LLNODES);

  /* Change order from NONE to DESC */
  prev_order = llist->order;
  llist_change_order(llist, DESC);
  ck_assert_int_eq(llist->order, DESC);
  ck_assert_int_ne(llist->order, prev_order);
  ck_assert_ptr_nonnull(llist->head->next);
  ck_assert_ptr_null(llist->tail->next);
  tsds_ck_compare_llnode_data(llist->head,
                              data_desc,
                              NUM_LLNODES);

  /* Change order from DESC to ASC */
  prev_order = llist->order;
  llist_change_order(llist, ASC);
  ck_assert_int_eq(llist->order, ASC);
  ck_assert_int_ne(llist->order, prev_order);
  ck_assert_ptr_nonnull(llist->head->next);
  ck_assert_ptr_null(llist->tail->next);
  tsds_ck_compare_llnode_data(llist->head,
                              data_asc,
                              NUM_LLNODES);

  /* Change order from ASC to NONE */
  prev_order = llist->order;
  llist_change_order(llist, NONE);
  ck_assert_int_eq(llist->order, NONE);
  ck_assert_int_ne(llist->order, prev_order);
  ck_assert_ptr_nonnull(llist->head->next);
  ck_assert_ptr_null(llist->tail->next);
  tsds_ck_compare_llnode_data(llist->head,
                              data_asc,
                              NUM_LLNODES);
}
END_TEST

START_TEST(test_mt_llist_insert)
/* Tests the thread-safety of the llist_insert(...)
** function. Both insertion in ascending and descending
** order.
**/
{
  int num;
  int const NUM_LLNODES = 15;
  pthread_t threads[NUM_LLNODES];
  llnode_t * cur;

  llist->order = ASC;
  tsds_ck_llist_insert_llnodes(threads, NUM_LLNODES);
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

  llist->order = DESC;
  tsds_ck_llist_insert_llnodes(threads, NUM_LLNODES);
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

Suite * 
llist_suite(void)
{
  Suite * suite;
  TCase * tc_core;

  suite = suite_create("Linked List");

  tc_core = tcase_create("Core");
  tcase_add_checked_fixture(tc_core, setup, teardown);

  /* Single thread tests */
  tcase_add_test(tc_core, test_llist_size);
  tcase_add_test(tc_core, test_llist_head_tail);
  tcase_add_test(tc_core, test_llist_index_into_llist);
  tcase_add_test(tc_core, test_llist_get_element);
  tcase_add_test(tc_core, test_llist_sort);
  tcase_add_test(tc_core, test_llist_change_order);

  /* Multithreaded tests */
  tcase_add_test(tc_core, test_mt_llist_insert);

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
