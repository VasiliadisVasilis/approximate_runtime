#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#define SIZE(x,y)  (4 * x * y *3)
int main(int argc, char *argv[])
{
	FILE *corr, *perf;
	corr = fopen(argv[1],"rb");
	perf = fopen(argv[2], "rb");

	if ( corr == NULL )
	{
		printf("Could not access corr\n");
		return 1;
	}

	if ( perf == NULL )
	{
		printf("Could not access perf\n");
		return 1;
	}

  int x = atoi(argv[3]);
  int y = atoi(argv[4]);
	int i, t;
	unsigned char c,p;
	double MSE, PSNR;
	for ( i = 0 ; i < 54 ; i++){
		c = getc(perf);
		p = getc(corr);
	}
	MSE = 0.0;
	for ( i = 0 ; i < SIZE(x,y) ; i++){
		int ret = fread(&c,sizeof(char),1,corr);
		ret += fread(&p,sizeof(char),1,perf);
		if ( ret != 2 )
		{
			printf("Failed to read 2 chars\n");
			return 1;
		}
		t = (int)c - (int)p;
		if ( t > 256 )
		  exit(0);
		t = t * t;
		MSE = MSE + t;
	}
	MSE =MSE/SIZE(x,y);
	
	PSNR = 10 * log10(255*255/MSE);

	printf("ERROR=%.2lf\n",PSNR);
	
	return 0;

}
