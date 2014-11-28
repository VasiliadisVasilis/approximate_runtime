#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <assert.h>
#include "list.h"


pool_t* create_pool(){
  pool_t *new =(pool_t *) calloc(1, sizeof(pool_t));
  assert(new);
  pthread_mutex_init(&new->lock, NULL);
  new->head=NULL;
  new->tail=NULL;
  return new;
}

list_t * create_list(){
  list_t *new =(list_t *) calloc(1, sizeof(list_t));
  assert(new);
  new->args=NULL;
  new->next=NULL;
  return new;
}

list_t* add_pool_head(pool_t *pool, void *element){

  list_t *new = (list_t *) calloc(1, sizeof(list_t));
  assert(new);
  if(!pool->head){
    pool->head = new;
    pool->tail=new;
    new->next = NULL;
    new->args = element;
  }
  else{
    new->args = element;
    new->next = pool->head;
    pool->head=new;
  }

  return new;
}

list_t* add_pool_tail(pool_t *pool, void *element){

  list_t *new = (list_t *)calloc(1, sizeof(list_t));
  assert(new);
  if(!pool->tail){
    pool->head = new;
    pool->tail=new;
    new->next = NULL;
    new->args = element;

  }else{
    new->args = element;
    pool->tail->next = new;
    pool->tail = new;
  }

  return new;
}


void* remove_element(pool_t *pool, list_t *node, list_t *prev){

  void *ptr;
  if (prev == NULL && node != pool->head)
    assert(0);
  if(node == pool->head ){
    pool->head = node->next;
    if (node == pool->tail)
      pool->head=pool->tail = NULL;
    node->next = NULL;
    ptr = node->args;
  }
  else if(node == pool->tail){
    pool->tail = prev ;
    prev->next = NULL;
    ptr = node->args;
  }
  else{
    prev->next = node->next;
    node->next = NULL;
    ptr = node->args;
  }

  free(node);

  return ptr;

}

void* delete_element(pool_t *pool, int (*cmp) (void *,void *),void *args ){
  list_t *l,*l1;

  l = pool->head;
  l1 = NULL;

  for(; l!=NULL ; l=l->next){
    if(cmp(l->args,args))
      return remove_element(pool, l, l1);
    l1 = l;
  }
  return NULL;
}


list_t* search(pool_t *pool, int (*cmp) (void *,void *),void *args){
  list_t *node;

  for(node = pool->head; node!=NULL && cmp(node->args,args)==0 ; node= node->next);

  if(node)  
    return node;
  return NULL;

}


int exec_on_elem(pool_t *pool, int (*exec)(void*)){
  list_t *l;
  int ret = 0;

  for(l = pool->head ; l != NULL; l = l->next)
    ret += exec(l->args);
  
  return ret;
}

void exec_on_elem_targs(pool_t *pool, void (*exec)(void*, void*), void *args){
  list_t *l;

  for(l = pool->head ; l != NULL; l = l->next)
    exec(args,l->args);

}

void empty_pool(pool_t *pool)
{
  list_t *l;

  while(pool && pool->head!=pool->tail){
    l = pool->head;
    pool->head = pool->head->next;
    free(l);
  }

  pool->head = pool->tail = NULL;
}


void delete_list(pool_t *pool){
  list_t *l;

  while(pool && pool->head!=pool->tail){
    l = pool->head;
    pool->head = pool->head->next;
    free(l);
  }

  free(pool->head);
  return;
}





