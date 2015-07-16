#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <execinfo.h>
#include "coordinator.h"
#include "constants.h"
#include "list.h"
#include "task.h"
#include "group.h"
#include "debug.h"
#include "config.h"
#include "verbose.h"


pool_t *pending_tasks;
pool_t *ready_tasks;
pool_t *executing_tasks;
pool_t *finished_tasks;
int debug_flag = 0;
unsigned int total_workers;
info *my_threads;
task_t **assigned_jobs;

pthread_cond_t cord_condition;
pthread_mutex_t cord_lock;


#ifdef ENERGY_STATS
#include <likwid.h>
#endif



//#ifdef GEMFI
extern char __executable_start;
extern char __etext;
//#endif

int explicit_sync(void *args);
void* main_acc(void *args);
int finished_task(task_t* task);

void print_trace(int nsig)
{
  printf("print_trace: got signal %d\n", nsig);

  void *array[32];
  size_t size;
  char **strings;
  size_t cnt;


  size = backtrace(array, 32);
  strings = backtrace_symbols(array, size);
  for (cnt=0; cnt < size; ++cnt) {
    fprintf(stderr, "%s\n", strings[cnt]);
  }

}

void my_action(int sig, siginfo_t* siginfo, void *context){
  print_trace(sig);
  exit(0);
}


void* init_acc(void *args){
  main_acc(args);
  return NULL;
}


void check_sync(){
  exec_on_elem(groups,explicit_sync);
  return;
}

void init_system(unsigned int workers)
{
  /* Create the corresponing pulls to store the task descriptors */
  int i;
	total_workers = workers;
  if(total_workers == 0){
    printf("Cannot request 0 workers\n Aborting....\n");
    exit(0);
  }
  
  pending_tasks = create_pool();

  ready_tasks = create_pool();
  //Tasks which are executed at the moment.  
  executing_tasks = create_pool();
  // Tasks finished their execution. I store them in 
  //a queue so that we can re-execute the entire group
  // if requested.
  finished_tasks = create_pool();

#ifdef ENABLE_SIGNALS
  struct sigaction act;
  memset(&act, 0, sizeof(act));
  act.sa_sigaction = my_action;
  act.sa_flags = SA_SIGINFO;

  // Creating a signal handler to catch fault related signals

  if ( (sigaction(SIGILL,&act,NULL)<0)||
      (sigaction(SIGFPE,&act,NULL)<0)||
      (sigaction(SIGPIPE,&act,NULL)<0)||
      (sigaction(SIGBUS,&act,NULL)<0)||
      (sigaction(SIGUSR1,&act,NULL)<0)||
      (sigaction(SIGUSR2,&act,NULL)<0)||
      (sigaction(SIGSEGV,&act,NULL)<0)||
      (sigaction(SIGSYS,&act,NULL)<0) ) {
    perror("Could not assign signal handlers\n");
    exit(0);
  }
#endif
	
	#ifdef ENERGY_STATS
		likwid_markerInit();
	#endif

  my_threads = (info*) calloc(total_workers, sizeof(info));

  assigned_jobs = (task_t**) calloc(total_workers, sizeof(task_t*));
  // Initialize runtime information.
  for( i = 0 ; i < total_workers ; i++){
    assigned_jobs[i] = NULL;
    pthread_cond_init(&my_threads[i].cond,NULL);
    pthread_attr_init(&my_threads[i].attributes);
    pthread_attr_setdetachstate(&my_threads[i].attributes,PTHREAD_CREATE_DETACHED);
		my_threads[i].running = 1;
    my_threads[i].id = i;
    my_threads[i].sanity = NULL;
    my_threads[i].sanity_args = NULL;
    my_threads[i].execution = NULL;
    my_threads[i].execution_args = NULL;
    my_threads[i].checked_results = 1;
		queue_init(&(my_threads[i].work_queue));
    assigned_jobs[i] = NULL;
		

    pthread_create(&(my_threads[i].my_id), &(my_threads[i].attributes), init_acc, &my_threads[i]);
  } 

}

void shutdown_system()
{
	int i;

	for ( i=0; i<total_workers; ++i )
	{
		my_threads[i].running = 0;
	}

	for (i=0; i<total_workers; ++i )
	{
		pthread_join(my_threads[i].my_id, NULL);
	}
	
	#ifdef ENERGY_STATS
	likwid_markerClose();
	#endif
}
