#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <math.h>
#include <runtime.h>
#include <sys/mman.h>
#include <unistd.h>
#ifdef GEMFI
#include <m5op.h>
#endif

long my_time();

#define TASK_WORK 200
#define THRESHOLD 3 	// should be properly calculated

typedef struct ARG_T arg_t;

struct ARG_T
{
  long i, size;
  double *A, *x, *x1, *b;
};


int THREADS;

double *construct_jacobi_matrix(double *ret, int diagonally_dominant, int size);
double *construct_b(int size);
int jacobi(double *A, double *x, double *x1,  double *b, double* y,
    long int size, double itol, unsigned int *iters, float ratio);
double *construct_right(double* ret, int size, double *mat, double* sol);

#ifdef SANITY
int jacobi_trc(void *_args, void* not_used_at_all, int faulty);
#endif




double *construct_solution(double *ret,  int size) 
{
  int j;

  for ( j=0; j<size; ++j ) {
    //ret[j] = fmod((double)(rand()-RAND_MAX/2.0),size);
    ret[j] = 1.0;
  }

  return ret;
}

double *construct_right(double *ret, int size, double *A, double *sol)
{
  int i, j;
  double s;

  for ( i=0; i<size; ++i )
  {
    s = 0;
    for ( j=0; j<size; ++j )
    {
      s += A[i*size+j] * sol[j];
    }
    ret[i] = s;
  }
  return ret;
}

// This is just a simple way to make sure that a matrix
// which consists of random numbers is diagonally dominant
// This feature is optional, set call with diagonally_dominant!=0
// in order to use it during the matrix generation.
double *construct_jacobi_matrix(double *ret, int diagonally_dominant, int size) {
  int i, j;
  double sum;
  int sure;

  sure = size/2;
  if ( sure < 5 )
    sure = 5;
  else if ( sure > 500 )
    sure = 500;

  for ( i=0; i<size; ++i ) {
    for ( j=0; j<size; ++j ) {
      if (  fabs(j-i) < sure )  
      {
        ret[i*size+j] = (double)(rand())-(double)RAND_MAX/2.0;
        ret[i*size+j] = fmod(ret[i*size+j], size);
        ret[i*size+j] /= (double)(pow(fabs(i-j)+1,4));
      }
      else if ( (rand()%100) < 10)
      {
        ret[i*size+j] = (double)(rand())-RAND_MAX/2.0;
        ret[i*size+j] = fmod(ret[i*size+j], size);
        ret[i*size+j] /= (size*size); 
      }
      else
        ret[i*size+j] = 0.0;
    }
  }

  if ( diagonally_dominant ) {
    for ( i=0; i<size; ++i ) {
      sum = 0;
      for ( j=0; j<i; ++j )  {
        sum+= fabs(ret[i*size+j]);
      }
      for ( j=i+1; j<size; ++j ) {
        sum+= fabs(ret[i*size+j]);
      }
      sum += 0.00000000000001;
      ret[i*size+i] = sum;
      if ( rand()%100 < 50 )
        ret[i*size+i] = - ret[i*size+i];
    }
  }

  return ret;
}



void jacobi_task(void *_args)
{
  arg_t *a = (arg_t*) _args;
  int i, size;
  double *A, *x, *x1, *b;
  int i_e, i_b;
  int j, j_e, j_b;
  int c;
  double s[TASK_WORK];

  i = a->i;
  size = a->size;
  A = a->A;
  x = a->x;
  x1 = a->x1;
  b = a->b;

#ifdef GEMFI
  if ( significance == NON_SIGNIFICANT )
  {
    fi_activate_inst(task_id, START);
  }
#endif

  i_b = i;
  if ( size-i > TASK_WORK )
    i_e = i+TASK_WORK;
  else
    i_e = size;
  for (c=0; c<TASK_WORK; ++c)
    s[c] = 0.0;

  for (j_b=0, j_e=TASK_WORK;
      j_e <= size;
      j_e+=TASK_WORK, j_b+=TASK_WORK)
  {
    for ( j=j_b; j<j_e; ++j)
    {
      for (c=0, i=i_b; i<i_e; ++i, ++c)
      {
        if ( j!=i )
        {
          s[c] += A[i*size+j] * x[j];
        }
      }
    }
  }

  for ( c=0, i=i_b; i<i_e; ++i, ++c )
  { 
    s[c] = ( (double) b[i] - s[c] ) / (double)A[i*size+i];
    x1[i] = s[c];
  }

#ifdef GEMFI
  if ( significance == NON_SIGNIFICANT )
  {
    fi_activate_inst(task_id, PAUSE);
  }
#endif
}

