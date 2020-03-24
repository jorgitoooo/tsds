#include <stdlib.h>

#include "./headers/llist.h"
#include "./headers/utils.h"

int
main(int argc, char ** argv)
{
  if (argc < 2) 
    return 0;

  order_type order = (order_type) atoi(argv[1]);

  llnode_t * n0 = create_llnode(0);
  llnode_t * n1 = create_llnode(5);
  llnode_t * n2 = create_llnode(6);
  llnode_t * n3 = create_llnode(9);
  llnode_t * n4 = create_llnode(13);
  llnode_t * n5 = create_llnode(17);
  llnode_t * n6 = create_llnode(0);
  llnode_t * n7 = create_llnode(17);

  llist_t * list = create_llist_with_order(order);

  insert_llnode(list, n4);
  insert_llnode(list, n1);
  insert_llnode(list, n2);
  insert_llnode(list, n3);
  insert_llnode(list, n0);
  insert_llnode(list, n5);
  insert_llnode(list, n6);
  insert_llnode(list, n7);

  print(list);

  delete_llnode(list, 13);
  print(list);
  delete_llnode(list, 14);
  delete_llnode(list, 0);
  print(list);
  delete_llnode(list, 17);
  print(list);
  delete_llnode(list, 17);
  print(list);
  delete_llnode(list, 9);
  print(list);
  delete_llnode(list, 6);
  print(list);
  delete_llnode(list, 5);
  print(list);
  delete_llnode(list, 0);
  print(list);
  delete_llnode(list, 12);
  print(list);
  delete_llnode(list, 0);
  print(list);

  puts("Before free()");
  print_state(list);
  free_llist(list);
  puts("After free()");
  print_state(list);
  puts("");

  return 0;
}
