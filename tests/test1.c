#include <stdlib.h>
#include <stdio.h>
#include "runtime.h"
#include <time.h>

void latency(){
  int i,j,k;
  int A[100][100],B[100][100],C[100][100];
  for ( i = 0 ;  i < 100 ; i++)
    for ( j = 0 ; j < 100 ; j++)
      for( k = 0 ; k < 100; k++)
        C[i][j] = A[i][k]*B[k][j];
}


void hello(void *args){
  int i;

  for ( i=0; i<10000; ++i)
  latency();
}

int my_sanity(void *args,void *args2){
  return 1;
}

int big_sanity(void *args){
  return 1;
}

float randomFloat()
{
  float r = (float)rand()/(float)RAND_MAX;
  return r;
}

void monte_carlo_pi(unsigned int Num_samples){
  int under_curve = 0;
  int count;
  double a,x,y;

  for (count=0; count<Num_samples; count++)
  {
    x= randomFloat();
    y= randomFloat();
    a= x*x + y*y;

    if ( a <= 1.0){
      under_curve ++;
    }
  }

  a = ((double) under_curve / Num_samples) * 4.0;  
  printf("PI is : %2.16g \n",a);
}


int main(){
  int  i,j;
  task_t* task;
  char name[10][10][10];
  char sanity[10][10];
  struct timespec  tv1, tv2;

  init_system(2,2);
  clock_gettime(CLOCK_MONOTONIC_RAW, &tv1);
  for ( i = 0 ; i < 1; i++){
    sprintf(sanity[i],"Mytask%d",i);
    for ( j = 0 ; j < 10; j++){
      task = new_task(hello,"task arg",sizeof(char)*9, my_sanity,"sanity arg",11*sizeof(char),j%2,0);
      push_task(task, "main_group");
    }
  }
  //   wait_group(char *group, int (*func) (void *),  void * args , 
  // 	     unsigned int type, unsigned int time_ms, unsigned int 
  // 	     time_us, float ratio, unsigned int redo);
  wait_group("main_group" , big_sanity, sanity[0] , SYNC_RATIO, 0 , 0 , 0.8f, 0) ; 
  //    monte_carlo_pi(1000000);
  clock_gettime(CLOCK_MONOTONIC_RAW, &tv2);

  printf ("Total time = %10g seconds\n",
      (double) (tv2.tv_nsec - tv1.tv_nsec) / 1000000000.0 +
      (double) (tv2.tv_sec - tv1.tv_sec));


  return 1;
}
