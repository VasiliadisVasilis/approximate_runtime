#include <assert.h>
#include <stdio.h>
#include <sys/time.h>
#include <math.h>
#include <stdlib.h>
#include <stdint.h>
#include <runtime.h>
#include <sys/mman.h>
#include <unistd.h>
#include <string.h>

#ifdef GEMFI
#include <m5op.h>
#endif
#define N 512
int WIDTH, HEIGHT;
double RATIO;

typedef struct DCT_TASK_ARGS_T dct_task_args_t;
typedef struct IDCT_TASK_ARGS_T idct_task_args_t;

#define MAX_DCT_COEFF 7000
#define MAX_DCT_COEFF_SIG 65392

struct DCT_TASK_ARGS_T
{
  long r, c, i, j;
};

struct IDCT_TASK_ARGS_T
{
  long r, c;
};

#ifdef SANITY
int dct_trc(void *args, void *not_used_at_all, int faulty);
int _dct_trc(long _r, long _c, long _i, long _j, int faulty);
#endif

void dct_task(void* args, unsigned int task_id, unsigned int significance);
void idct_task(void* args, unsigned int task_id, unsigned int significance);
void _idct_task(long _r, long _c);

int *quant_table;



int _quant_table[8][8] = {
  {16, 11, 10, 16, 24, 40, 51, 61 },
  {12, 12, 14, 19, 26, 58, 60, 55 } ,
  {14, 13, 16, 24, 40, 57, 69, 56 },
  {14, 17, 22, 29, 51, 87, 80, 82 },
  {18, 22, 37, 56, 68, 109, 103, 77 },
  {24, 35, 55, 64, 81, 104, 113, 92 },
  {49, 64, 78, 87, 103, 121, 120, 101},
  {72, 92, 95, 98, 112, 100, 103, 99}
};

int my_round(double a) {
  int s = a >= 0.0 ? 1 : -1;
  return (int)(a+s*0.5);
}

void quantization_task(double dct[], int table, int r, int c, int i, int j)
{
  dct[(r * 8 + i)*WIDTH*8 + c * 8 + j] = my_round(dct[(r * 8 + i)*8*WIDTH+c * 8 + j] / table);
  dct[(r * 8 + i)*WIDTH*8 + c * 8 + j] = dct[(r * 8 + i)*8*WIDTH+c * 8 + j] * table;
}


double *COS, *C,  *idct;

#ifdef GEMFI
double dct[512*512];
#else
double *dct;
#endif

unsigned char *pic;

long this_time()
{
  struct timeval tv;
  gettimeofday(&tv, NULL);
  return tv.tv_sec*1000000+tv.tv_usec;
}

/* Calculation of the dct/idct coefficients */ 
void init_coef() {
  int i, j;
  for (i = 0; i < 8; i++) {
    for (j = 0; j < 8; j++)
      COS[i*8+j] = cos((2 * i + 1) * j * acos(-1) / 16.0);
    if (i) C[i] = 1;
    else C[i] = 1 / sqrt(2);
  }

  /* 8x8 dct coefficients */
#if 0
  for ( i=0; i<8; ++i )
  {
    for (j=0; j<8; ++j)
      printf("%1.4lf ", COS[i*8+j]);
    printf("\n");
  }
#endif
}

#ifdef SANITY
int dct_trc(void *_args, void* not_used_at_all, int faulty)
{
  dct_task_args_t *args = (dct_task_args_t*) _args;
  return _dct_trc(args->r, args->c, args->i, args->j, faulty);
}

int _dct_trc(long _r, long _c, long _i, long _j, int faulty)
{
  long i, j, r, c;
  long c_e;

  r = _r;
  c = _c;
  if ( WIDTH-_c > STEP_C )
    c_e = _c+STEP_C;
  else
    c_e = WIDTH; 

  if ( faulty )
  {
    printf("[RTS] TRC %ld %ld %ld %ld %d\n", _r, _c, _i, _j, faulty);
    for ( c = _c; c<c_e; ++c )
    {
      for ( i=_i; i<_i+4; ++i )
        for ( j=_j; j<_j+2; ++j)
        {
          dct[(r * 8 + i)*8*WIDTH + c * 8 + j] = 0;
        }
    }
  }
#ifndef RAZOR
  else
  {
    for ( c = _c; c<c_e; ++c )
    {
      for ( i=_i; i<_i+4; ++i )
        for ( j=_j; j<_j+2; ++j)
        {
          if (dct[(r * 8 + i)*8*WIDTH + c * 8 + j]*quant_table[i*8+j] > MAX_DCT_COEFF )
          {
            printf("[RTS] TRC (NoSigTask) %ld %ld %ld %ld %d, faulty Value: %f\n", _r, _c, _i, _j, faulty, dct[(r * 8 + i)*8*WIDTH + c * 8 + j]*quant_table[i*8+j]);
            dct[(r * 8 + i)*8*WIDTH + c * 8 + j] = 0;
          }
        }
    }
  }
#endif
  return SANITY_SUCCESS;
}
#endif

