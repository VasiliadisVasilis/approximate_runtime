#ifndef __COORDINATOR__
#define __COORDINATOR__

#include <pthread.h>
#include <time.h>
#include <ucontext.h>
#define coord 0
#define NUM_THREADS 4

typedef struct info_t{
  
  struct ucontext context;
  pthread_t my_id;
  pthread_attr_t attributes;
  
  unsigned int id;
  volatile int flag;
  
  pthread_mutex_t my_mutex;
  pthread_cond_t cond;
  
  unsigned int work;
  unsigned char sig;
  
  void *execution_args;
  void (*execution) (void *);
  
  void *sanity_args;
  int (*sanity) (void *, void*);
  
  unsigned int return_val;
  unsigned int checked_results;
  
  unsigned int redo;
  
}info;

void init_system();

#endif 