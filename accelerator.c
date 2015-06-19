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
  pool_t *ready;

  ready = ready_tasks;
  do{
    pthread_mutex_lock(&ready->lock);
    if(ready->head){
      element = pop_first(ready);
    }
    if(element){
      move_q(element);
			if ( element->significance == SIGNIFICANT )
			{
      	me->execution = element->execution;
			}
			else
			{
				me->execution = element->execution_nonsig;
			}
      me->execution_args = element->execution_args;
      me->significance = element->significance;
      element->execution_thread = me->my_id;
      element->execution_id = me->id;
      pthread_mutex_unlock(&ready->lock);
      break;
    }
    pthread_mutex_unlock(&ready->lock);
    pthread_yield();

  }while(element == NULL);
  return element;
}



/* Just execute tasks as non-significant */
void* main_acc(void *args){
  info *whoami = (info*) args;
  task_t *exec_task;

  while(1)
  { 
    exec_task=get_job(whoami);
    whoami->execution(whoami->execution_args);
    finished_task(exec_task);
  }
}

