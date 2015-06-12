#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <pthread.h>
#include <stdarg.h>
#include <string.h>
#include "task.h"
#include "debug.h"
#include "include/runtime.h"

extern pool_t *pending_tasks;
#ifdef DOUBLE_QUEUES
extern pool_t *sig_ready_tasks;
extern pool_t *non_sig_ready_tasks;
#else
extern pool_t *ready_tasks;
#endif
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
  task_t *t1 = args1;
  task_t *t2 = args2;
  if(t1 && t2 && t1->task_id == t2->task_id)
    return 1;
  return 0;
}

// Task descriptor allocator
// I am just filling the descriptor's variables values.

task_t* new_task(void  (*exec)(void *, unsigned int, unsigned int), void *args, 
    unsigned int size_args ,int (*san)(void *, void *, int),
    void *san_args, unsigned int san_size_args , unsigned char sig, unsigned int redo){
  static int id = 0;
  task_t *new = (task_t *) calloc(1,sizeof(task_t));
  assert(new);

  //TASK_ID is unique for each task
  new->task_id = id++;

  new->execution_id = -1;
  
  if ( size_args )
  {
    new->execution_args = (void*) calloc(1, size_args);
    memcpy(new->execution_args, args, size_args);
  }
  else
  {
    new->execution_args = NULL;
  }
  new->execution = exec;

  new->sanity_func = san;
  if ( san_size_args )
  {
    new->sanity_args = (void*) calloc(1, san_size_args);
    memcpy(new->sanity_args, san_args, san_size_args);
  }
  else
  {
    new->sanity_args = NULL;
  }

  new->redo = redo;
  new->executed_times = 0;

  new->significance = sig;

  new->num_in = 0;
  new->inputs = NULL;

  new->num_out = 0;
  new->outputs = NULL;

  new->my_group = NULL;
//NEVER DEBUGGED THE DEPENDENCIES PART
#ifdef DEPENDENCIES 
#warning compilin with dependencies turned on
  new->depend_on = create_pool(); // this can be commented out
  new->dependent_tasks = create_pool(); // this can be commented out
  new->dependencies = 0;
#endif
  return new;

}

void print_id(void *args){
  task_t *me = (task_t*) args ;
  printf("%d ",me->task_id);
}

#ifdef DEPENDENCIES 
void define_in_dependencies(task_t* task, int number, ...){
  va_list args;
  int i;

  if(number == 0)
    return;

  task->inputs = (d_t*) calloc(number, sizeof(d_t));
  assert(task->inputs);
  task->num_in = number;

  va_start(args,number);

  for(i = 0 ; i < number ; i++){
    task->inputs[i].start  = va_arg(args,void *);
  }

  for(i = 0 ; i < number ; i++){
    task->inputs[i].size  = va_arg(args,int);
  }

  //   printf("Define Input dependencies of task %d\n",task->task_id);
  //   for(i = 0 ; i < number ; i++)
  //     printf("Addr : %p , size %d\n",task->inputs[i].start,task->inputs[i].size );
  //   printf("Define Input dependencies of task %d\n",task->task_id);
}


void define_out_dependencies(task_t* task, int number, ...){
  va_list args;
  int i;

  if(number == 0)
    return;

  task->outputs = (d_t*) calloc(number, sizeof(d_t));
  assert(task->outputs);
  task->num_out = number;

  va_start(args,number);

  for(i = 0 ; i < number ; i++){
    task->outputs[i].start  = va_arg(args,void *);
  }

  for(i = 0 ; i < number ; i++){
    task->outputs[i].size  = va_arg(args,int);
  }

  //   printf("Define Ouput dependencies of task %d\n",task->task_id);
  //   for(i = 0 ; i < number ; i++)
  //     printf("Addr : %p , size %u\n",task->outputs[i].start,task->outputs[i].size );
  //   printf("~Define Ouput dependencies of task %d~\n",task->task_id);

}



