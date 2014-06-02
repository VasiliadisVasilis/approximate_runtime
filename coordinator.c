#include <stdio.h>
#include <stdlib.h>
#include "coordinator.h"
#include "list.h"
#include "task.h"
#include "group.h"


pool_t *pending_tasks;
pool_t *ready_tasks;
pool_t *executing_tasks;
pool_t *finished_tasks;
int debug_flag = 0;
info my_threads[NUM_THREADS];
task_t *assigned_jobs[NUM_THREADS];

pthread_cond_t cord_condition;
pthread_mutex_t cord_lock;

void explicit_sync(void *args);
void* main_acc(void *args);
void finished_task(task_t* task);



void action(int sig, siginfo_t* siginfo, void *context){
  int i;
  pthread_t my_id = pthread_self();
  
  for ( i = 0 ; i < NUM_THREADS ; i++){
    if(my_id == my_threads[i].my_id ){
      if(my_threads[i].flag == 1){
	setcontext(&(my_threads[i].context));
	return ;
    }
      else{
	break;
      }
    }
  }
  return ;
}


void init_acc(){
  int i;
  for( i = 1 ; i < NUM_THREADS; i++){
      assigned_jobs[i] = NULL;
      pthread_cond_init(&my_threads[i].cond,NULL);
      pthread_attr_init(&my_threads[i].attributes);
      pthread_attr_setdetachstate(&my_threads[i].attributes,PTHREAD_CREATE_DETACHED);
      my_threads[i].id = i;
      my_threads[i].sanity = NULL;
      my_threads[i].sanity_args = NULL;
      my_threads[i].execution = NULL;
      my_threads[i].execution_args = NULL;
      my_threads[i].work = 0;
      my_threads[i].checked_results = 1;
      assigned_jobs[i] = NULL;
      pthread_create(&(my_threads[i].my_id), &(my_threads[i].attributes), main_acc, &my_threads[i]);
  }
}


void check_sync(){
  exec_on_elem(groups,explicit_sync);
  return;
}

void* main_c(void *args){
  init_acc();
  pthread_cond_init(&cord_condition,NULL);
    pthread_mutex_lock(&cord_lock);
  while(1){
     check_sync();
  }
  
}

void init_system(){
  
  pending_tasks = create_pool();
  ready_tasks = create_pool();
  executing_tasks = create_pool();
  finished_tasks = create_pool();
  
  pthread_attr_init(&my_threads[coord].attributes);
  pthread_attr_setdetachstate(&my_threads[coord].attributes,PTHREAD_CREATE_DETACHED);
  struct sigaction act;
  act.sa_sigaction = action;
  act.sa_flags = SA_SIGINFO;
  if ( (sigaction(SIGILL,&act,NULL)<0)||
    (sigaction(SIGFPE,&act,NULL)<0)||
    (sigaction(SIGPIPE,&act,NULL)<0)||
    (sigaction(SIGBUS,&act,NULL)<0)||
    (sigaction(SIGUSR1,&act,NULL)<0)||
    (sigaction(SIGUSR2,&act,NULL)<0)||
//     (sigaction(SIGSEGV,&act,NULL)<0)||
    (sigaction(SIGSYS,&act,NULL)<0) ) {
      perror("Could not assign signal handlers\n");
      exit(0);
    }
    pthread_create(&(my_threads[coord].my_id), &(my_threads[coord].attributes), main_c, NULL); 
}