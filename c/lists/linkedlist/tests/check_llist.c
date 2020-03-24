#include <stdio.h>
#include <check.h>

#include "../headers/llist.h"
#include "../headers/utils.h"

/* Globals */
llist_t * list;

void
setup(void)
{
  puts("setup()");
  list = create_llist_with_order(NONE);
}

void
teardown(void)
{
  puts("teardown()");
  free_llist(list);
}


START_TEST (llist_size_test)
{ 
  llnode_t * n0;
  llnode_t * n1;
  llnode_t * n2;
  llnode_t * n3;

  ck_assert_uint_eq(list->sz, 0);

  n0 = create_llnode(0);
  n1 = create_llnode(1);
  n2 = create_llnode(4);
  n3 = create_llnode(-8);

  insert_llnode(list, n0);
  insert_llnode(list, n1);
  ck_assert_uint_eq(list->sz, 2);

  insert_llnode(list, n2);
  insert_llnode(list, n3);
  ck_assert_uint_eq(list->sz, 4);

  delete_llnode(list, 1);
  ck_assert_uint_eq(list->sz, 3);

  delete_llnode(list, 1);
  ck_assert_uint_eq(list->sz, 3);

  delete_llnode(list, 4);
  ck_assert_uint_eq(list->sz, 2);

  delete_llnode(list, 2);
  ck_assert_uint_eq(list->sz, 2);

  delete_llnode(list, -8);
  ck_assert_uint_eq(list->sz, 1);

  delete_llnode(list, 0);
  ck_assert_uint_eq(list->sz, 0);

  delete_llnode(list, 1);
  ck_assert_uint_eq(list->sz, 0);

  delete_llnode(list, 4);
  ck_assert_uint_eq(list->sz, 0);
}
END_TEST

START_TEST(head_tail_test)
{
  llnode_t * head;
  llnode_t * tail;
  llnode_t * n0;
  llnode_t * n1;
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

  tcase_add_test(tc_core, llist_size_test);
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
