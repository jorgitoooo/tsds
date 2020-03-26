#include <stdio.h>
#include <check.h>

#include "../headers/llist.h"
#include "../headers/utils.h"

/* Globals */
llist_t * list;

void
setup(void)
{
  list = llist_create();
}

void
teardown(void)
{
  llist_free(list);
}

START_TEST(test_llist_size)
// Tests for correct incrementing 
// and decrementing of linked list
// size after insertion and deletion
// operations.
{ 
  llnode_t * n0 = llnode_create(0);
  llnode_t * n1 = llnode_create(1);
  llnode_t * n2 = llnode_create(4);
  llnode_t * n3 = llnode_create(-8);

  ck_assert_uint_eq(list->sz, 0);

  llist_insert(list, n0);
  llist_insert(list, n1);
  ck_assert_uint_eq(list->sz, 2);

  llist_insert(list, n2);
  llist_insert(list, n3);
  ck_assert_uint_eq(list->sz, 4);

  llist_delete(list, 1);
  ck_assert_uint_eq(list->sz, 3);

  llist_delete(list, 1);
  ck_assert_uint_eq(list->sz, 3);

  llist_delete(list, 4);
  ck_assert_uint_eq(list->sz, 2);

  llist_delete(list, 2);
  ck_assert_uint_eq(list->sz, 2);

  llist_delete(list, -8);
  ck_assert_uint_eq(list->sz, 1);

  llist_delete(list, 0);
  ck_assert_uint_eq(list->sz, 0);

  llist_delete(list, 1);
  ck_assert_uint_eq(list->sz, 0);

  llist_delete(list, 4);
  ck_assert_uint_eq(list->sz, 0);
}
END_TEST

START_TEST(test_llist_head_tail)
// Tests that head and tail pointers
// are correctly assigned and maintained
// throughout the life of the llist_t
// structure.
{
  llnode_t * n_0   = llnode_create(0);
  llnode_t * n_3   = llnode_create(3);
  llnode_t * n_295 = llnode_create(295);
  llnode_t * n_38  = llnode_create(38);

  llist_insert(list, n_0);
  llist_insert(list, n_3);
  llist_insert(list, n_295);
  llist_insert(list, n_38);
  /* [0, 3, 295, 38]  */

  ck_assert_ptr_eq(list->head, n_0);
  ck_assert_ptr_eq(list->tail, n_38);
  /* [0, 3, 295, 38]  */
  /*  |          |    */
  /*  head       tail */

  llist_delete(list, n_0->data);
  ck_assert_ptr_eq(list->head, n_3);
  ck_assert_ptr_eq(list->tail, n_38);
  /* [3, 295, 38]  */
  /*  |       |    */
  /*  head    tail */

  llist_delete(list, n_295->data);
  ck_assert_ptr_eq(list->head, n_3);
  ck_assert_ptr_eq(list->tail, n_38);
  /* [3,  38]   */
  /*  |    |    */
  /*  head tail */

  llist_delete(list, n_38->data);
  ck_assert_ptr_eq(list->head, n_3);
  ck_assert_ptr_eq(list->tail, n_3);
  /*     [3]     */
  /*      |      */
  /* head - tail */

  llist_delete(list, n_3->data);
  ck_assert_ptr_null(list->head);
  ck_assert_ptr_null(list->tail);
  /*     [ ]     */
  /*      |      */
  /* head - tail */
}
END_TEST 

START_TEST(test_llist_index_into_list)
// Tests that the llist_at(...) function
// returns either the correct node residing
// at the specified index or NULL if index
// is out of bounds.
{
  int const NUM_LLNODES = 30;
  llnode_t * nodes[NUM_LLNODES];

  /* Changed order to ascending */
  list->order = ASC;

  int i;
  for (i = 0; i < NUM_LLNODES; i++)
  {
    nodes[i] = llnode_create(i);
    llist_insert(list, nodes[i]);
  }

  for (i = 0; i < NUM_LLNODES; i++)
    ck_assert_ptr_eq(llist_at(list, i), nodes[i]);

  ck_assert_ptr_null(llist_at(list, -1));
  ck_assert_ptr_null(llist_at(list, NUM_LLNODES));
}
END_TEST

START_TEST(test_llist_get_element)
// Tests that the llist_get(...) function
// returns either the correct node containing
// specified data or NULL if no nodes contain
// the data.
{
  int const NUM_LLNODES = 30;
  llnode_t * nodes[NUM_LLNODES];

  /* Changed order to ascending */
  list->order = ASC;

  int i;
  for (i = 0; i < NUM_LLNODES; i++)
  {
    nodes[i] = llnode_create(i);
    llist_insert(list, nodes[i]);
  }

  for (i = 0; i < NUM_LLNODES; i++)
  {
    int data = nodes[i]->data;
    ck_assert_ptr_eq(llist_get(list, data), nodes[i]);
    ck_assert_ptr_eq(llist_get(list, data)->next, nodes[i]->next);
    ck_assert_int_eq(llist_get(list, data)->data, nodes[i]->data);
  }

  ck_assert_ptr_null(llist_get(list, -1));
  ck_assert_ptr_null(llist_get(list, NUM_LLNODES));
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

  tcase_add_test(tc_core, test_llist_size);
  tcase_add_test(tc_core, test_llist_head_tail);
  tcase_add_test(tc_core, test_llist_index_into_list);
  tcase_add_test(tc_core, test_llist_get_element);
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