void dct_task(void *_args, unsigned int task_id, unsigned int significance)
{
  dct_task_args_t *args = (dct_task_args_t*) _args;
  long _r = args->r;
  long _c = args->c;
  long _i = args->i;
  long _j = args->j;
  long x, y, i, j, r, c;
  long c_e;
  double sum = 0;

#ifdef GEMFI
  if ( significance == NON_SIGNIFICANT )
  {
    fi_activate_inst(task_id, START);
  }
#endif

#if 0
  long start;
  if ( significance == NON_SIGNIFICANT )
  {
    start = my_time();

    while ( my_time() - start < 10000 * 1 );
  }
  else
  {
    start = my_time();

    while ( my_time() - start < 100000 * 1 );
    printf("sig\n");
  }
#endif

  r = _r;
  c = _c;
  if ( WIDTH-_c > STEP_C )
    c_e = _c+STEP_C;
  else
    c_e = WIDTH; 

  for ( c = _c; c<c_e; ++c )
  {
    for ( i=_i; i<_i+4; ++i )
      for ( j=_j; j<_j+2; ++j)
      {
        sum = 0;
        for (x = 0; x < 8; x++)
          for (y = 0; y < 8; y++)
          {
            sum += ( (double)pic[(r * 8 + x)*8*WIDTH + c * 8 + y] - 128) * COS[x*8+i] * COS[y*8+j];
          }
        sum *= C[i] * C[j] * 0.25;
        dct[(r * 8 + i)*8*WIDTH + c * 8 + j] = sum;
        quantization_task(dct, quant_table[i*8+j], r, c, i, j);
      }
  }

#ifdef GEMFI
  if ( significance == NON_SIGNIFICANT )
  {
    fi_activate_inst(task_id, PAUSE);
  }
#endif
}

void spawn_dct_task(long r, long c, long i, long j, uint8_t significance)
{
  task_t *task;
  dct_task_args_t args;

  args.r = r;
  args.c = c;
  args.i = i;
  args.j = j;

#ifdef SANITY
  task = new_task(dct_task, &args, sizeof(args), dct_trc, NULL, 0, significance, 0);
#else
  task = new_task(dct_task, &args, sizeof(args), NULL, NULL, 0, significance, 0);
#endif
  push_task(task, "dct");
}

/*
   Significance, higher is ... more significant
   100  90  80  70
   90  85  75  60
   80  75  60  50
   70  60  50  40
 */

#ifdef NON_SIGNIFICANT_TASKS
int _dct_trc_sig(long _r, long _c, long _i, long _j, int faulty)
{
  long i, j, r, c;
  long c_e;

  r = _r;
  c = _c;
  if ( WIDTH-_c > STEP_C )
    c_e = _c+STEP_C;
  else
    c_e = WIDTH; 

  if ( faulty )
  {
    printf("[RTS] TRC %d %d %d %d %d\n", _r, _c, _i, _j, faulty);
    for ( c = _c; c<c_e; ++c )
    {
      for ( i=_i; i<_i+4; ++i )
        for ( j=_j; j<_j+2; ++j)
        {
          dct[(r * 8 + i)*8*WIDTH + c * 8 + j] = 0;
        }
    }
  }
#ifndef RAZOR
  else
  {
    for ( c = _c; c<c_e; ++c )
    {
      for ( i=_i; i<_i+4; ++i )
        for ( j=_j; j<_j+2; ++j)
        {
          if (dct[(r * 8 + i)*8*WIDTH + c * 8 + j]*quant_table[i*8+j] > MAX_DCT_COEFF_SIG )
          {
            printf("[RTS] TRC (SigTask) %d %d %d %d %d, faulty Value: %f\n", _r, _c, _i, _j, faulty, dct[(r * 8 + i)*8*WIDTH + c * 8 + j]*quant_table[i*8+j]);
            dct[(r * 8 + i)*8*WIDTH + c * 8 + j] = 0;
          }
        }
    }
  }
#endif
  return SANITY_SUCCESS;
}

