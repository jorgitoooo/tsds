#include "./headers/bintree.h"
#include "./headers/utils.h"

int main(int argc, char **argv)
{
  bintree_t *bt;
  init(&bt);

  node_t * n0 = new_node(19, "root");
  node_t * n1 = new_node(21, "root right");
  node_t * n2 = new_node(18, "root left");
  node_t * n3 = new_node(17, "root left left");
  node_t * n4 = new_node(20, "root right left");

  insert(bt, n0);
  insert(bt, n1);
  insert(bt, n2);
  insert(bt, n3);
  insert(bt, n4);

  print(bt, PRE);
  print(bt, IN);
  print(bt, POST);

  print_node(find(bt, 19));
  print_node(find(bt, 20));
  print_node(find(bt, 18));
  print_node(find(bt, 49));

 free_bt(bt);
  
  return 0;
}
