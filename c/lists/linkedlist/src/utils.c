#include "../headers/llist.h"
#include "../headers/utils.h"

void
print(llist_t const * const list)
{
  static char order_msg[128];
  if (list)
  {
    if (list->order == ASC)
      strcpy(order_msg, "Ascending order");
    else if (list->order == DESC)
      strcpy(order_msg, "Descending order");
    else
      strcpy(order_msg, "Unordered");

    printf("%s list of size %zu:\n [", order_msg, list->sz);

    llnode_t * cur = list->head;
    while (cur)
    {
      printf("%d%s", cur->data, cur->next ? ", " : "");
      fflush(stdout);
      cur = cur->next;
    }
    puts("]\n");
  }
}

void
print_node(llnode_t const * const node)
{
  if (node)
    printf("Node {\n    address: %p,\n    data: %d,\n    next: %p\n  }\n", node, node->data, node->next);
  else
    printf("NULL\n");
}

void
print_state(llist_t const * const list)
{
  if (list)
    printf("List {\n");
    printf("  address: %p,\n", list);
    printf("  head: "); print_node(list->head);
    printf("  tail: "); print_node(list->tail);
    printf("  sz: %zu\n", list->sz);
    printf("}\n");
}
