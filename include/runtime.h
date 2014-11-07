#ifndef __SGNF_RUNTIME__
#define __SGNF_RUNTIME__

#ifndef __TASK__
#define task_t void*
#endif

#define SYNC_TIME 1
#define SYNC_RATIO 2
#define SYNC_ALL 4

enum sanity_return_t {SANITY_SUCCESS, SANITY_FAILURE};
enum significance_value_t {NON_SIGNIFICANT, SIGNIFICANT };

task_t* new_task(void (*exec)(void *), void *args, unsigned int size_args ,int (*san)(void *, void *),
		 void *san_args, unsigned int san_size_args , unsigned char sig, unsigned int redo);
void define_in_dependencies(task_t* task, int number, ...); 
void define_out_dependencies(task_t* task, int number, ...);
void push_task(task_t* task, char *group);


int wait_group(char *group, int (*func) (void *),  void * args , unsigned int type, unsigned int time_ms, unsigned int time_us, float ratio, unsigned int redo);
void init_system(unsigned int reliable_workers , unsigned int nonrel_workers);
#endif
