#ifndef LIST_H
#define LIST_H

typedef struct lst{
  void *args;
  struct lst *next;
}list_t;

typedef struct pool{
  pthread_mutex_t lock;
  list_t *head;
  list_t *tail;
}pool_t;

pool_t* create_pool();
list_t* create_list();
list_t* add_pool_head(pool_t *pool, void *element);
list_t* add_pool_tail(pool_t *pool, void *element);
void* delete_element(pool_t *pool, int (*cmp) (void *,void *),void *args);
void empty_pool(pool_t *pool);
void delete_list(pool_t *pool);
list_t* search(pool_t *pool, int (*cmp) (void *,void *),void *args);
int exec_on_elem(pool_t *pool,  int (*exec)(void*) );
void exec_on_elem_targs(pool_t *pool,  void (*exec)(void*,void *),void* );
void* remove_element(pool_t *pool, list_t *node, list_t *prev);
#endif
