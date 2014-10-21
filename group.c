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

enum { WAIT_DONE, WAIT_PENDING };

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
  //   printf("IN HERE\n");
  int id = whoami();
  int result =0;

  if (id == -1 ){
    printf("I am main application\n");
    result = group->sanity_func(group->sanity_func_args);
  }else {
    printf("**************************I am %d\n", id);
    assert(0);
  }

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

  my_group = (group_t *) malloc (sizeof(group_t));
  assert(my_group);
  add_pool_head(groups,my_group);

  size_string = strlen(name) + 1;
  my_group->name = (char*) malloc (sizeof(char) * size_string);
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
  my_group->result = 0;
  my_group->sanity_func = NULL;

  my_group->redo = 0;
  my_group->ratio = -1.0;

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
#ifdef DEBUG
  printf("Calculating my ratio : ");
#endif    
  temp = calculate_ratio(group);
#ifdef DEBUG
  printf("%f executing num %d\n", ratio,group->executing_num);
#endif
  if(temp >= ratio){
    if(group->executing_num == 0 && group->total_sig_tasks == group->finished_sig_num){
      group->schedule = 0;
#ifdef DEBUG
      printf("I am going to execute sanity funct\n");
#endif
      return WAIT_DONE;
    }
  }
#ifdef DEBUG
  else{
    printf("Number of pending tasks %d\n",group->finished_sig_num);
  }
#endif
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
    ret =pthread_cond_timedwait(&group->condition, &group->lock, &watchdog);
    if (ret == ETIMEDOUT) 
    {
      debug("Watchdog timer went off\n");
      if(group->finished_sig_num != group->total_sig_tasks){
        group->ratio = 0.0;
        debug("Shall wait for all significant tasks\n");
        pthread_cond_wait(&group->condition, &group->lock);
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
  my_group->sanity_func = func;
  my_group->sanity_func_args = args;
  my_group->ratio = ratio;
  my_group->redo = redo;
  my_group->locked = 1;

  pthread_mutex_lock(&my_group->lock);
group_redo:
  if (type & SYNC_RATIO)
  {
    if ( wait_group_ratio(my_group, ratio) == WAIT_DONE )
    {
      goto done_exec_group;
    }
  }
  if( (type&SYNC_TIME) )
  {
    wait_group_time(my_group, time_us);
  }
  else if(type&SYNC_ALL)
  {
    wait_group_all(my_group);
  }
  else 
  {
    pthread_cond_wait(&my_group->condition, &my_group->lock);
  }


done_exec_group:
  
  debug("******* Executing sanity function for %s:%f\n", my_group->name,
    calculate_ratio(my_group));
  my_group->result = exec_sanity(my_group);
  my_group->executed++;
  if ( my_group->result != SANITY_SUCCESS )
  {
    if ( my_group->executed <= my_group->redo )
    {
      debug("^^^^^^^ Need to re-execute (%d/%d)\n", my_group->executed, my_group->redo);
      my_group->pending_num = 0;
      my_group->schedule = 1;
      my_group->terminated = 0;
      my_group->locked = 1;
      exec_on_elem(my_group->finished_q, actual_push);
      debug("^^^^^^^ Done setting up re-execution\n");
      goto group_redo;
    }
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
#ifdef DEBUG
    printf("Ratio is %f\n",ratio);
#endif
    if(ratio < curr_group ->ratio ){
      pthread_mutex_unlock(&curr_group->lock);
      return ;
    }
  }else{
#ifdef DEBUG
    printf("Finished %d --- Total %d\n",curr_group->finished_sig_num,curr_group->total_sig_tasks);
#endif
    pthread_mutex_unlock(&curr_group->lock);
    return;
  }

  // I am here only if I need to wake up the main application from 
  // the barrier and all requested ratios are met.

  // I am stopping to scheduling tasks from this group.
  curr_group->schedule = 0;

  // I am forcing termination of tasks of this group.
  debug("Moving Here\n");
  if(curr_group->executing_num != 0){
    debug("Tasks are still being executed\n");
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

#if 0
  curr_group->result = exec_sanity(curr_group);
  curr_group->executed++;
#endif
  curr_group ->locked = 0;
  // wake up the main application
  printf("******* Waking up main\n");
  pthread_cond_signal(&curr_group->condition);
  //release mutex of the thread.
  pthread_mutex_unlock(&curr_group->lock);
  debug("Waking up main application\n");
  return;

}

