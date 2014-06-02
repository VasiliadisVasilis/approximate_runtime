#include <stdlib.h>
#include <stdio.h>
#include "group.h"
#include "task.h"
#include "coordinator.h"
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
  printf("Hello %s\n",(char*) args);
  latency();
}

int my_sanity(void *args,void *args2){
   printf("I am sanity function %s --- %s\n",(char*)args,(char *) args2);
  return 1;
}

int big_sanity(void *args){
  printf("I am big sanity function %s\n",(char*) args);
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
   task_t* task[10][10];
   char name[10][10][10];
   char sanity[10][10];
   struct timespec  tv1, tv2;
#warning the init_system function must be added to the report and implement some kind of topology configuration e.g inform the number of reliable/unreliable workers.
   init_system();
   clock_gettime(CLOCK_MONOTONIC_RAW, &tv1);
   for ( i = 0 ; i < 10 ; i++){
     sprintf(sanity[i],"Mytask%d",i);
     for ( j = 0 ; j < 10 ; j++){
       sprintf(name[i][j],"task%d%d",i,j);
       task[i][j] = new_task(hello,&name[i][j][0],sizeof(char)*10, my_sanity,sanity[i],10*sizeof(char),0,0);
       push_task(task[i][j], "main_group");
    }
  }
  wait_group("main_group" , 0.8f, 0, big_sanity, sanity[1]) ; 
//    monte_carlo_pi(1000000);
   clock_gettime(CLOCK_MONOTONIC_RAW, &tv2);
   
   printf ("Total time = %10g seconds\n",
   (double) (tv2.tv_nsec - tv1.tv_nsec) / 1000000000.0 +
   (double) (tv2.tv_sec - tv1.tv_sec));
   

   return 1;
}