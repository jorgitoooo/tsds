#include <stdlib.h>

#include "./headers/llist.h"
#include "./headers/utils.h"

int
main(int argc, char ** argv)
{
  if (argc < 2) 
    return 0;

  order_type order = (order_type) atoi(argv[1]);

  llnode_t * n0 = create_node(0);
  llnode_t * n1 = create_node(5);
  llnode_t * n2 = create_node(6);
  llnode_t * n3 = create_node(9);
  llnode_t * n4 = create_node(13);
  llnode_t * n5 = create_node(17);
  llnode_t * n6 = create_node(0);
  llnode_t * n7 = create_node(17);

  llist_t list;

  init_with_order(&list, order);

  insert_node(&list, n4);
  insert_node(&list, n1);
  insert_node(&list, n2);
  insert_node(&list, n3);
  insert_node(&list, n0);
  insert_node(&list, n5);
  insert_node(&list, n6);
  insert_node(&list, n7);

  print(&list);

  delete_node(&list, 13);
  print(&list);
  delete_node(&list, 14);
  delete_node(&list, 0);
  print(&list);
  delete_node(&list, 17);
  print(&list);
  delete_node(&list, 17);
  print(&list);
  delete_node(&list, 9);
  print(&list);
  delete_node(&list, 6);
  print(&list);
  delete_node(&list, 5);
  print(&list);
  delete_node(&list, 0);
  print(&list);
  delete_node(&list, 12);
  print(&list);
  delete_node(&list, 0);
  print(&list);

  // DEBUG
  print_state(&list);
  
  while (list.head)
  {
    llnode_t * tmp = list.head->next;
    free(list.head);
    list.head = tmp;
  }
  //

  puts("");

  return 0;
}
