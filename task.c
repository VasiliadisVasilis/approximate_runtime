#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <pthread.h>
#include <stdarg.h>
#include <string.h>
#include "group.h"
#include "include/task.h"
#include "constants.h"
#include "debug.h"

extern pool_t *pending_tasks;
extern pool_t *ready_tasks;
extern pool_t *executing_tasks;
extern pool_t *finished_tasks;
pthread_mutex_t global_lock;
int explicit_sync(void *args);

int del_non_signf(void *args1, void *args2)
{
  task_t *t = (task_t*) args1;
  if ( t->significance == NON_SIGNIFICANT )
  {
    return 1;
  }

  return 0;
}

int cmp_tasks(void *args1, void *args2){
  task_t *t1 = (task_t*) args1;
  task_t *t2 = (task_t*) args2;
  if(t1 && t2 && t1->task_id == t2->task_id)
    return 1;
  return 0;
}

// Task descriptor allocator
// I am just filling the descriptor's variables values.

task_t* new_task(void (*exec)(void *), void *args, unsigned int size_args ,void (*exec_nonsig)(void *),
		   unsigned char sig){
  static int id = 0;
  task_t *new_item = (task_t *) calloc(1,sizeof(task_t));
  assert(new_item);

  //TASK_ID is unique for each task
  new_item->task_id = id++;

  new_item->execution_id = -1;
  
  if ( size_args )
  {
    new_item->execution_args = (void*) calloc(1, size_args);
    memcpy(new_item->execution_args, args, size_args);
  }
  else
  {
    new_item->execution_args = NULL;
  }
  new_item->execution = exec;
	new_item->execution_nonsig = exec_nonsig;

  new_item->significance = sig;

  new_item->num_in = 0;
  new_item->inputs = NULL;

  new_item->num_out = 0;
  new_item->outputs = NULL;

  new_item->my_group = NULL;
  return new_item;

}

void print_id(void *args){
  task_t *me = (task_t*) args ;
  printf("%d ",me->task_id);
}

int free_args(void *_task)
{
  task_t *task = (task_t*) _task;
  if ( task->execution_args )
  {
    free(task->execution_args);
    return 1;
  }

  return 0;
}

void add_task_to_pending_queue(task_t *task, group_t *group)
{
	const int	sig = task->significance;

	if (group->pending_num_size[sig] == 0 )
	{
		group->pending_q[sig] = (task_t**) realloc(group->pending_q[sig],
			sizeof(task_t*) * (group->pending_num[sig]+PENDING_QUEUE_SIZE_STEP));
		group->pending_num_size[sig] = PENDING_QUEUE_SIZE_STEP;
	}
	group->pending_q[sig][ group->pending_num[sig] ] = task;
	(group->pending_num[sig])++;
	(group->pending_num_size[sig])--;
}

// Pushing task also at the global queues.
int push_task(task_t *task, char *name){
  group_t *group;

  //   pthread_mutex_lock(&global_lock);
  if(name == NULL){
    pthread_mutex_lock(&pending_tasks->lock);
    add_pool_head(pending_tasks,task);
    pthread_mutex_unlock(&pending_tasks->lock);
    return 1;
  }
  group = create_group(name);

  task->my_group = group;
	(group->total)++;
	add_task_to_pending_queue(task, group);
  debug("Pushing Task %s%d : %d\n",name,task->task_id, 
      task->significance );
  //  pthread_mutex_unlock(&global_lock);
  return 1;
}

//Whe tasks finishes i cannot delete, the entire group might be 
//re-executed therefore i push tasks to the groups' finished_q queue
int finished_task(task_t* task)
{
  return explicit_sync(task->my_group);
}


int move_q(task_t *task){

  if(!task)
    return 0;

  pthread_mutex_lock(&task->my_group->executing_q->lock);
  add_pool_head(task->my_group->executing_q,task);
  pthread_mutex_unlock(&task->my_group->executing_q->lock);
  
  return 1;
}