int dct_trc_sig(void *_args, void* not_used_at_all, int faulty)
{
  dct_task_args_t *args = (dct_task_args_t*) _args;
  return _dct_trc_sig(args->r, args->c, args->i, args->j, faulty);
}



void spawn_dct_task_sig (long r, long c, long i, long j, uint8_t significance)
{
  task_t *task;
  dct_task_args_t args;

  args.r = r;
  args.c = c;
  args.i = i;
  args.j = j;

#ifdef SANITY
  task = new_task(dct_task, &args, sizeof(args), dct_trc_sig, NULL, 0, significance, 0);
#else
  task = new_task(dct_task, &args, sizeof(args), NULL, NULL, 0, significance, 0);
#endif
  push_task(task, "dct");
}


#endif



void DCT(unsigned char pic[], double dct[], double COS[], double C[]) {

  long r, c, k;

  for (r = 0; r < HEIGHT; r++)
    for (k=0; k < WIDTH/STEP_C; k++)
    {
      c = k*STEP_C;
#ifdef NON_SIGNIFICANT_TASKS
      spawn_dct_task_sig(r, c, 0, 0, 0);
#else
      spawn_dct_task(r, c, 0, 0, SIGNIFICANT );
#endif
      spawn_dct_task(r, c, 0, 2, 60  < RATIO*100);
      spawn_dct_task(r, c, 0, 4, 50  < RATIO*100);
      spawn_dct_task(r, c, 0, 6, 40  < RATIO*100);
      /*    spawn_dct_task(r, c, 2, 0, 90);
            spawn_dct_task(r, c, 2, 2, 85);
            spawn_dct_task(r, c, 2, 4, 75);
            spawn_dct_task(r, c, 2, 6, 60);*/
      spawn_dct_task(r, c, 4, 0, 60  < RATIO*100);
      spawn_dct_task(r, c, 4, 2, 50  < RATIO*100);
      spawn_dct_task(r, c, 4, 4, 40  < RATIO*100);
      spawn_dct_task(r, c, 4, 6, 30  < RATIO*100);
      /*      spawn_dct_task(r, c, 6, 0, 70);
              spawn_dct_task(r, c, 6, 2, 60);
              spawn_dct_task(r, c, 6, 4, 50);
              spawn_dct_task(r, c, 6, 6, 40);*/
    }
#ifdef TIMER
  wait_group("dct", NULL, NULL, SYNC_RATIO|SYNC_TIME, 400, 0, 1.0f, 0);
#else
  wait_group("dct", NULL, NULL, SYNC_RATIO, 0, 0, 1.0f, 0);
#endif
  //#pragma taskwait all label(dct)

  return;
}


void idct_task(void *_args, unsigned int task_id, unsigned int significance)
{
  idct_task_args_t *args = (idct_task_args_t*) _args;
  _idct_task(args->r, args->c);
}

void _idct_task(long _r, long _c)
{
  long x, y, i, j, r, c;
  long c_e;
  double sum = 0;
  r = _r;
  c = _c;
  if ( WIDTH-_c > STEP_C )
    c_e = _c+STEP_C;
  else
    c_e = WIDTH; 

  for ( c = _c; c<c_e; ++c )
  {
    for ( i=0; i<8; ++i )
      for ( j=0; j<8; ++j)
      {
        sum = 0.0;
        for (x = 0; x < 8; x++)
          for (y = 0; y < 8; y++)
            sum += C[x] * C[y] * dct[(r * 8 + x)*WIDTH*8+ c * 8 + y] * COS[i*8+x] * COS[j*8+y];
        idct[(r*8+i)*WIDTH*8+ c*8+ j] = sum*0.25+128;
      }
  }
}

void IDCT() {
  idct_task_args_t args;
  task_t *task;
  long r, c;
  FILE *out;

  for (r = 0; r < HEIGHT; r++)
    for (c = 0; c < WIDTH; c+=STEP_C)
    { 
      args.r = r;
      args.c = c;
      task = new_task(idct_task, &args, sizeof(args), NULL, NULL, 0, 1, 0);
      push_task(task, "idct");
    }

  wait_group("idct", NULL, NULL, SYNC_RATIO, 0, 0, 1.0f, 0);
  // freopen("decoded_image.raw", "wb", stdout);
  out = fopen("decoded_image.raw", "wb");
  assert(out);
  for (r = 0; r < N; r++)
    for (c = 0; c < N; c++)
      fputc(idct[r*WIDTH*8+c], out);
  fclose(out);
}

