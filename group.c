#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <signal.h>
#include <time.h>
#include <ucontext.h>

#include "list.h"
#include "group.h"
#include "coordinator.h"
#include "task.h"

extern info my_threads[NUM_THREADS];
extern int debug_flag ;
int cmp_group(void *args1, void *args2){
  group_t *g1 = (group_t*) args1;
  char *g2 = (char*) args2;
  if(strcmp(g1->name,g2) == 0)
    return 1;
  return 0;
}

group_t *create_group(char *name){
  list_t *element;
  group_t *my_group;
  int size_string;
  if(!groups)
    groups = create_pool();
  
  element = search(groups, cmp_group, (void*) name); 
  
  if(element){
    my_group = (group_t*) element -> args;
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
  my_group->ratio = 1.0;
  
  pthread_cond_init(&my_group->condition,NULL);
  
  return my_group;
}


int wait_group(char *group, float ratio, unsigned int redo, int (*func) (void *), void * args){
  list_t *list;
  group_t *my_group;
  if(!groups){
    printf("No such group\n");
    return 0;
  }
  
  list = search(groups, cmp_group, (void*) group);
  
  if (!list){
    printf("No such group\n");
    return 0;
  }
  
  my_group = (group_t*) list->args; //get actual group
  
  pthread_mutex_lock(&my_group->lock);
  my_group->sanity_func = func;
  my_group->sanity_func_args = args;
  my_group->ratio = ratio;
  my_group->redo = redo;
  my_group->locked = 1;
  pthread_cond_wait(&my_group->condition, &my_group->lock);

  pthread_mutex_unlock(&my_group->lock);
  
  return 1; 
  
}



int exec_sanity(group_t *group){
//   printf("IN HERE\n");
  volatile int flag = 0;
  volatile int result =0;
  getcontext(&(my_threads[coord].context));
  if(group->sanity_func){
    if(flag == 0){
      flag = 1;
      result = group->sanity_func(group->sanity_func_args);
    }
  }
  else 
    result = 1;
  return result;
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
  int check = 0;
  pthread_t acc = task->execution_thread;
  if(my_threads[task->execution_id].flag == 1){
     fflush(stdout);
     pthread_kill(acc,SIGUSR1);
  }
  pthread_mutex_unlock(&task->lock);

}

void explicit_sync(void *args){
  group_t *curr_group = (group_t*) args;
  
  float ratio;
  if(!curr_group->locked){
    return ;
  }
  
  if ( curr_group->finished_sig_num == curr_group->total_sig_tasks){
    ratio = calculate_ratio(curr_group);
    if(ratio < curr_group ->ratio ){
      return ;
    }
  }
  else{
    return;
  }

  curr_group->schedule = 0;

  
  if(curr_group->executing_num != 0){
    if(!curr_group->terminated){
       exec_on_elem(curr_group->executing_q,force_termination); 
       curr_group->terminated = 1;
    }
  }

  do{
  }while(curr_group->executing_num!=0);
  

  
   curr_group->result = exec_sanity(curr_group);
   curr_group->executed++;

   curr_group ->locked = 0;
    pthread_cond_signal(&curr_group->condition);


    
    return;
  
}

