#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <signal.h>
#include <time.h>
#include <ucontext.h>
#include <errno.h>
#include <sys/time.h>
#include "list.h"
#include "group.h"
#include "coordinator.h"
#include "task.h"
#include "debug.h"
/* Include this *after* task.h if you wish to access task_t fields*/
#include "include/runtime.h" 
#include "config.h"

#define BATCH_SIZE 10


extern info *my_threads;
extern int debug_flag ;
extern unsigned int total_workers;
extern info* my_threads;


long my_time()
{
  struct timeval tv;
  gettimeofday(&tv, NULL);
  return tv.tv_sec*1000000+tv.tv_usec;
}

int whoami(){
  unsigned int i;
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

group_t *create_group(char *name){
	int i;
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

  my_group = (group_t *) calloc(1, sizeof(group_t));
  assert(my_group);
  add_pool_head(groups,my_group);

  size_string = strlen(name) + 1;
  my_group->name = (char*) calloc(size_string, sizeof(char));
  assert(my_group->name);

  strcpy(my_group->name,name);
  my_group->name[size_string-1] = '\0';

  my_group->executing_q = create_pool();

  my_group->executing_num = 0;

  my_group->total_sig_tasks = 0;
  my_group->total_non_sig_tasks = 0;
	
	my_group->total = 0;
  my_group->ratio = -1.0;


	/*vasiliad: Create as many queues as there are levels of significance */
	for ( i=NON_SIGNIFICANT; i<=SIGNIFICANT; ++i )
	{
		my_group->pending_num[i] = 0;
		my_group->pending_num_size[i] = 0;
		my_group->pending_q[i] = NULL;
	}
	
  pthread_mutex_init(&my_group->lock, NULL);
  pthread_cond_init(&my_group->condition,NULL);

  return my_group;
}

//user called function
int wait_group(char *group, int (*func) (void *),  void * args , unsigned int type,
    unsigned int time_ms, unsigned int time_us, float ratio, unsigned int redo)
{
  list_t *list;
  group_t *my_group;
	int sig;
	unsigned int goal, current, i;

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
  pthread_mutex_lock(&my_group->lock);
  my_group->ratio = ratio;

	goal = my_group->total * ratio;
	current = 0;


	
	int consequtive = 0;
	int worker = 0;

	element_t *e;
	my_group->executing_num = 0;
	/*vasiliad: Select the actual significance values of tasks*/
 	for (sig=SIGNIFICANT; sig >= NON_SIGNIFICANT; sig--)
	{	
		for (i=0; i<my_group->pending_num[sig]; ++i)
		{
			/*vasiliad: the first @ratio tasks will be set as significant*/
			if ( my_group->pending_q[sig][i]->significance != SIGNIFICANT 
					&& my_group->pending_q[sig][i]->significance != NON_SIGNIFICANT )
				my_group->pending_q[sig][i]->significance = ( current++ < goal ) ? 
					SIGNIFICANT : NON_SIGNIFICANT;
			queue_make_element(&e, my_group->pending_q[sig][i]);
			consequtive++;
			if ( consequtive%BATCH_SIZE == 0 )
			{
				++worker;
				worker = worker % total_workers;
			}

			queue_push(my_threads[worker].work_queue, e);
			
			my_group->executing_num++;
			if ( my_group->pending_q[sig][i]->significance == SIGNIFICANT )
			{
				my_group->total_sig_tasks ++;
			}
			else
			{
				my_group->total_non_sig_tasks ++;
			}
		}
	}

 	printf("Tasks: %d\n", my_group->executing_num);
	/*vasiliad: Rendezvous with the thread workers*/
	pthread_cond_wait(&my_group->condition, &my_group->lock);

  printf("[%10s] Significant: %6d | Non-significant: %6d | Ratio: %3.2lf/%3.2lf\n",
      my_group->name, my_group->total_sig_tasks, my_group->total_non_sig_tasks,
      my_group->total_sig_tasks/(double)(my_group->total_sig_tasks+ 
      my_group->total_non_sig_tasks), ratio);
   return 1; 
}

int explicit_sync(group_t *curr_group){
  pthread_mutex_lock(&curr_group->lock);
	
	/*vasiliad: last thread wakes up the main application*/
  curr_group->executing_num--;

	if ( curr_group->executing_num == 0 )
	{
		pthread_cond_signal(&curr_group->condition);
	}

	pthread_mutex_unlock(&curr_group->lock);

  return 1;
}