double MSE_PSNR() {
  double MSE = 0;
  int r, c;
  double PSNR;

  for (r = 0; r < N; r++)
    for (c = 0; c < N; c++) {
      MSE += (pic[r*8*WIDTH+c] - idct[r*8*WIDTH+c]) * (pic[r*8*WIDTH+c] - idct[r*8*WIDTH+c]);
    }
  MSE /= (N * N);
  PSNR = 10 * log10(255 * 255 / MSE);
  return PSNR;
}

void fillInDCT(double *dct){
  FILE *fd = fopen("ApplicationOutput","rb");
  if ( fread(dct,sizeof(double),512*512,fd) != 512*512 )
  {
    assert(0 && "Failed to open file");
  }
  fclose(fd);
}


int main(int argc, char* argv[]) {
  int r, c, THREADS;
  long start, end;
  double psnr;
  int bytes, page;
  FILE *in;
  int non_sig;

  if ( argc != 5 ) {
    printf("usage %s ''WIDTH in blocks'' ''HEIGHT in blocks'' ''SIG_RATIO''"
        " ''THREADS''\n", argv[0]);
    return (0);
  }




  /*  C = malloc(sizeof(double)*8);
      COS = malloc(sizeof(double)*64); 
      pic = malloc(sizeof(unsigned char)*WIDTH*HEIGHT*64);
   */
  WIDTH = atoi(argv[1]);
  HEIGHT = atoi(argv[2]);
  RATIO = atof(argv[3]);
  THREADS = atoi(argv[4]);
#ifndef GEMFI
  dct = malloc(sizeof(double)*WIDTH*HEIGHT*64);
#endif
  idct = malloc(sizeof(double)*WIDTH*HEIGHT*64);
  bytes = sizeof(double) *(64+8) + sizeof(int)*64 + (WIDTH*HEIGHT*64);
  page = sysconf(_SC_PAGESIZE);
  bytes = ceil(bytes/(double)page) * page;
  if ( posix_memalign((void**)&quant_table, page, bytes) )
  {
    assert(0 && "Failed to allocate memory");
  }

  C = (double*)(quant_table + 64 );
  COS = C + 8;
  pic = (unsigned char*)(COS + 64 );
  memcpy(quant_table, _quant_table, sizeof(int)*64);


#if 1
  assert(N==WIDTH*8);
  in = fopen("lena512.raw", "rb");
  assert(in);
  for (r = 0; r < N; r++)
    for (c = 0; c < N; c++){
      dct[r*N+c] = 0.0;
      assert(fscanf(in, "%c", &pic[r*8*WIDTH+c]));
    }
  fclose(in);
#else
  for (r = 0; r < N; r++)
    for (c = 0; c < N; c++){
      pic[r*8*WIDTH+c] = r%16;
    }
#endif

  init_coef();
  for (r = 0; r < HEIGHT*8; r++)
    for (c = 0; c < WIDTH*8; c++){
      pic[r*8*WIDTH+c] = pic[(r%512)*8*WIDTH+c%512];
    }

#ifdef PROTECT
  mprotect(quant_table, bytes,  PROT_READ);
#endif

  non_sig = THREADS/2;
  if ( non_sig == 0 )
  {
    non_sig = 1;
  }
#ifdef GEMFI
  m5_switchcpu();
#endif
  init_system(THREADS-non_sig, non_sig);
#ifdef GEMFI
#warning compiling for gemfi execution
  m5_dumpreset_stats(0,0);
#else
  start = this_time();
#endif
  DCT(pic, dct, COS, C);
#ifdef GEMFI
#warning compiling for gemfi execution
  stop_exec();
  m5_dumpreset_stats(0,0);
  int i;
  unsigned char *vals = (unsigned char *) dct;
  for ( i = 0 ;  i < WIDTH*HEIGHT*64*sizeof(double) ; i++){
    m5_writefile((unsigned long) vals[i],sizeof(unsigned char) ,i);
  }
#else
  //  fillInDCT(dct);
  IDCT();
  end = this_time();
  psnr = MSE_PSNR();
  fprintf(stderr, "===Approx DCT===\n");
  fprintf(stderr, "  Duration[ms]=%g\n", (double)(end-start)/1000.0);
  fprintf(stderr, "  PSNR=%g\n", psnr);
  fprintf(stderr, "  Ratio=%g\n", RATIO);
  fprintf(stderr, "=================\n");
#endif
  return 0;
}

