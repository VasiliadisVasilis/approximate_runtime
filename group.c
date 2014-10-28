#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <signal.h>
#include <time.h>
#include <ucontext.h>
#include <errno.h>
#include "list.h"
#include "group.h"
#include "coordinator.h"
#include "task.h"
#include "debug.h"
/* Include this *after* task.h if you wish to access task_t fields*/
#include "include/runtime.h" 

extern info *my_threads;
extern int debug_flag ;
extern unsigned int total_workers;

int whoami(){
  int i;
  pthread_t my_id = pthread_self();
  for ( i = 0 ; i < total_workers ; i++)
    if(my_id == my_threads[i].my_id )
      return i;
  return -1;
}

int cmp_group(void *args1, void *args2){
  group_t *g1 = (group_t*) args1;
  char *g2 = (char*) args2;
  if(strcmp(g1->name,g2) == 0)
    return 1;
  return 0;
}

int exec_sanity(group_t *group){
  int result = SANITY_SUCCESS;

  if ( group->sanity_func )
    result = group->sanity_func(group->sanity_func_args);

  return result;
}


group_t *create_group(char *name){
  list_t *element;
  group_t *my_group;
  int size_string;
  if(!groups)
    groups = create_pool();

  element = search(groups, cmp_group, (void*) name); 

  if(element){
    my_group = (group_t*) element->args;
    return my_group;
  }

  my_group = (group_t *) calloc(1, sizeof(group_t));
  assert(my_group);
  add_pool_head(groups,my_group);

  size_string = strlen(name) + 1;
  my_group->name = (char*) calloc(size_string, sizeof(char));
  assert(my_group->name);

  strcpy(my_group->name,name);
  my_group->name[size_string-1] = '\0';

  my_group->pending_q = create_pool();
  my_group->executing_q = create_pool();
  my_group->finished_q = create_pool();

  my_group->pending_num=0;
  my_group->executing_num = 0;
  my_group->finished_sig_num = 0;
  my_group->finished_non_sig_num = 0;

  my_group->total_sig_tasks = 0;
  my_group->total_non_sig_tasks = 0;

  my_group->locked = 0;
  my_group->terminated = 0;
  my_group->schedule = 1;
  my_group->executed = 0;
  my_group->result = SANITY_SUCCESS;
  my_group->sanity_func = NULL;

  my_group->redo = 0;
  my_group->ratio = -1.0;

  pthread_mutex_init(&my_group->lock, NULL);
  pthread_cond_init(&my_group->condition,NULL);

  return my_group;
}

float calculate_ratio(group_t *elem){
  float a = elem->finished_non_sig_num;
  float b = elem->total_non_sig_tasks;
  if( b != 0.0 )
    return a/ b;
  else return 1.1;
}



void force_termination(void *args){
  task_t *task = (task_t*) args;
  pthread_mutex_lock(&task->lock);
  pthread_t acc = task->execution_thread;
  if(my_threads[task->execution_id].flag == 1){
    fflush(stdout);
    pthread_kill(acc,SIGUSR1);
  }
  pthread_mutex_unlock(&task->lock);
}

int wait_group_ratio(group_t* group, float ratio)
{
  float temp;
  debug("Calculating my ratio : ");

  pthread_mutex_lock(&group->executing_q->lock);
  temp = calculate_ratio(group);
  debug("%f executing num %d\n", ratio,group->executing_num);
  if(temp >= ratio){
    if(group->executing_num == 0 && group->total_sig_tasks == group->finished_sig_num){
      group->schedule = 0;
      debug("I am going to execute sanity funct\n");
      pthread_mutex_unlock(&group->executing_q->lock);
      return WAIT_DONE;
    }
  }
  else{
    debug("Number of pending tasks %d\n",group->finished_sig_num);
  }
  pthread_mutex_unlock(&group->executing_q->lock);
  return WAIT_PENDING;
}

int wait_group_time(group_t *group, unsigned int time_ms)
{
  int ret;
  struct timespec watchdog ={0, 0};
  time_t secs;
  long nsecs;

  clock_gettime(CLOCK_REALTIME, &watchdog);
  secs = watchdog.tv_sec + time_ms/1000;
  nsecs = watchdog.tv_nsec + (time_ms % 1000)*100000;
  watchdog.tv_sec = secs + nsecs / 1000000000L;
  watchdog.tv_nsec = nsecs % 1000000000L;

  do 
  {
    debug("Wait on watchdog...\n");
    ret =pthread_cond_timedwait(&group->condition, &group->lock, &watchdog);
    if (ret == ETIMEDOUT) 
    {
      debug("Wait on watchdog...Done\n");
      if(group->finished_sig_num != group->total_sig_tasks){
        group->ratio = 0.0;
        debug("Wait for significant tasks...\n");
        pthread_cond_wait(&group->condition, &group->lock);
        debug("Wait for significant tasks...Done\n");
        break;
      }
      else
      {
        if(group->executing_num != 0)
        {
          if(!group->terminated){
            pthread_mutex_lock(&group->executing_q->lock);
            exec_on_elem(group->executing_q,force_termination); 
            pthread_mutex_unlock(&group->executing_q->lock);
            group->terminated = 1;
            #warning this might be wrong ...
            pthread_mutex_unlock(&group->lock);
          }
        }
        while(group->executing_num != 0){}
      }
    } 
    else if (ret == 0) 
    {
      debug("ratio of tasks achieved\n");
      break;
    }
  }while (ret == EINTR); 
  pthread_mutex_lock(&group->executing_q->lock);

  return WAIT_DONE;
}

