#ifndef __COORDINATOR__
#define __COORDINATOR__

#include <pthread.h>
#include <time.h>
#include <ucontext.h>
#include "queue.h"

enum ReliabilityMode {NON_RELIABLE, RELIABLE};
enum TaskStatus {TASK_NONE, TASK_EXECUTING, TASK_SANITY, TASK_CRASHED, TASK_TERMINATED};

// info_t is the worker descriptor.

typedef struct info_t{

  // context of the running application on a safe point.
  // It is mainly used in order to implement a smart soft
  // checkpoint.
  struct ucontext context;
  
  pthread_t my_id;
  pthread_attr_t attributes;
  
  // unique runtime provided id for this worker
  unsigned int id;

  // vasiliad: currently executed task ( used for gemfi identification purposes )
  unsigned int task_id, significance;
  
  // variable used by approximate workers. 
  // It is mainly used as a safe-guard protecting the
  // execution of the task function. If exec_status=0 then 
  // the task function is not used.
  volatile int exec_status;
  
  //synchronization variables.
  pthread_cond_t cond;

  // reliability of the worker.
  unsigned char sig;
  
  
  // arguments of the task function
  void *execution_args;
  // task function.
  /* vasiliad: see task.h*/
  void (*execution_nonsig) (void *);
  void (*execution) (void *);
  
  //arguments of the tasks result check function
  void *sanity_args;
  // result check function
  int (*sanity) (void *, void*, int);
  
  // return value of result check function.
  unsigned int return_val;
  
  unsigned int checked_results;
  
  // number of re-executions of task.
  unsigned int redo;
  // Not used variable
  unsigned int reliable;
  
	unsigned int running;
	long long energy, dram_energy;
	queue_t *work_queue;
	
}info;

void init_system(unsigned int workers);
void shutdown_system();

#endif 
