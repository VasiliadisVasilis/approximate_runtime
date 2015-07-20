#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <sched.h>
#include <pthread.h>
#include <string.h>
#include <execinfo.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <math.h>
#include <pthread.h>
#include "coordinator.h"
#include "constants.h"
#include "list.h"
#include "task.h"
#include "group.h"
#include "debug.h"
#include "config.h"
#include "verbose.h"

#define MSR_RAPL_POWER_UNIT		0x606
#define MSR_PKG_ENERGY_STATUS		0x611
#define MSR_DRAM_ENERGY_STATUS		0x619


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


#define msr_energy(fd) msr_read(fd, MSR_PKG_ENERGY_STATUS)

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

long long msr_read(int fd, int which) {

  uint64_t data;

  if ( pread(fd, &data, sizeof data, which) != sizeof data ) {
    perror("rdmsr:pread");
    pthread_exit( NULL );
  }

  return (long long)data;
}

int msr_open(int core) {

  char msr_filename[1024];
  int fd;

  sprintf(msr_filename, "/dev/cpu/%d/msr", core);
  fd = open(msr_filename, O_RDONLY);
  if ( fd < 0 ) {
    if ( errno == ENXIO ) {
      fprintf(stderr, "rdmsr: No CPU %d\n", core);
      exit(2);
    } else if ( errno == EIO ) {
      fprintf(stderr, "rdmsr: CPU %d doesn't support MSRs\n", core);
      exit(3);
    } else {
      perror("rdmsr:open");
      fprintf(stderr,"Trying to open %s\n",msr_filename);
      return -1;
    }
  }

  return fd;
}

double msr_energy_units(int fd)
{
  long long msr_reg;

	msr_reg = msr_read(fd, MSR_RAPL_POWER_UNIT);
	return  pow(0.5,(double)((msr_reg>>8)&0x1f));
}

double msr_dram_energy_units(int fd)
{
	/*vasiliad:DRAM energy not implemented*/
	return 0.0;
}

double msr_dram_energy(int fd)
{
	return msr_read(fd,MSR_DRAM_ENERGY_STATUS);
}

void init_system(unsigned int workers)
{
  /* Create the corresponing pulls to store the task descriptors */
  int i;
	cpu_set_t cpu;
	total_workers = workers;
  if(total_workers == 0){
    printf("Cannot request 0 workers\n Aborting....\n");
    exit(0);
  }
  
	CPU_ZERO(&cpu);
	CPU_SET(0, &cpu);
	i = sched_setaffinity(0, sizeof(cpu), &cpu);

	if ( i < 0 )
	{
		printf("Could not pin thread 0\n");
		exit(1);
	}
	
  pending_tasks = create_pool();

  ready_tasks = create_pool();
  //Tasks which are executed at the moment.  
  executing_tasks = create_pool();
  // Tasks finished their execution. I store them in 
  //a queue so that we can re-execute the entire group
  // if requested.
  finished_tasks = create_pool();

  my_threads = (info*) calloc(total_workers, sizeof(info));

  assigned_jobs = (task_t**) calloc(total_workers, sizeof(task_t*));
  // Initialize runtime information.
	#ifdef ENERGY_STATS
  for( i = 0 ; i < total_workers ; i++) {
		int fd = msr_open(i+1);
		my_threads[i].energy = msr_energy(fd);
		my_threads[i].dram_energy = msr_dram_energy(fd);
		close(fd);
	}
	#endif

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
		CPU_ZERO(&cpu);
		CPU_SET(i+1, &cpu);
		pthread_setaffinity_np(my_threads[i].my_id, sizeof(cpu_set_t), &cpu);
  } 

}

void shutdown_system()
{
	int i;
	double cpu_units, energy;

	energy = 0.0;

	for ( i=0; i<total_workers; ++i )
	{
		my_threads[i].running = 0;
	}

	for (i=0; i<total_workers; ++i )
	{
		pthread_join(my_threads[i].my_id, NULL);
		#ifdef ENERGY_STATS
		int fd = msr_open(i+1);
		cpu_units = msr_energy_units(fd);
		energy += cpu_units*(msr_energy(fd) - my_threads[i].energy);
		/* energy_dram += dram_units*(msr_dram_energy(fd) - my_threads[i].dram_energy); */
		close(fd);
		#endif
	}
	
	printf("Energy,%lg\n", energy/(double)(total_workers));
}
