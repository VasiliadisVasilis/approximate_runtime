#include <pthread.h>
#include <stdio.h>
#include <assert.h>
#include <stdint.h>
#include <pthread.h>
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

int check_schedule(void *task, void *dont_care){
  task_t *curr = (task_t*) task;
  // check if my group is being executed. 
  if(curr->my_group->schedule){
    return 1;
  }
  return 0;
}

task_t* get_job(info *me){
  task_t *element = NULL;
  pool_t *ready;
#ifdef DOUBLE_QUEUES
  if( me->reliable == 0 )
  {
    ready = non_sig_ready_tasks;
  }
  else
  {
    ready = sig_ready_tasks;
  }
#else
  ready = ready_tasks;
#endif
  do{
    pthread_mutex_lock(&ready->lock);
    if(ready->head){
      element = delete_element(ready, check_schedule, NULL);
    }
    if(element){
      move_q(element);
      me->execution = element->execution;
      me->execution_args = element->execution_args;
      me->sanity = element ->sanity_func;
      me->sanity_args= element->sanity_args;
      me->checked_results = 0;
      me->redo = element->redo;
      /* vasiliad: gemfi uses this to pause faults when a segfault is detected */
      me->task_id = element->task_id;
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

#if defined(ENABLE_CONTEXT) == 0 && defined(ENABLE_SIGNALS) == 0
void* main_acc(void *args){
  info *whoami = (info*) args;
  task_t *exec_task;
  uint64_t am_i_faulty;

  pthread_mutex_lock(&whoami->my_mutex);
  while(1)
  { 
    exec_task=get_job(whoami);
 redo_task:
    assert(exec_task);
    whoami->execution(whoami->execution_args, exec_task->task_id, 
        TASK_SIGNIFICANCE);
    if ( MAY_FAIL )
    {
     if ( whoami->sanity )
      {
        am_i_faulty = 0;
        whoami->return_val = SANITY_SUCCESS;
        whoami->return_val = whoami->sanity(whoami->execution_args, 
            whoami->sanity_args, am_i_faulty);  
        if ( whoami->return_val != SANITY_SUCCESS  && whoami->redo > 0 )
        {
          whoami->redo--;
          whoami->exec_status = TASK_NONE;
          goto redo_task;
        }
      }
    }

    finished_task(exec_task);
    whoami->exec_status = TASK_NONE;
  }
}
#else
void* main_acc(void *args){
  info *whoami = (info*) args;
  task_t *exec_task;
  uint64_t am_i_faulty;

  pthread_mutex_lock(&whoami->my_mutex);
  while(1)
  { 
    exec_task=get_job(whoami);
    assert(exec_task);
    // if a fault is detected I am going to 
    // return to the following line
#ifdef ENABLE_CONTEXT
    if ( MAY_FAIL )
    {
      getcontext(&(whoami->context));
    }
#endif
GET_CONTEXT_LABEL
    if(whoami->exec_status == TASK_NONE)
    {
      whoami->exec_status = TASK_EXECUTING;
      whoami->execution(whoami->execution_args, exec_task->task_id, 
          TASK_SIGNIFICANCE);
      whoami->exec_status = TASK_SANITY;
#if ENABLE_CONTEXT
      if ( MAY_FAIL)
      {
        setcontext(&(whoami->context));
      }
#endif
    }
    /*vasiliad: What if a SIGSEV or w/e occurs during a trc/grc ??? */
    else if ( MAY_FAIL 
        && (   whoami->exec_status == TASK_SANITY 
            || whoami->exec_status == TASK_CRASHED 
            || whoami->exec_status == TASK_TERMINATED ) )
    {
      am_i_faulty = 0;
#ifdef RAZOR
      am_i_faulty |= gemfi_faulty();
      if ( am_i_faulty )
      {
        printf("[RTS] Fault detected %d\n", whoami->task_id);
      }
#endif      
      am_i_faulty |= ( whoami->exec_status == TASK_TERMINATED );
#ifdef ENABLE_SIGNALS
      am_i_faulty |= ( whoami->exec_status == TASK_CRASHED );
      if ( whoami->exec_status == TASK_CRASHED )
      {
        printf("[RTS] TaskCrashed %d\n", whoami->task_id);
      }
#endif
      if ( whoami->sanity )
      {
        whoami->return_val = SANITY_SUCCESS;
        whoami->return_val = whoami->sanity(whoami->execution_args, 
            whoami->sanity_args, am_i_faulty);  
        if ( whoami->return_val != SANITY_SUCCESS  && whoami->redo > 0 )
        {
          whoami->redo--;
          whoami->exec_status = TASK_NONE;
#if ENABLE_CONTEXT          
          setcontext(&(whoami->context));
#endif          
        }
      }
    }

    finished_task(exec_task);
    whoami->exec_status = TASK_NONE;
  }
}
#endif
