#ifndef __GROUP_T__
#define __GROUP_T__

#include <pthread.h>
#include "include/task.h"

#define SYNC_TIME 1
#define SYNC_RATIO 2
#define SYNC_ALL 4

enum { WAIT_DONE, WAIT_PENDING };

pthread_mutex_t group_lock;
pool_t *groups;

#define PENDING_QUEUE_SIZE_STEP 100

typedef struct tasks task_t;

typedef struct groups{
  char *name;
  
  int (*sanity_func) (void *);
  void* sanity_func_args;
  
  float ratio;
	unsigned int total;
  
  pool_t *executing_q;
  volatile unsigned int executing_num;
  
  unsigned int total_sig_tasks;
  unsigned int total_non_sig_tasks;
  
  pthread_mutex_t group_lock;
  pthread_mutex_t lock, wait_exec;
  pthread_cond_t condition;
  
	/*vasiliad: buckets of tasks, each bucket has the same significance*/
	task_t **pending_q[101];
	unsigned int pending_num[101];
	unsigned int pending_num_size[101];
}group_t;

group_t *create_group(char *name);
int wait_group(char *group, int (*func) (void *),  void * args , unsigned int type, unsigned int time_ms, unsigned int time_us, float ratio, unsigned int redo);
void stop_exec(); 
#endif
