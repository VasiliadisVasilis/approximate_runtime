#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#define SIZE(x,y)  (x * y *3)

typedef unsigned short WORD;
typedef unsigned int  DWORD;

typedef struct tagBmpHeader {
  //	char  type[2];       /* = "BM", it is matipulated separately to make the size of this structure the multiple of 4, */
  DWORD sizeFile;      /* = offbits + sizeImage */
  DWORD reserved;      /* == 0 */
  DWORD offbits;       /* offset from start of file = 54 + size of palette */
  DWORD sizeStruct;    /* 40 */
  DWORD width, height;
  WORD  planes;        /* 1 */
  WORD  bitCount;      /* bits of each pixel, 256color it is 8, 24 color it is 24 */
  DWORD compression;   /* 0 */
  DWORD sizeImage;     /* (width+?)(till multiple of 4) * height£¬in bytes */
  DWORD xPelsPerMeter; /* resolution in mspaint */
  DWORD yPelsPerMeter;
  DWORD colorUsed;     /* if use all, it is 0 */
  DWORD colorImportant;/*  */
} BmpHeader;


int main(int argc, char *argv[])
{
	FILE *corr, *perf;
	FILE *diff;
  BmpHeader h1, h2;

	corr = fopen(argv[1],"rb");
	perf = fopen(argv[2], "rb");

	if ( argc == 4 )
	diff = fopen(argv[3], "wb");

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

	int i, t;
	unsigned char c,p;
	double MSE, PSNR;
	for ( i = 0 ; i < 2 ; i++){
		c = getc(perf);
		p = getc(corr);
		if ( argc==4 )
			putc(c, diff);
	}

  fread(&h1, sizeof(h1), 1, perf);
  fread(&h2, sizeof(h2), 1, corr);

  if ( h1.width != h2.width 
      || h1.height != h2.height )
  {
    printf("File dimensions do not match (%dx%d and %dx%d)\n", 
      h1.width, h1.height, h2.width, h2.height);
  }
	if ( argc==4 )
		fwrite(&h1, sizeof(h1), 1, diff);

	MSE = 0.0;
	for ( i = 0 ; i < SIZE(h1.width,h1.height) ; i++){
		int ret = fread(&c,sizeof(char),1,corr);
		ret += fread(&p,sizeof(char),1,perf);
		if ( ret != 2 )
		{
			printf("Failed to read 2 chars\n");
			return 1;
		}
		t = abs((int)c - (int)p);
		
		c = t;

		if ( argc==4 )
			fwrite(&c, sizeof(char), 1, diff);

		t = t * t;
		MSE = MSE + t/(double)SIZE(h1.width,h1.height);
	}
	
	PSNR = 10.0 * log10(255.0*255.0/MSE);

	printf("ERROR=%.2lf\n",PSNR);


	if ( argc==4 )
		fclose(diff);
	fclose(corr);
	fclose(perf);
	
	return 0;

}