void dependent(void *m1, void *m2){
  task_t *task1 = (task_t *) m1; 
  task_t *task2 = (task_t *) m2;

  unsigned long start1 ,end1 , start2,end2;
  //    printf("task id1 %d, task id2 %d\n",task1->task_id,task2->task_id);
  int i,j;
  for( i = 0 ; i < task1->num_in; i++){
    start1 = (unsigned long )  task1->inputs[i].start;
    end1 = start1 +task1->inputs[i].size;
    for( j = 0 ; j < task2->num_out ; j++){
      start2 = (unsigned long )  task2->outputs[j].start;
      end2 = start2 +task2->outputs[j].size;
      //       printf("%ld --- %ld\n",start1,end1);
      //       printf("%ld --- %ld\n",start2,end2);
      if(start1 <= start2 && end1 > start2 ){
        add_pool_head(task1->depend_on,task2);
        add_pool_head(task2->dependent_tasks,task1);
        task1->dependencies++;
        return;
      }
      else if(start1 > start2 && start1 < end2 ){
        add_pool_head(task1->depend_on,task2);
        add_pool_head(task2->dependent_tasks,task1);
        task1->dependencies++;
        return;
      }
    }
  }


  for( i = 0 ; i < task1->num_out; i++){
    start1 =(unsigned long )  task1->outputs[i].start;
    end1 = start1 +task1->outputs[i].size;
    for( j = 0 ; j < task2->num_in ; j++){
      start2 =(unsigned long )  task2->inputs[j].start;
      end2 = start2 +task2->inputs[j].size;
      if(start1<= start2 && end1 > start2 ){
        add_pool_head(task1->depend_on,task2);
        add_pool_head(task2->dependent_tasks,task1);
        task1->dependencies++;
        return;
      }
      else if(start1 > start2 && start1 < end2 ){
        add_pool_head(task1->depend_on,task2);
        add_pool_head(task2->dependent_tasks,task1);
        task1->dependencies++;
        return;
      }
    }
  }



  for( i = 0 ; i < task1->num_out; i++){
    start1 = (unsigned long )  task1->outputs[i].start;
    end1 = start1 +task1->outputs[i].size;
    for( j = 0 ; j < task2->num_out ; j++){
      start2 = (unsigned long )  task2->outputs[j].start;
      end2 = start2 +task2->outputs[j].size;
      if(start1<= start2 && end1 > start2 ){
        add_pool_head(task1->depend_on,task2);
        add_pool_head(task2->dependent_tasks,task1);
        task1->dependencies++;
        return;
      }
      else if(start1 > start2 && start1 < end2 ){
        add_pool_head(task1->depend_on,task2);
        add_pool_head(task2->dependent_tasks,task1);
        task1->dependencies++;
        return;
      }
    }
  }
  return;

}


void print_dependecies(task_t *task){
  printf("%d ->( ",task->task_id);
  exec_on_elem(task->depend_on,print_id);
  printf(")\n");
}

void remove_dependency(void *removed, void *dependent){
  task_t *rmv = (task_t *) removed;
  task_t *dpnt = (task_t *) dependent;
  delete_element(dpnt->depend_on,cmp_tasks,rmv);
  dpnt->dependencies--;
  if(dpnt->dependencies == 0){
    delete_element(pending_tasks,cmp_tasks, dpnt);
  #ifdef DOUBLE_QUEUES
    if(dpnt->significance == 0)
      add_pool_tail(non_sig_ready_tasks,dpnt);
    else
      add_pool_tail(sig_ready_tasks,dpnt);
   #else
    add_pool_tail(ready_tasks, dpnt);
   #endif
  }
  return ;
}


#endif

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

int actual_push(void *_task)
{
  task_t *task = (task_t*) _task;
  group_t *group = task->my_group;
  pool_t *temp;

  if ( task->significance == SIGNIFICANT )
    return 0;

  task->executed_times = 0;
#ifdef DEPENDENCIES 
  exec_on_elem_targs(pending_tasks,dependent,task);
#ifdef DOUBLE_QUEUES
  exec_on_elem_targs(sig_ready_tasks,dependent,task);
  exec_on_elem_targs(non_sig_ready_tasks,dependent,task);
#else
  exec_on_elem_targs(ready_tasks,dependent,task);
#endif
#endif

#ifdef DOUBLE_QUEUES
  if ( task->significance == SIGNIFICANT )
  {
    group->total_sig_tasks++;
    temp = sig_ready_tasks;
  }
  else
  {
    group->total_non_sig_tasks++;
    temp = non_sig_ready_tasks;
  }
#else
  if ( task->significance == SIGNIFICANT)
  {
    group->total_sig_tasks++;
  }
  else
  {
    group->total_non_sig_tasks++;
  }
  temp = ready_tasks;
#endif
  group->pending_num++;

#ifdef DEPENDENCIES 
  if(task->dependencies == 0){
    pthread_mutex_lock(&temp->lock);
    add_pool_tail(temp,task);
    pthread_mutex_unlock(&temp->lock);
  }
  else{
    pthread_mutex_lock(&pending_tasks->lock);
    add_pool_head(pending_tasks,task); // I am pushing it into the pending q. 
    pthread_mutex_unlock(&pending_tasks->lock);
  }
#else
  pthread_mutex_lock(&temp->lock);
  if ( task->significance == NON_SIGNIFICANT )
  {
    add_pool_tail(temp,task);
  }
  else
  {
    add_pool_head(temp,task);
  }
  pthread_mutex_unlock(&temp->lock);
#endif

  pthread_mutex_lock(&group->pending_q->lock);
  add_pool_tail(group->pending_q,task);
  pthread_mutex_unlock(&group->pending_q->lock);
  debug("Pushing Task %s%d : %s\n",group->name,task->task_id, 
      task->significance == SIGNIFICANT ? "sig": "non_sig" );

  return 1;
}


