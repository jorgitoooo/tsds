#include "../headers/llist.h"
#include "../headers/utils.h"

void
print(llist_t * list)
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
      cur = cur->next;
    }
    puts("]\n");
  }
}

void
print_node(llnode_t * node)
{
  if (node)
    printf("Node {\n  address: %p,\n  data: %d,\n  next: %p\n }\n", node, node->data, node->next);
  else
    printf("NULL\n");
}

void
print_state(llist_t * list)
{
  if (list)
    //printf("List {\n head: %p,\n tail: %p,\n sz: %zu\n}\n", list->head, list->tail, list->sz);
    printf("List {\n head: ");
    print_node(list->head);
    printf(" tail: ");
    print_node(list->tail);
    printf(" sz: %zu\n}\n", list->sz);
}
