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
      printf("%d%s", cur->data, cur->next ? ", " : "]\n");
      cur = cur->next;
    }
  }
}
