#include <stdlib.h>

#include "./headers/llist.h"
#include "./headers/utils.h"

int
main(int argc, char ** argv)
{
  if (argc < 2) 
    return 0;

  order_type order = (order_type) atoi(argv[1]);

  llnode_t * n0 = llnode_create(0);
  llnode_t * n1 = llnode_create(5);
  llnode_t * n2 = llnode_create(6);
  llnode_t * n3 = llnode_create(9);
  llnode_t * n4 = llnode_create(13);
  llnode_t * n5 = llnode_create(17);
  llnode_t * n6 = llnode_create(0);
  llnode_t * n7 = llnode_create(17);

  llist_t * list = llist_create_with_order(order);

  llist_insert(list, n4);
  llist_insert(list, n1);
  llist_insert(list, n2);
  llist_insert(list, n3);
  llist_insert(list, n0);
  llist_insert(list, n5);
  llist_insert(list, n6);
  llist_insert(list, n7);

  print(list);

  llist_delete(list, 13);
  print(list);
  llist_delete(list, 14);
  llist_delete(list, 0);
  print(list);
  llist_delete(list, 17);
  print(list);
  llist_delete(list, 17);
  print(list);
  llist_delete(list, 9);
  print(list);
  llist_delete(list, 6);
  print(list);
  llist_delete(list, 5);
  print(list);
  llist_delete(list, 0);
  print(list);
  llist_delete(list, 12);
  print(list);
  llist_delete(list, 0);
  print(list);

  puts("Before free()");
  print_state(list);
  llist_free(list);
  puts("After free()");
  print_state(list);
  puts("");

  return 0;
}
