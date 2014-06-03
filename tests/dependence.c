#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <stdint.h>
#include "runtime.h"

typedef struct DATA{
  long i;
  double *term;
  double *x;
}data_t;

unsigned NUM_TERMS;

int trc_taylor(void *args, void *args1);

void sum_elements(void *args)
{
  int *data = (int*) args;
  *(data+1) = *data + 1;
}





// taylor series for (x)^(-1) for a = 0
int simple_sum()
{
  task_t *temp;
  int *terms= (int *) calloc((NUM_TERMS+1),sizeof(int));
  int i;
  terms[0] = 1;
  
  for (i=0; i<NUM_TERMS; ++(i))
  {
    temp = new_task(sum_elements,&terms[i],sizeof(int), NULL,NULL,0,1,0);
//      define_in_dependencies(temp, 1, &terms[i] , sizeof(int)); 
//      define_out_dependencies(temp, 1, &terms[i+1] , sizeof(int)); 
    push_task(temp,"sum");
  }
  wait_group("sum", NULL,  NULL, SYNC_ALL , 0, 0, 1.0,  0);
  for (i=0; i<NUM_TERMS+1; ++i)
  {
    printf("%d \n",terms[i]);
  }
  
  return 0;
}

int main(int argc, char* argv[])
{
  double *x, ret, ratio;
  unsigned  nonreliable_threads,reliable_threads;
  
  
  if (argc != 6) {
    printf("usage: %s terms reliable_threads nonreliable_threads ratio input \n", argv[0]);
    exit(1);
  }
  
  NUM_TERMS = atoi(argv[1]);
  reliable_threads = atoi(argv[2]);
  nonreliable_threads = atoi(argv[3]);
  printf("Reliable threads are %d non reliable %d\n",reliable_threads,nonreliable_threads);
  ratio = atof(argv[4]);
  init_system(reliable_threads,nonreliable_threads);
  x = malloc(sizeof(double));
  *x = atof(argv[5]);
  
  simple_sum();
  
  return 0;
}