#ifndef __GROUP_T__
#define __GROUP_T__

#include <pthread.h>
#include "list.h"


pthread_mutex_t group_lock;
pool_t *groups;

typedef struct groups{
  
  char *name;
  
  int (*sanity_func) (void *);
  void* sanity_func_args;
  
  unsigned int redo;
  float ratio;
  
  pool_t *pending_q;
  unsigned int pending_num;
  
  pool_t *executing_q;
  unsigned int executing_num;
  
  pool_t *finished_q;
  unsigned int finished_sig_num;
  unsigned int finished_non_sig_num;
  
  
  
  unsigned int total_sig_tasks;
  unsigned int total_non_sig_tasks;
  
  pthread_mutex_t group_lock;
  pthread_mutex_t lock;
  pthread_cond_t condition;
  
  
  unsigned int locked;
  unsigned int terminated;
  unsigned int schedule;
  unsigned int executed;
  unsigned int result;
}group_t;

group_t *create_group(char *name);
int wait_group(char *group, float ratio, unsigned int redo, int (*func) (void *), void * args);
#endif