int wait_group_all(group_t *group)
{
  debug(" I am waiting for ALL tasks\n");
  if(group->finished_sig_num != group->total_sig_tasks)
  {
    debug("Locking in conditional wait\n");
    pthread_cond_wait(&group->condition, &group->lock);
    debug("Main application just woke up\n");
  }
  else
  {
    if(group->executing_num != 0)
    {
      if(!group->terminated)
      {
        pthread_mutex_lock(&group->executing_q->lock);
        exec_on_elem(group->executing_q,force_termination); 
        pthread_mutex_unlock(&group->executing_q->lock);
        #warning this might be wrong
        group->terminated = 1;
        pthread_mutex_unlock(&group->lock);
      }
    }
    while(group->executing_num != 0){}
    pthread_mutex_lock(&group->executing_q->lock);
  }

  return WAIT_DONE;
}


int wait_group(char *group, int (*func) (void *),  void * args , unsigned int type,
    unsigned int time_ms, unsigned int time_us, float ratio, unsigned int redo)
{
  list_t *list;
  group_t *my_group;
  if(!groups){
    assert(0);
    return 0;
  }

  list = search(groups, cmp_group, (void*) group);

  if (!list){
    assert(0);
    return 0;
  }

  my_group = (group_t*) list->args; //get actual group
  pthread_mutex_lock(&my_group->lock);
  my_group->sanity_func = func;
  my_group->sanity_func_args = args;
  my_group->ratio = ratio;
  my_group->redo = redo;
  my_group->locked = 1;

group_redo:
  if (type & SYNC_RATIO)
  {
    debug("Wait for ratio...\n");
    if ( wait_group_ratio(my_group, ratio) == WAIT_DONE )
    {
      debug("Waiting for ratio...Done\n");
      goto done_exec_group;
    }
  }
  if( (type&SYNC_TIME) )
  {
    debug("Waiting for time...\n");
    wait_group_time(my_group, time_us);
    debug("Waiting for time...Done\n");
  }
  else if(type&SYNC_ALL)
  {
    debug("Waiting for all...\n");
    wait_group_all(my_group);
    debug("Waiting for all...Done\n");
  }
  else 
  {
    debug("Waiting for condition...\n");
    pthread_cond_wait(&my_group->condition, &my_group->lock);
    debug("Waiting for condition...Done\n");
  }


done_exec_group:

  debug("******* Executing sanity function for %s:%f\n", my_group->name,
  calculate_ratio(my_group));
  my_group->result = exec_sanity(my_group);
  my_group->executed++;
  if (   my_group->result != SANITY_SUCCESS 
      && my_group->executed <= my_group->redo )
  {
    debug("^^^^^^^ Need to re-execute (%d/%d)\n", my_group->executed, my_group->redo);
    my_group->pending_num = 0;
    my_group->terminated = 0;
    my_group->locked = 1;
    exec_on_elem(my_group->finished_q, actual_push);
    pthread_mutex_lock(&my_group->finished_q->lock);
    empty_pool(my_group->finished_q);
    my_group->schedule = 1;
    pthread_mutex_unlock(&my_group->finished_q->lock);
    debug("^^^^^^^ Done setting up re-execution\n");
    goto group_redo;
  }
  else
  {
    /* vasiliad: nothing to do but free the finished_q */
    exec_on_elem(my_group->finished_q, free_args);
  }

  pthread_mutex_unlock(&my_group->lock);
  return 1; 
}

void explicit_sync(group_t *curr_group){
  float ratio;
  pthread_mutex_lock(&curr_group->lock);
  // Check if I have a pending barrier.
  if(!curr_group->locked){
    pthread_mutex_unlock(&curr_group->lock);
    // If not just return.
    return ;
  }

  // check if the current group has executed all the significant tasks
  if ( curr_group->finished_sig_num == curr_group->total_sig_tasks){
    ratio = calculate_ratio(curr_group);
    if(ratio < curr_group ->ratio ){
      pthread_mutex_unlock(&curr_group->lock);
      return ;
    }
  }else{
    pthread_mutex_unlock(&curr_group->lock);
    return;
  }

  // I am here only if I need to wake up the main application from 
  // the barrier and all requested ratios are met.

  // I am stopping to scheduling tasks from this group.
  curr_group->schedule = 0;

  // I am forcing termination of tasks of this group.
  if(curr_group->executing_num != 0){
    if(!curr_group->terminated){
      pthread_mutex_lock(&curr_group->executing_q->lock);
      exec_on_elem(curr_group->executing_q,force_termination); 
      pthread_mutex_unlock(&curr_group->executing_q->lock);
      curr_group->terminated = 1;
      pthread_mutex_unlock(&curr_group->lock);
      // I am returning because at this point the terminated tasks are going to check again whether 
      // to continue execution of this group or not.
      return;
    }
  }

  curr_group ->locked = 0;
  // wake up the main application
  pthread_cond_signal(&curr_group->condition);
  //release mutex of the thread.
  pthread_mutex_unlock(&curr_group->lock);
  debug("Waking up main application\n");
  fflush(stdout);
  return;

}

