#ifndef __GROUP_T__
#define __GROUP_T__

#include <pthread.h>
#include "list.h"

#define SYNC_TIME 1
#define SYNC_RATIO 2
#define SYNC_ALL 4

enum { WAIT_DONE, WAIT_PENDING };

pthread_mutex_t group_lock;
pool_t *groups;

typedef struct groups{
  
  char *name;
  
  int (*sanity_func) (void *);
  void* sanity_func_args;
  
  unsigned int redo;
  float ratio;
  
  pool_t *pending_q;
  volatile unsigned int pending_num;
  
  pool_t *executing_q;
  volatile unsigned int executing_num;
  
  pool_t *finished_q;
  volatile unsigned int finished_sig_num;
  volatile unsigned int finished_non_sig_num;
  
  
  
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
int wait_group(char *group, int (*func) (void *),  void * args , unsigned int type, unsigned int time_ms, unsigned int time_us, float ratio, unsigned int redo);
void stop_exec(); 
#endif
