#include <semaphore.h>
#include <pthread.h>
#include "../headers/bintree.h"

sem_t mutex, turnstile, read_write;

void init_sems()
{
  sem_init(&mutex, 0, 1);
  sem_init(&turnstile, 0, 1);
  sem_init(&read_write, 0, 1);
}

/*
 * NOT THREAD SAFE
 * Initializers/creational functions should be called
 * before threads are spawned
 */
node_t* new_node(int key, char *value)
{
  node_t *node  = (node_t *)malloc(sizeof(node_t));
  
  node->key = key;
  node->value = (char *)malloc(strlen(value) + 1);
  strcpy(node->value, value);

  node->left = NULL;
  node->right = NULL;
  
  return node;
}

bintree_t* init_bt_with_head(int head_key, char* value)
{
  bintree_t *bt = (bintree_t *)malloc(sizeof(bintree_t));
  bt->head = new_node(head_key, value);
  return bt;
}

void init(bintree_t **bt)
{
  init_sems();
  *bt = (bintree_t *)malloc(sizeof(bintree_t));
}

/*
 * NOT THREAD SAFE
 * Helper function only used by find
 */
void free_subtree(node_t *n)
{
  if(n)
  {
    free_subtree(n->left);
    free_subtree(n->right);
    free(n->value);
    free(n);
  }
}

void free_bt(bintree_t *bt)
{
  sem_wait(&read_write);
  sem_wait(&mutex);

  if(bt)
  {
    free_subtree(bt->head);
    free(bt);
  }
  
  sem_post(&mutex);
  sem_post(&read_write);
}

/*
 * NOT THREAD SAFE
 * Helper function only used by insert
 */
int insert_node(node_t **bt_n, node_t *n)
{
  if(!*bt_n || (*bt_n)->key == n->key)
  {
    *bt_n = n;
    return 0;
  }
  
  if((*bt_n)->key < n->key) return insert_node( &(*bt_n)->right, n);
  return insert_node( &(*bt_n)->left, n);
}

// Note: May have to pass *bt by reference
int insert(bintree_t *bt, node_t *n)
{
  int ret = 0;

  sem_wait(&turnstile);
  sem_wait(&read_write);

  if(bt && bt->head && n)
  {
    if(bt->head->key == n->key)
    {
      bt->head = n;
      ret = 0;
    }
    else if(bt->head->key < n->key)
    {
      ret = insert_node(&bt->head->right, n);
    }
    else
    {
      ret = insert_node(&bt->head->left, n);
    }
  }
  else
  {
    if(bt && n && !bt->head)
      bt->head = n;
    else
      ret = -1;
  }

  sem_post(&read_write);
  sem_post(&turnstile);

  return ret;
}

/*
 * NOT THREAD SAFE
 * Helper function only used by find
 */
node_t* find_in_subtree(node_t *n, int key)
{
  node_t *node = NULL;

  if(n)
  {
    if(n->key == key)
      node = n;
    else if(n->key < key)
      node = find_in_subtree(n->right, key);
    else
      node = find_in_subtree(n->left, key);
  }

  return node;
}

node_t* find(bintree_t *bt, int key)
{
  static int readers_count = 0;

  node_t *n = NULL;

  sem_wait(&turnstile);
  sem_post(&turnstile);

  sem_wait(&mutex);

  readers_count++;
  if(readers_count == 1)
    sem_wait(&read_write);

  sem_post(&mutex);

  if(bt && bt->head)
    n = find_in_subtree(bt->head, key);

  sem_wait(&mutex);

  readers_count--;
  if(readers_count == 0)
    sem_post(&read_write);

  sem_post(&mutex);

  return n;
}

