#include <stdio.h>
#include <math.h>
#include <stdlib.h>

int main(int argc, char *argv[])
{
  FILE *input1, *input2;
  double d, e;
  int iter, itercorrect;
  double rel_err = 0 ;
  double err1, err2;
  int identical = 1;
  int i = 0;
  

  if ( argc != 5 )
  {
    printf("%s Diff CorrectDiff Iterations CorrectIterations\n", argv[0]);    
    return 1;
  }

  err1 = strtod(argv[1], NULL);
  err2 = strtod(argv[2], NULL);
  iter = atoi(argv[3]);
  itercorrect = atoi(argv[4]);

    
  rel_err = fabs(err2-err1)/err2;
  
//  printf("[%lg %lg] %lf : ", err1, err2, rel_err);
  
  identical = iter == itercorrect;
  identical &= (err1 == err2);


  if ( identical )
  {
    printf("strict");
  } else if ( err1 < err2 || rel_err < 0.1f && iter < 60 )
  {
    printf("correct");
  }
  else
  {
    printf("altered");
  }
  return 0;
}
