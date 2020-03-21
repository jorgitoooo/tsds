#include "./headers/llist.h"
#include "./headers/utils.h"

int
main(int argc, char ** argv)
{
  int i;
  for (i = 1; i <= 2; i++)
  {
    order_type order = i == 1 ? ASC : DESC;

    llnode_t * n0 = create_node(0);
    llnode_t * n1 = create_node(5);
    llnode_t * n2 = create_node(6);
    llnode_t * n3 = create_node(9);
    llnode_t * n4 = create_node(13);
    llnode_t * n5 = create_node(17);
    llnode_t * n6 = create_node(0);
    llnode_t * n7 = create_node(17);

    llist_t list;

    init(&list, order);

    insert_node(&list, n4);
    insert_node(&list, n1);
    insert_node(&list, n2);

    print(&list);

    insert_node(&list, n3);
    insert_node(&list, n0);
    insert_node(&list, n5);

    print(&list);

    insert_node(&list, n6);
    insert_node(&list, n7);

    print(&list);

    puts("");
  }

  return 0;
}
