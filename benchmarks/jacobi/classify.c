#include <stdio.h>
#include <math.h>

int main(int argc, char *argv[])
{
  FILE *input1, *input2;
  double d, e;
  double rel_err = 0 ;
  double err1, err2;
  int identical = 1;


  if ( argc != 5 )
  {
    printf("%s File1 File2 Iterations CorrectIterations\n", argv[0]);    
    return 1;
  }
  
  input1 = fopen(argv[0], "rb");
  input2 = fopen(argv[1], "rb");
  identical = atoi(argv[2]) == atoi(argv[3]);
  err1 = 0;
  err2 = 0;
  rel_err = 0;

  while( fread(&d, sizeof(double), 1, input1) 
    && fread(&d, sizeof(double), 1, input2))
  {
    if ( err1 < fabs(d-1.0) )
    {
      err1 = fabs(d-1.0);
    }
    if ( err2 < fabs(e-1.0) )
    {
      err2 = fabs(e-1.0);
    }
    
    identical = ( *((long*)&d)==*((long*)&e)) ? identical : 0;
  }
  
  rel_err = fabs(err2-err1)/err2;
  
  printf("Rel Error: %lf\nErr1: %lf\nErr2: %lf\n", rel_err, err1, err2);
  printf("Verdict: ");
  if ( identical )
  {
    printf("Strict\n");
  } else if ( err1 < err2 || rel_err < 0.1f  )
  {
    printf("Correct\n");
  }
  else
  {
    printf("SDC\n");
  }
  return 0;
}
