#ifndef __TASK__
#define __TASK__


#include "list.h"
#include "group.h"

// typedef struct tasks;

typedef struct dependence{
  void *start;
  unsigned int size;
  int checked;
}d_t;


typedef struct tasks{
  
  // Unique id for each task
  unsigned int task_id;
  
  
  //ID of thread that executes the task
  pthread_t execution_thread;
  unsigned int execution_id;
  
  
  //The execution function of the task coupled with the arguments
  void *execution_args;
  /* vasiliad: extra args: task_id, significance gemfi needs this info to turn
               fi on/off INSIDE the task */
  void (*execution) (void *, unsigned int, unsigned int);
  
  //Sanity function of the task
  int (*sanity_func) (void *,void *, int);
  void* sanity_args;
  
  

  unsigned int redo;
  unsigned int executed_times;
  
  //significance of the task (currently 0 - 1)
  unsigned char significance;
  
  
  //Inputs of the task
  unsigned int num_in;
  d_t *inputs;
 
  //Outputs of the task
  unsigned int num_out;
  d_t *outputs;
  
#ifdef DEPENDENCIES  
  unsigned int dependencies;
  pool_t *depend_on;
  pool_t *dependent_tasks;
#endif  
  group_t *my_group;
  pthread_mutex_t lock;
}task_t;



//Create a task and assign the task to a group
task_t* new_task(void (*exec)(void *, unsigned int, unsigned int), void *args, unsigned int size_args ,int (*san)(void *, void *, int),
		  void *san_args, unsigned int san_size_args , unsigned char sig, unsigned int redo);

#ifdef DEPENDENCIES  
void define_in_dependencies(task_t* task, int number, ...); 
void define_out_dependencies(task_t* task, int number, ...);
void print_dependecies(task_t *task);
#endif

int free_args(void *_task);
int actual_push(void *_task);
int push_task(task_t* task, char *group);
int finished_task(task_t* task);
int move_q(task_t *task);
int del_non_signf(void *args1, void *args2);
#endif
