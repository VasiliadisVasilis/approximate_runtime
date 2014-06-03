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

extern info *my_threads;
extern int debug_flag ;
extern unsigned int total_workers;
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


int wait_group(char *group, int (*func) (void *),  void * args , unsigned int type, unsigned int time_ms, unsigned int time_us, float ratio, unsigned int redo){
  list_t *list;
  group_t *my_group;
  printf("going to wait group\n");
  struct timespec watchdog ={0, 0};
  time_t secs;
  long nsecs;
  int ret;
  float temp;
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
  if (type & SYNC_RATIO){
    printf("Calculating my ratio : ");
    temp = calculate_ratio(my_group);
    printf("%f executing num %d\n", ratio,my_group->executing_num);
    if(temp >= ratio){
      if(my_group->executing_num == 0 && my_group->total_sig_tasks == my_group->finished_sig_num){
	my_group->schedule = 0;
	printf("I am going to execute sanity funct\n");
	my_group->result = exec_sanity(my_group);
	my_group->executed++;
	pthread_mutex_unlock(&my_group->lock);
	return 1;
      }
    }else{
      printf("Number of pending tasks %d\n",my_group->finished_sig_num);
    }
  }

  if( (type&SYNC_TIME) ){
      clock_gettime(CLOCK_REALTIME, &watchdog);
      secs = watchdog.tv_sec + time_ms/1000;
      nsecs = watchdog.tv_nsec + (time_ms % 1000)*100000;
      watchdog.tv_sec = secs + nsecs / 1000000000L;
      watchdog.tv_nsec = nsecs % 1000000000L;
  }
  

  my_group->sanity_func = func;
  my_group->sanity_func_args = args;
  my_group->ratio = ratio;
  my_group->redo = redo;
  my_group->locked = 1;
  if((type & SYNC_TIME)){
    do {
      ret =pthread_cond_timedwait(&my_group->condition, &my_group->lock, &watchdog);
      if (ret == ETIMEDOUT) {
	printf("Watchdog timer went off\n");
	if(my_group->finished_sig_num != my_group->total_sig_tasks){
	  my_group->ratio = 0.0;
	  printf("Shall wait for all significant tasks\n");
	  pthread_cond_wait(&my_group->condition, &my_group->lock);
	  break;
	}
	else{
	  if(my_group->executing_num != 0){
	    if(!my_group->terminated){
	      pthread_mutex_lock(&my_group->executing_q->lock);
	      exec_on_elem(my_group->executing_q,force_termination); 
	      pthread_mutex_unlock(&my_group->executing_q->lock);
	      my_group->terminated = 1;
	      pthread_mutex_unlock(&my_group->lock);
	    }
	  }
	  while(my_group->executing_num != 0){}
	  my_group->result = exec_sanity(my_group);
	  my_group->executed++;
	}
      } else if (ret == 0) {
	printf("ratio of tasks achieved\n");
      }
    }while (ret == EINTR); 
  }
  else if(type&SYNC_ALL){
    if(my_group->finished_sig_num != my_group->total_sig_tasks)
      pthread_cond_wait(&my_group->condition, &my_group->lock);
    else{
      if(my_group->executing_num != 0){
	if(!my_group->terminated){
	  pthread_mutex_lock(&my_group->executing_q->lock);
	  exec_on_elem(my_group->executing_q,force_termination); 
	  pthread_mutex_unlock(&my_group->executing_q->lock);
	  my_group->terminated = 1;
	  pthread_mutex_unlock(&my_group->lock);
	}
      }
      while(my_group->executing_num != 0){}
      my_group->result = exec_sanity(my_group);
      my_group->executed++;
    }
  }
  else 
    pthread_cond_wait(&my_group->condition, &my_group->lock);
  pthread_mutex_unlock(&my_group->lock);
  
  return 1; 
  
}

int whoami(){
  int i;
  pthread_t my_id = pthread_self();
  for ( i = 0 ; i < total_workers ; i++)
    if(my_id == my_threads[i].my_id )
      return i;
    return -1;
}

int exec_sanity(group_t *group){
//   printf("IN HERE\n");
  int id = whoami();
  volatile int flag = 0;
  volatile int result =0;
  if (id == -1 ){
    printf("I am main application\n");
  }else
    getcontext(&(my_threads[id].context));
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




void explicit_sync(group_t *curr_group){
  float ratio;
  pthread_mutex_lock(&curr_group->lock);
  if(!curr_group->locked){
    pthread_mutex_unlock(&curr_group->lock);
    return ;
  }
 
  
  if ( curr_group->finished_sig_num == curr_group->total_sig_tasks){
    ratio = calculate_ratio(curr_group);
    printf("Ratio is %f\n",ratio);
    if(ratio < curr_group ->ratio ){
      pthread_mutex_unlock(&curr_group->lock);
      return ;
    }
  }else{
    pthread_mutex_unlock(&curr_group->lock);
    return;
  }

  curr_group->schedule = 0;

  
  if(curr_group->executing_num != 0){
    if(!curr_group->terminated){
       pthread_mutex_lock(&curr_group->executing_q->lock);
       exec_on_elem(curr_group->executing_q,force_termination); 
       pthread_mutex_unlock(&curr_group->executing_q->lock);
       curr_group->terminated = 1;
       pthread_mutex_unlock(&curr_group->lock);
       return;
    }
  }

  if (curr_group->executing_num !=0){
    pthread_mutex_unlock(&curr_group->lock);
    return;
  }
  curr_group->result = exec_sanity(curr_group);
  curr_group->executed++;

  curr_group ->locked = 0;
  pthread_cond_signal(&curr_group->condition);
  pthread_mutex_unlock(&curr_group->lock);
  return;
  
}