// pushing task into the correspoing group queues. 
// Pushing task also at the global queues.
int push_task(task_t *task, char *name){
  group_t *group;
  pool_t *temp;

  //   pthread_mutex_lock(&global_lock);
  // Here I am pushing the task into the correct q
#ifdef DEPENDENCIES 
  exec_on_elem_targs(pending_tasks,dependent,task);
  exec_on_elem_targs(sig_ready_tasks,dependent,task);
  exec_on_elem_targs(non_sig_ready_tasks,dependent,task);
#endif

  if(name == NULL){
    pthread_mutex_lock(&pending_tasks->lock);
    add_pool_head(pending_tasks,task);
    pthread_mutex_unlock(&pending_tasks->lock);
    return 1;
  }
  group = create_group(name);
  task->my_group = group;
  if(task->significance == SIGNIFICANT)
    group->total_sig_tasks++;
  if(task->significance == NON_SIGNIFICANT)
    group->total_non_sig_tasks++;

  group->pending_num++;
#ifdef DOUBLE_QUEUES
  if(task->significance == NON_SIGNIFICANT)
  {
    temp= non_sig_ready_tasks;
  }
  else
  {
    temp= sig_ready_tasks;
  }
#else
  temp = ready_tasks;
#endif

#ifdef DEPENDENCIES 
  if(task->dependencies == 0){
    pthread_mutex_lock(&temp->lock);
    add_pool_tail(temp,task);
    pthread_mutex_unlock(&temp->lock);
  }
  else{
    pthread_mutex_lock(&pending_tasks->lock);
    add_pool_head(pending_tasks,task); // I am pushing it into the pending q. 
    pthread_mutex_unlock(&pending_tasks->lock);
  }
#else
  pthread_mutex_lock(&temp->lock);
  if ( task->significance == NON_SIGNIFICANT )
  {
    add_pool_tail(temp,task);
  }
  else
  {
    add_pool_head(temp,task);
  }
  pthread_mutex_unlock(&temp->lock);
#endif


  pthread_mutex_lock(&group->pending_q->lock);
  add_pool_tail(group->pending_q,task);
  pthread_mutex_unlock(&group->pending_q->lock);
  debug("Pushing Task %s%d : %s\n",name,task->task_id, 
      task->significance == SIGNIFICANT ? "sig": "non_sig" );
  //  pthread_mutex_unlock(&global_lock);
  return 1;
}

//Whe tasks finishes i cannot delete, the entire group might be 
//re-executed therefore i push tasks to the groups' finished_q queue
int finished_task(task_t* task)
{
  task_t *elem;

  pthread_mutex_lock(&task->my_group->executing_q->lock);
  pthread_mutex_lock(&task->my_group->finished_q->lock);
  elem = delete_element(task->my_group->executing_q,cmp_tasks,task);
  if(!elem)
    printf("Something went wrong\n");
  // Here I am pushing tasks in the head. 
  // When re-executing the entire group 
  // I will need to move the elemets from the tail 
  // of the finished_q to the head of the pending_q.
  add_pool_head(task->my_group->finished_q, task); 

  task->my_group->executing_num--;
  if(task->significance == 0)
    task->my_group->finished_non_sig_num++;
  else
    task->my_group->finished_sig_num++;

  // Lets move pending_tasks to the ready q 
#ifdef DEPENDENCIES
  exec_on_elem_targs(task->dependent_tasks,remove_dependency,task); 
#endif
  pthread_mutex_unlock(&task->my_group->executing_q->lock);
  pthread_mutex_unlock(&task->my_group->finished_q->lock);

  return explicit_sync(task->my_group);
}


int move_q(task_t *task){

  if(!task)
    return 0;

  pthread_mutex_lock(&task->my_group->pending_q->lock);
  delete_element(task->my_group->pending_q,cmp_tasks,task);
  pthread_mutex_unlock(&task->my_group->pending_q->lock);

  pthread_mutex_lock(&task->my_group->executing_q->lock);
  add_pool_head(task->my_group->executing_q,task);
  task->my_group->executing_num++;
  pthread_mutex_unlock(&task->my_group->executing_q->lock);
  
  return 1;
}

