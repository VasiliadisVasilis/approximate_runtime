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
unsigned victim;
volatile int flag = 1;
volatile uint64_t mask;

int trc_taylor(void *args, void *args1);

void taylor_series_task(void *args)
{
  data_t *data = (data_t*) args;
  double ret;
  long pos;
  double *term;
  double *x;
  
  pos = data->i;
  term  = data->term;
  x = data->x;
  
  ret = pow((*x), pos);
  *term = ret;
}

int trc_taylor(void *args, void* args1)
{
  data_t *data = (data_t*) args;
  double *term;
  term  = data->term;

  if (fabs(*term) <= 1.0)
    return 1;
  else
    return 2;
}


int grc_taylor(void *args)
{
  double *array = (double*) args;
  int size=NUM_TERMS;
  int i;
  
  for (i=0; i<size-1; ++i) {
      if(array[i] < array[i+1] || isinf(array[i]) || isnan(array[i])) {
      printf("Error in elements %d(%lf) %d(%lf)\n", i, array[i], i+1, array[i+1]);
      return 2;
    }
  }
  
  if (isinf(array[size-1]) || isnan(array[size-1]))
    return 2;
  
  printf("Success!!!\n");
  return 1;
}



// taylor series for (x)^(-1) for a = 0
double taylor_series(double *x)
{
  double *terms;
  double ret = 0;
  long i;
  task_t *temp;
  terms= (double*) malloc(sizeof(double)*NUM_TERMS);
  data_t *data = (data_t*) malloc (sizeof(data_t)*NUM_TERMS);
  
  for (i=0; i<NUM_TERMS; ++(i))
  {
    data[i].x = x;
    data[i].i = i;
    data[i].term = &terms[i];
    
    temp = new_task(taylor_series_task,&data[i],sizeof(data_t), trc_taylor,NULL,0,0,0);
    push_task(temp,"taylor");
  }
  wait_group("taylor", grc_taylor,  terms ,SYNC_RATIO , 0, 0, 0.1,  0);
  for (i=0; i<NUM_TERMS; ++i)
  {
    ret += terms[i];
  }
  
  return ret;
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
  
  ret = taylor_series(x);
  printf("Taylor Series for '%lf': %lf\n", *x, ret);
  
  return 0;
}