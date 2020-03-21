#include "../headers/llist.h"
#include "../headers/utils.h"

void
print(llist_t * list)
{
  if (list)
  {
    llnode_t * cur = list->head;
    printf("List of size %zu:\n [", list->sz);
    while (cur)
    {
      printf("%d%s", cur->data, cur->next ? ", " : "]\n");
      cur = cur->next;
    }
  }
}
