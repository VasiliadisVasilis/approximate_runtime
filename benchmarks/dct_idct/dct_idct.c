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

#define STEP_ROW 16

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
  long r, i, j, step, length;
};

struct IDCT_TASK_ARGS_T
{
  long r, c;
};

#ifdef SANITY
int dct_trc(void *args, void *not_used_at_all, int faulty);
int _dct_trc(long _r, long _c, long _i, long _j, int faulty);
#endif

void dct_task(void* args);
void idct_task(void* args);
void _idct_task(long _r);
void dct_task_dc(void *dummy);


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

void dct_task_dc(void *dummy)
{
  long x, y, r, c, l;
  double sum = 0, factor = C[0] * C[0] * 0.25;

  for ( r = 0; r<HEIGHT; ++r)
  {
    for ( c = 0; c<WIDTH; ++c )
    {
      sum = 0;
      for (x = 0; x < 8; x++)
        for (y = 0; y < 8; y++)
        {
          sum += ( (double)pic[(r * 8 + x)*8*WIDTH + c * 8 + y] - 128) *
            COS[x*8] * COS[y*8];
        }
      sum *= factor;
      dct[(r * 8 )*8*WIDTH + c * 8] = sum;
      quantization_task(dct, quant_table[0], r, c, 0, 0);
    }
  }
}


void dct_task(void *_args)
{
  dct_task_args_t *args = (dct_task_args_t*) _args;
  long _r = args->r;
  long _i = args->i;
  long _j = args->j;
  long step = args->step;
  long length = args->length;
  long x, y, i, j, r, c, l;
  double sum = 0;

  for ( r = _r; r<_r+STEP_ROW; ++r)
  {
    for ( c = 0; c<WIDTH; ++c )
    {
      i = _i;
      j = _j;

      for ( l=0; l<length; ++l)
      {
        sum = 0;
        for (x = 0; x < 8; x++)
          for (y = 0; y < 8; y++)
          {
            sum += ( (double)pic[(r * 8 + x)*8*WIDTH + c * 8 + y] - 128) *
              COS[x*8+i] * COS[y*8+j];
          }
        sum *= C[i] * C[j] * 0.25;
        dct[(r * 8 + i)*8*WIDTH + c * 8 + j] = sum;
        quantization_task(dct, quant_table[i*8+j], r, c, i, j);
        i+=-step;
        j+=step;
      }
    }
  }
}

void spawn_dct_task(int row, int i, int j, int length, int step,
    uint8_t significance)
{
  task_t *task;
  dct_task_args_t args;

  args.r = row;
  args.i = i;
  args.j = j;
  args.step = step;
  args.length = length;

  task = new_task(dct_task, &args, sizeof(args), NULL, significance);
  push_task(task, "dct");
}



void DCT(unsigned char pic[], double dct[], double COS[], double C[]) {

  long r;

  task_t *task;

  task = new_task(dct_task_dc, NULL, 0, NULL, 100);
  push_task(task, "dct");

  /*  100, 49, 25, 16, 12, 9, 8, 7, 6, 5, 4, 3, 2, 2, 1*/
  for (r = 0; r < HEIGHT; r+=STEP_ROW)
  {
    spawn_dct_task(r, 0, 1,   2, -1,  49);
    spawn_dct_task(r, 2, 0,   3,  1,  25);
    spawn_dct_task(r, 0, 3,   4, -1,  16);
    spawn_dct_task(r, 4, 0,   5,  1,  12);
    spawn_dct_task(r, 0, 5,   6, -1,  10);
    spawn_dct_task(r, 6, 0,   7,  1,   9);

    spawn_dct_task(r, 0, 7,   8, -1,   8);

    spawn_dct_task(r, 7, 1,   7,  1,   7);
    spawn_dct_task(r, 2, 7,   6, -1,   6);
    spawn_dct_task(r, 7, 3,   5,  1,   5);
    spawn_dct_task(r, 4, 7,   4, -1,   4);
    spawn_dct_task(r, 7, 5,   3,  1,   3);
    spawn_dct_task(r, 6, 7,   2, -1,   2);
    spawn_dct_task(r, 7, 7,   1,  1,   1);
  }
  wait_group("dct", NULL, NULL, SYNC_RATIO, 0, 0, RATIO, 0);
  return;
}


void idct_task(void *_args)
{
  idct_task_args_t *args = (idct_task_args_t*) _args;
  _idct_task(args->r);
}

void _idct_task(long _r)
{
  long x, y, i, j, r, c;
  double sum = 0;
  r = _r;

  for ( c = 0; c<WIDTH; ++c )
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

  for (r = 0; r < HEIGHT; r++)
  { 
    args.r = r;
    args.c = 0;
    task = new_task(idct_task, &args, sizeof(args), NULL, 100);
    push_task(task, "idct");
  }

  wait_group("idct", NULL, NULL, SYNC_RATIO, 0, 0, 1.0f, 0);
  // freopen("decoded_image.raw", "wb", stdout);
  /*
  FILE *out;
  out = fopen("decoded_image.raw", "wb");
  assert(out);
  for (r = 0; r < N; r++)
    for (c = 0; c < N; c++)
      fputc(idct[r*WIDTH*8+c], out);
  fclose(out);
  */
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

  WIDTH = atoi(argv[1]);
  HEIGHT = atoi(argv[2]);
  RATIO = atof(argv[3]);
  THREADS = atoi(argv[4]);
  dct = malloc(sizeof(double)*WIDTH*HEIGHT*64);
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


  in = fopen("lena512.raw", "rb");
  assert(in);
  for (r = 0; r < N; r++)
    for (c = 0; c < N; c++){
      dct[r*N+c] = 0.0;
      assert(fscanf(in, "%c", &pic[r*8*WIDTH+c]));
    }
  fclose(in);

  init_coef();
  for (r = 0; r < HEIGHT*8; r++)
    for (c = 0; c < WIDTH*8; c++){
      pic[r*8*WIDTH+c] = pic[(r%512)*8*WIDTH+c%512];
    }


  non_sig = THREADS/2;
  if ( non_sig == 0 )
  {
    non_sig = 1;
  }
  init_system(THREADS);
  start = this_time();
  DCT(pic, dct, COS, C);
  //  fillInDCT(dct);
  IDCT();
  end = this_time();
  shutdown_system();
  psnr = MSE_PSNR();
  printf("Duration,%g\n", (double)(end-start)/1000.0);
  printf("PSNR,%.32lg\n", psnr);
  printf("Ratio,%g\n", RATIO);
  return 0;
}

