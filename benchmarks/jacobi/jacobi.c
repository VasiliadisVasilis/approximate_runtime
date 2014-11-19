#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <math.h>
#include <runtime.h>

#define LINE_BUNDLE 10
#define THRESHOLD 3 	// should be properly calculated

typedef struct ARG_T arg_t;

struct ARG_T
{
  long i, size;
  double *A, *x, *x1, *b;
  long start, stop;
};


int SKIP = 4, THREADS, LIMIT;

double ratio;
double *construct_jacobi_matrix(int diagonally_dominant, int size);
double *construct_b(int size);
int jacobi(double *A, double *x, double *x1,  double *b, double* y,
    long int size, double itol, unsigned int *iters);

double *construct_right(int size, double *mat, double* sol);

static unsigned long int next = 1;

long my_time()
{
  struct timeval tv;
  gettimeofday(&tv, NULL);
  return tv.tv_sec*1000000+tv.tv_usec;
}

double *construct_solution(int size) {
  int j;
  double	*ret = (double*) malloc(sizeof(double)*size);

  assert(ret);
  for ( j=0; j<size; ++j ) {
    //ret[j] = fmod((double)(rand()-RAND_MAX/2.0),size);
    ret[j] = 1.0;
  }

  return ret;
}

double *construct_right(int size, double *A, double *sol)
{
  int i, j;
  double *ret = (double*) malloc(sizeof(double)*size);
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
double *construct_jacobi_matrix(int diagonally_dominant, int size) {
  double *ret;
  int i, j;
  double sum;
  int sure;

  sure = size/2;
  if ( sure < 5 )
    sure = 5;
  else if ( sure > 500 )
    sure = 500;

  ret = (double*) malloc(sizeof(double)*size*size);
  assert(ret);
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

void _jacobi_task(int i, int size, double *A, double *x,
    double *x1, double *b, long start, long stop)
{
  int i_e, i_b;
  int j, j_e, j_b;
  int c;
  double s[LINE_BUNDLE];

  i_b = i;
  if ( size-i > LINE_BUNDLE )
    i_e = i+LINE_BUNDLE;
  else
    i_e = size;
  for (c=0; c<LINE_BUNDLE; ++c)
    s[c] = 0.0;

  for (j_b=0, j_e=LINE_BUNDLE;
      j_e <= size;
      j_e+=LINE_BUNDLE, j_b+=LINE_BUNDLE)
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
}

void jacobi_task(void *_args)
{
  arg_t *a = (arg_t*) _args;
  _jacobi_task(a->i, a->size, a->A, a->x, a->x1, a->b, a->start, a->stop);
}

// This is the actual benchmark kernel
int jacobi(double *A, double *x, double *x1,  double *b, double* y,
    long int size, double itol, unsigned int *iters)
{
  int iter, i;
  double dif, old_diff, t;
  task_t *task;
  arg_t arg;
  char group_name[100];

  dif = itol+1;
  old_diff = dif+1;
  for (i=0; i<size; ++i )
  {
    x[i] = 1.0/A[i*size+i];
  }

  for ( iter=0; iter<LIMIT&& (  old_diff - dif > 0 ) && (dif>itol) ; ++iter ) {
    old_diff = dif;
    sprintf(group_name, "jcb%d", iter);
    for ( i=0; i<SKIP; i+=LINE_BUNDLE ) {
       arg.i = i;
       arg.size = size;
       arg.A = A;
       arg.x = x;
       arg.x1 = x1;
       arg.b = b;
       arg.start = 0;
       arg.stop = size;
//     arg.stop = SKIP+2*LINE_BUNDLE;
       
      task = new_task(jacobi_task, &arg, sizeof(arg), NULL, NULL, 0, NON_SIGNIFICANT, 0);
      push_task(task, group_name);
    }
    for ( ; i<size-SKIP; i+=LINE_BUNDLE ) {
       arg.i = i;
       arg.size = size;
       arg.A = A;
       arg.x = x;
       arg.x1 = x1;
       arg.b = b;
       arg.start = 0;
       //arg.stop = SKIP;
       arg.stop = size;
       
      task = new_task(jacobi_task, &arg, sizeof(arg), NULL, NULL, 0, SIGNIFICANT, 0);
      push_task(task, group_name);
    }
    for ( ; i<size; i+=LINE_BUNDLE ) {
      arg.i = i;
      arg.size = size;
      arg.A = A;
      arg.x = x;
      arg.x1 = x1;
      arg.b = b;
      //arg.start = (size-SKIP-2*LINE_BUNDLE);
      arg.start = 0;
      arg.stop = size;

      task = new_task(jacobi_task, &arg, sizeof(arg), NULL, NULL, 0, NON_SIGNIFICANT, 0);
      push_task(task, group_name);
    }

    wait_group(group_name, NULL, NULL, SYNC_RATIO, 0, 0, ratio, 0);

    dif = 0.0;
    #if 0
    for ( i=0; i<size; i++ ) {
      t = fabs(y[i] - x1[i] );
      x[i] = x1[i];
      dif += t;
    }
    dif/=(double)(size);
    #endif
    for ( i=0; i<size; i++ ) {
      t = fabs(y[i] - x1[i] );
      x[i] = x1[i];
      if ( t>dif ) {
        dif = t;
      }
    }
  }
  for ( ; (iter<*iters) && (  dif > itol ) ; ++iter ) {
    sprintf(group_name, "jcb%d", iter);
    for ( i=0; i<size; i+=LINE_BUNDLE ) {
      arg.i = i;
      arg.size = size;
      arg.A = A;
      arg.x = x;
      arg.x1 = x1;
      arg.b = b;
      arg.start = 0;
      arg.stop = SKIP;

      task = new_task(jacobi_task, &arg, sizeof(arg), NULL, NULL, 0, SIGNIFICANT, 0);
      push_task(task, group_name);
    }

    wait_group(group_name, NULL, NULL, SYNC_RATIO, 0, 0, 1.0f, 0);
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
  int i;
  int non_sig;
  long start, dur, _seed;
  FILE* output;

  if (argc != 11){
    printf("[%d] usage ./jacobi 'long:N' 'double:itol' "
        "'bool:diagonally_dominant' "
        "'long:max_iters long:skip long:seed string:output_file' "
        "''threads'' ''approx_limit_iters'' ''ratio''\n", argc);
    return(1);
  }

  N = strtol(argv[1], &endptr, 10);
  itol = strtod(argv[2], &endptr);
  if ( *endptr != '\0' ) {
    printf("Invalid input. Second argument (itol) must be a double"
        " represented in base 10.\n");
    return 1;
  }
  diagonally_dominant = strtol(argv[3], &endptr, 10);
  if ( *endptr != '\0' ) {
    printf("Invalid input. Third argument (diagonally_dominant) "
        "must be long represented in base 10.\n");
    return 1;
  }
  iters = strtod(argv[4], &endptr);
  if ( *endptr != '\0' ) {
    printf("Invalid input. Fourth argument (max_iters) must be a double"
        " represented in base 10.\n");
    return 1;
  }

  SKIP = strtol(argv[5],NULL, 10);
  _seed =strtol(argv[6], NULL, 10);
  output = fopen(argv[7] , "wb");
  THREADS=atoi(argv[8]);
  LIMIT = atoi(argv[9]);
  ratio = atof(argv[10]);

  assert(N%LINE_BUNDLE == 0);
  assert(SKIP>=0);
  assert(N>0);
  assert(diagonally_dominant>=0);
  x = (double*) calloc(N,sizeof(double));
  x1 = (double*) calloc(N, sizeof(double));
  assert(x1);
  assert(x);
  srand(_seed);
  y = construct_solution(N);
  assert(y);
  mat = construct_jacobi_matrix(diagonally_dominant, N);
  b = construct_right(N, mat, y);
  non_sig = THREADS/2;
  if ( non_sig == 0 )
  {
    non_sig = 1;
  }
  init_system(THREADS-non_sig, non_sig);

  start =my_time();
  ret = jacobi(mat, x, x1, b, y, N, itol, &iters);
  dur = my_time() - start;
  if ( ret == 0 ) {
    printf("Converged after %d iterations, solution:\n", iters);
  } else {
    printf("Could not converge even after %d iterations,"
        " solution:\n", iters);
  }
  printf("Threads=%d\nDuration=%g\nIterations=%u\n",
      THREADS, (double)(dur)/1000000.0, iters);
  fwrite(&N, sizeof(long), 1, output);
  fwrite(&dur, sizeof(long),  1, output);
  for ( i=0; i<N; ++i )
  {
    fwrite(x+i, sizeof(double), 1, output);
    fwrite(y+i, sizeof(double), 1, output);
  }

  fclose(output);

  free(x1);
  free(b);
  free(x);
  free(y);
  free(mat);
  return 0;
}

