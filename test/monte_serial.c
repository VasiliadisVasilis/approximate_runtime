#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <time.h>

float randomFloat()
{
  float r = (float)rand()/(float)RAND_MAX;
  return r;
}

double monte_carlo_pi(unsigned int Num_samples){
  int under_curve = 0;
  int count;
  double a,x,y;
  
  for (count=0; count<Num_samples; count++)
  {
    x= 1;//randomFloat();
    y= 1;//randomFloat();
    a= x*x + y*y;
    
    if ( a <= 1.0){
      under_curve ++;
    }
  }
  
  a = ((double) under_curve / Num_samples) * 4.0;  
  return a;
}

int main(){
  int  i,j;
  struct timespec  tv1, tv2;
  double a;
  unsigned int Num_samples = 1000000000;
  clock_gettime(CLOCK_MONOTONIC_RAW, &tv1);
  a = monte_carlo_pi(Num_samples);
  clock_gettime(CLOCK_MONOTONIC_RAW, &tv2);
  
  printf ("Total time = %10g seconds\n",
	  (double) (tv2.tv_nsec - tv1.tv_nsec) / 1000000000.0 +
	  (double) (tv2.tv_sec - tv1.tv_sec));
  
  printf("PI is : %2.16g \n",a);
  return 1;
}