void jacobi_task_approx(void *args)
{
	jacobi_task(args);
}

// This is the actual benchmark kernel
int jacobi(double *A, double *x, double *x1,  double *b, double* y,
    long int size, double itol, unsigned int *iters, float ratio)
{
  int iter, i, significance;
  double dif, t;
  task_t *task;
  arg_t arg;
  char group_name[100];

  dif = itol*100+1;
  for (i=0; i<size; ++i )
  {
    x[i] = 1.0/A[i*size+i];
  }

  for ( iter = 0; (iter<*iters) && (  dif > itol ) ; ++iter ) {
    sprintf(group_name, "jcb%d", iter);
    significance = dif < itol*100 ? SIGNIFICANT : NON_SIGNIFICANT +1;
    for ( i=0; i<size; i+=TASK_WORK ) {
      arg.i = i;
      arg.size = size;
      arg.A = A;
      arg.x = x;
      arg.x1 = x1;
      arg.b = b;
      task = new_task(jacobi_task, &arg, sizeof(arg), jacobi_task_approx, significance);
      push_task(task, group_name);
    }

    wait_group(group_name, NULL, NULL, SYNC_RATIO, 0, 0, ratio, 0);

    dif = 0.0;
    for ( i=0; i<size; i++ ) {
      t = fabs(y[i] - x1[i] );
      x[i] = x1[i];
      if ( t>dif ) {
        dif = t;
      }
    }
  }
  printf( "ERROR= %g\n",dif);
  *iters = iter;
  return dif>itol;
}

int main(int argc, char* argv[]) {
  double *x, itol, *y, *x1;
  double *mat, *b;
  long N;
  char *endptr;
  int diagonally_dominant;
  unsigned int iters;
  int ret;
  int non_sig;
  long start, dur, _seed;
  long bytes, page;
	float ratio;


  if (argc != 7){
    printf("[%d] usage ./jacobi long:N double:itol long:max_iters long:seed threads ratio\n",
      argc);
    return(1);
  }

  N = strtol(argv[1], &endptr, 10);
  itol = strtod(argv[2], &endptr);
  diagonally_dominant = 1;
  iters = strtod(argv[3], &endptr);
  _seed =strtol(argv[4], NULL, 10);
  THREADS=atoi(argv[5]);
	ratio = atof(argv[6]);

  assert(N%TASK_WORK == 0);
  x = (double*) calloc(N,sizeof(double));
  x1 = (double*) calloc(N, sizeof(double));

  bytes = sizeof(double) * (N*N + N + N);
  page = sysconf(_SC_PAGESIZE);
  bytes = ceil(bytes/(double)page) * page;
  mat = NULL;
  ret = posix_memalign((void**)&mat, page, bytes);
  assert(mat);
  y = mat + N*N;
  b = y + N;
  assert(x1);
  assert(x);
  srand(_seed);
  y = construct_solution(y, N);
  mat = construct_jacobi_matrix(mat, diagonally_dominant, N);
  b = construct_right(b, N, mat, y);
  non_sig = THREADS/2;
  if ( non_sig == 0 )
  {
    non_sig = 1;
  }
  init_system(THREADS-non_sig, non_sig);
  start =my_time();
  ret = jacobi(mat, x, x1, b, y, N, itol, &iters, ratio);
 
  dur = my_time() - start;
  if ( ret == 0 ) {
    printf("Converged after %d iterations, solution:\n", iters);
  } else {
    printf("Could not converge even after %d iterations,"
        " solution:\n", iters);
  }
  printf("Threads=%d\nDuration=%g\nIterations=%u\n",
      THREADS, (double)(dur)/1000000.0, iters);
	
	shutdown_system();
 return 0;
}

