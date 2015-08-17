#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <math.h>
#include <runtime.h>
#include <sys/mman.h>
#include <unistd.h>

typedef void (*TASK)(void*);

long my_time();

#define USE_SUPERBLOCKS 0
#define TASK_WORK   190
#define SUPER_BLOCK   2

#if TASK_WORK%SUPER_BLOCK != 0
#error Fix dem values son!
#endif

typedef struct ARG_T arg_t;

struct ARG_T
{
	long i, size;
	double *A, *A_appr, *x, *x1, *b;
};


int THREADS;

double *construct_jacobi_matrix(double *ret, int diagonally_dominant, int size);
double *construct_b(int size);
int jacobi(double *A, double *A_appr, double *x, double *x1,  double *b, double* y,
		long int size, double itol, unsigned int *iters, unsigned int *iters_approx, float ratio);
double *construct_right(double* ret, int size, double *mat, double* sol);


double *construct_solution(double *ret,  int size) 
{
	int j;

	for ( j=0; j<size; ++j ) {
		//ret[j] = fmod((double)(rand()-RAND_MAX/2.0),2);
		ret[j] = 10.0;
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
			if ( abs(j-i) < 200 )
			{
				ret[i*size+j] = (double)(rand()%(size-(abs(j-i))))/(double)size;
			}
			else
			{
				ret[i*size+j] = (double)(rand()%100)/(double)(size);
			}
		}
	}

	if ( diagonally_dominant ) {
		for ( i=0; i<size; ++i ) {
			sum = 0;
			for ( j=0; j<size; ++j )  {
				sum += fabs(ret[i*size+j]);
			}
			ret[i*size+i] = fabs(ret[i*size+i]) + sum;
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
}

#if USE_SUPERBLOCKS
void jacobi_task_approx(void *_args)
{
	arg_t *a = (arg_t*) _args;
	int i, size;
	double *A, *A_appr, *x, *x1, *b;
	int i_e, i_b;
	int j, j_e, j_b;
	int c;
	double s[TASK_WORK/SUPER_BLOCK];

	i = a->i/SUPER_BLOCK;
	size = a->size;
	A = a->A;
	A_appr = a->A_appr;
	x = a->x;
	x1 = a->x1;
	b = a->b;

	i_b = i;
	if ( size-i > TASK_WORK/SUPER_BLOCK )
		i_e = i+TASK_WORK/SUPER_BLOCK;
	else
		i_e = size/SUPER_BLOCK;
	for (c=0; c<TASK_WORK/SUPER_BLOCK; ++c)
		s[c] = 0.0;

	for (j_b=0, j_e=TASK_WORK/SUPER_BLOCK;
			j_e <= size/SUPER_BLOCK;
			j_e+=TASK_WORK/SUPER_BLOCK, j_b+=TASK_WORK/SUPER_BLOCK)
	{
		for ( j=j_b; j<j_e; ++j)
		{
			for (c=0, i=i_b; i<i_e; ++i, ++c)
			{
				if ( j!=i )
				{
					s[c] += A_appr[i*size/SUPER_BLOCK+j] * x[j*SUPER_BLOCK];
				}
			}
		}
	}

	for ( c=0, i=i_b; i<i_e/SUPER_BLOCK; ++i, ++c )
	{ 
		s[c] = ( (double) b[i*SUPER_BLOCK] - s[c] ) / (double)A[i*size+i];
		for ( j=0; j<SUPER_BLOCK; ++j )
			x1[i*SUPER_BLOCK+j] = s[c]*
				fabs(A_appr[i*size/SUPER_BLOCK+j]/A[(i*SUPER_BLOCK+j)*size+i*SUPER_BLOCK+j]);

	}

}
#else
void jacobi_task_approx(void *_args)
{
	arg_t *a = (arg_t*) _args;
	int i, size;
	double *A, *x, *x1, *b;
	long int i_e, i_b;
	long int j, j_e, j_b, j_start, j_end;
	long int c;
	double s[TASK_WORK];

	const long int num_blocks = 2;

	i = a->i;
	size = a->size;
	A = a->A;
	x = a->x;
	x1 = a->x1;
	b = a->b;


	i_b = i;
	if ( size-i > TASK_WORK )
		i_e = i+TASK_WORK;
	else
		i_e = size;
	for (c=0; c<TASK_WORK; ++c)
		s[c] = 0.0;

	j_start = i_b - num_blocks*TASK_WORK;
	j_end = i_b + num_blocks*TASK_WORK;
	

	if ( j_start < 0 )
		j_start = 0;

	if ( j_end > size )
		j_end = size;


	for (j_b=j_start, j_e=j_b+TASK_WORK;
			j_e <= j_end;
			j_e+=TASK_WORK, j_b+=TASK_WORK)
	{
		for ( j=j_b; j<j_e; j++)
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
#endif

// This is the actual benchmark kernel
int jacobi(double *A, double *A_appr, double *x, double *x1,  double *b, double* y,
		long int size, double itol, unsigned int *iters, unsigned int *iters_approx, float ratio)
{
	int iter, i, significance;
	double dif, t;
	task_t *task;
	arg_t arg;
	char group_name[100];
	double prev_diff = 10000;
	dif = itol*100+1;
	for (i=0; i<size; ++i )
	{
		x[i] = 1.0/A[i*size+i];
	}
	arg.size = size;
	arg.A = A;
	arg.A_appr = A_appr;
	arg.x = x;
	arg.x1 = x1;
	arg.b = b;

	significance = ratio<1.0? NON_SIGNIFICANT +1 : SIGNIFICANT;

	*iters_approx = 0;
	int start_i = 0;

	for ( iter = 0; (iter<(*iters)) /*&& (  dif > itol ) */; ++iter ) {
		sprintf(group_name, "jcb%d", iter);
		if ( significance != SIGNIFICANT)
		{
			start_i = rand() % (size/TASK_WORK);
			start_i *= TASK_WORK;

			for ( i=0; i<size; i+=TASK_WORK ) {
				arg.i = (i  + start_i) % size;
				task = new_task(jacobi_task, &arg, sizeof(arg), jacobi_task_approx, significance);
				push_task(task, group_name);
			}
		}
		else
		{
			for ( i=0; i<size; i+=TASK_WORK ) {
				arg.i = i;
				task = new_task(jacobi_task, &arg, sizeof(arg), jacobi_task_approx, significance);
				push_task(task, group_name);
			}
		}

		wait_group(group_name, NULL, NULL, SYNC_RATIO, 0, 0, ratio, 0);

		if ( significance != SIGNIFICANT )
		{
			(*iters_approx) ++;
			dif = 0.0;

			dif = 0.0;
			for ( i=0; i<size; i++ ) {
				t = fabs(y[i] - x1[i] )/fabs(y[i]);
				x[i] = x1[i];
				if ( t>dif ) {
					dif = t;
				}
			}


			if (  fabs(prev_diff - dif) < itol|| iter >= 0.9**iters )
			{
				significance = SIGNIFICANT;
				ratio = 1.0;
			}
			prev_diff = dif;
		}

		dif = 0.0;
		for ( i=0; i<size; i++ ) {
			t = fabs(y[i] - x1[i] )/fabs(y[i]);
			x[i] = x1[i];
			if ( t>dif ) {
				dif = t;
			}
		}
		printf( "dif=%.32lg\n",dif);
	}
	printf( "ERROR=%.32lg\n",dif);
	*iters = iter;
	return dif>itol;
}

void task_construct(arg_t *args)
{
	double *A_appr = args->A_appr, *A = args->A, m;
	int size = args->size, start=args->i;
	int i, j, r, c;

	for ( i=start/SUPER_BLOCK; i< (start+TASK_WORK)/SUPER_BLOCK; ++i )
	{
		for (j=0; j<size/SUPER_BLOCK; ++j )
		{
			m = 0; // A[ (i*SUPER_BLOCK)*size + j*SUPER_BLOCK];
			for ( r=0; r<SUPER_BLOCK; ++r )
			{
				for ( c=0; c<SUPER_BLOCK; ++c )
				{
					double t = A[(i*SUPER_BLOCK + r )*size + j*SUPER_BLOCK +c]
						/ (double)(SUPER_BLOCK*SUPER_BLOCK);
					m += t;
				}
			}

			A_appr[i*(size/SUPER_BLOCK)+j] = m;
		}
	}
}

#if 0
m += A[(i*SUPER_BLOCK + r )*size + j*SUPER_BLOCK +c] / (double)(SUPER_BLOCK*SUPER_BLOCK);
#endif

void construct_appr_mat(double *appr_mat, double *mat, int size)
{
	int i;
	arg_t arg;
	task_t *task;
	arg.A = mat;
	arg.A_appr = appr_mat;
	arg.size = size;

	for ( i=0; i<size; i+=TASK_WORK)
	{
		arg.i = i;
		task = new_task((TASK)task_construct, &arg, sizeof(arg), NULL, 100);
		push_task(task, "construct");
	}
	wait_group("construct", NULL, NULL, SYNC_RATIO, 0, 0, 1.0, 0);
}

int main(int argc, char* argv[]) {
	double *x, itol, *y, *x1;
	double *mat, *b, *appr_mat;
	long N;
	char *endptr;
	int diagonally_dominant;
	unsigned int iters, iters_approx;
	int ret, i, j;
	long _seed;
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

	printf("Seed: %ld\n", _seed);
	printf("Size: %ld\n", N);
	printf("Tol: %lg\n", itol);
	printf("Ratio: %lg\n", ratio);

	assert(N%TASK_WORK == 0);
	assert(N%SUPER_BLOCK == 0 );
	x = (double*) calloc(N,sizeof(double));
	x1 = (double*) calloc(N, sizeof(double));
	assert(x1);
	assert(x);

	if ( ratio < 1.0 )
	{
		appr_mat = calloc((N*N)/(SUPER_BLOCK*SUPER_BLOCK), sizeof(double));
		assert(appr_mat);
	}
	mat = calloc(N*N, sizeof(double));
	assert(mat);
	y = calloc(N, sizeof(double));
	b = calloc(N, sizeof(double));
	srand(_seed);
	y = construct_solution(y, N);
	mat = construct_jacobi_matrix(mat, diagonally_dominant, N);
	b = construct_right(b, N, mat, y);
	init_system(THREADS);

	#if USE_SUPERBLOCKS
	if ( ratio < 1.0 )
	{
		construct_appr_mat(appr_mat, mat, N);
	}
	#endif
	ret = jacobi(mat, appr_mat, x, x1, b, y, N, itol, &iters, &iters_approx, ratio);
	shutdown_system();
	printf("Iterations: %ld - %ld\n", iters, iters_approx);

#if 0
	for ( i=0; i<N; ++i )
	{
		for ( j=0; j<N; ++j )
		{
			printf("%10lg ", mat[i*N+j]);
		}
		printf("\n");
	}

	for ( i=0; i<N/SUPER_BLOCK; ++i )
	{
		for ( j=0; j<N/SUPER_BLOCK; ++j )
		{
			printf("%10lg ", appr_mat[i*N/SUPER_BLOCK+j]);
		}
		printf("\n");
	}

	for ( i=0; i<N; ++i )
	{
		printf("%20lg\n", x[i]);
	}
#endif

	return 0;
}

