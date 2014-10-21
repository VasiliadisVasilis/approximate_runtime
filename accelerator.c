#include <pthread.h>
#include <stdio.h>
#include <assert.h>
#include "coordinator.h"
#include "task.h"

extern task_t **assigned_jobs;

extern pthread_cond_t cord_condition;
extern pthread_mutex_t cord_lock;

extern pool_t *pending_tasks;
extern pool_t *sig_ready_tasks;
extern pool_t *non_sig_ready_tasks;
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
  pool_t *ready_tasks ;
  if( me->reliable == 0 )
    ready_tasks = non_sig_ready_tasks;
  else
    ready_tasks = sig_ready_tasks;
  do{
    pthread_mutex_lock(&ready_tasks->lock);
    if(ready_tasks->head){
      element = delete_element(ready_tasks, check_schedule, NULL);
    }
    if(element){
#ifdef DEBUG
      printf("Found task %s%d\n",element->my_group->name,element->task_id);
#endif
      move_q(element);
      me->execution = element->execution;
      me->execution_args = element->execution_args;
      me->sanity = element ->sanity_func;
      me->sanity_args= element->sanity_args;
      me->checked_results = 0;
      me->redo = element->redo;
      element->execution_thread = me->my_id;
      element->execution_id = me->id;
    }
    pthread_mutex_unlock(&ready_tasks->lock);
  }while(element == NULL);
  return element;
}


void* main_acc(void *args){
  info *whoami = (info*) args;
  task_t *exec_task;

  pthread_mutex_lock(&whoami->my_mutex);
  while(1){
    exec_task=get_job(whoami);
    assert(exec_task);
    // if a fault is detected I am going to 
    // return to the following line
    getcontext(&(whoami->context));
    if(whoami->flag == 0){
      whoami->flag = 1;
      whoami->execution(whoami->execution_args);
      whoami->flag = 2;
    }
    whoami->return_val = 1;
    if(whoami->sanity)
      whoami->return_val = whoami->sanity(whoami->execution_args,whoami->sanity_args);  
    if ( whoami->return_val != 1  && whoami->redo >0 ){
      whoami->redo--;
      whoami->flag =0;
      setcontext(&(whoami->context));
    }
    else{
      finished_task(exec_task);
    }
    whoami->flag = 0;
  }

}
