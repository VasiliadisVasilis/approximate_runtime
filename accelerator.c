#include <pthread.h>
#include <stdio.h>
#include <assert.h>
#include <stdint.h>
#include <pthread.h>
#define _GNU_SOURCE
#include "coordinator.h"
#include "task.h"

/* Include this *after* task.h if you wish to access task_t fields */
#include "include/runtime.h"
#include "debug.h"
#include "config.h"


extern task_t **assigned_jobs;

extern pthread_cond_t cord_condition;
extern pthread_mutex_t cord_lock;

extern pool_t *pending_tasks;
#ifdef DOUBLE_QUEUES
extern pool_t *sig_ready_tasks;
extern pool_t *non_sig_ready_tasks;
#else
extern pool_t *ready_tasks;
#endif
extern pool_t *executing_tasks;
extern pool_t *finished_tasks;


task_t* get_job(info *me){
  task_t *element = NULL;
	int ret;

	do
	{
		ret = queue_pop(me->work_queue, (void**)&element);
		if ( ret != 0 )
		{
			sched_yield();
			continue;
		}
		if ( element->significance == NON_SIGNIFICANT )
		{
			me->execution = element->execution_nonsig;
		}
		else
		{
			me->execution = element->execution;
		}
		me->execution_args = element->execution_args;
  }while(ret!=0 && me->running);

  return element;
}

/* Just execute tasks as non-significant */
void* main_acc(void *args){
  info *whoami = (info*) args;
  task_t *exec_task;
	
 while(whoami->running)
  { 
    exec_task=get_job(whoami);
		if ( exec_task == NULL )
		{
			sched_yield();
			continue;
		}
    if ( whoami->execution )
      whoami->execution(whoami->execution_args);

    finished_task(exec_task);
  }
	pthread_exit(NULL);
	return NULL;
}

