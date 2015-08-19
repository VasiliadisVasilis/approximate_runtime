// This will apply the sobel filter and return the PSNR between the golden sobel and the produced sobel
// sobelized image
#include <stdio.h>
#include <math.h>
#include <string.h>
#include <stdlib.h>
#include <sys/time.h>
#include <runtime.h>

#define STEP 10

typedef unsigned char byte;

long this_time()
{
	struct timeval tv;
	gettimeofday(&tv, NULL);
	return tv.tv_sec*1000000+tv.tv_usec;
}

typedef struct SOBEL_ARGS
{
	byte *out, *input;
	int width, height, row;
} sobel_args_t;


int inline sobely1(byte input[], int width, int y, int x)
{
	int ret = -(int)(unsigned int)input[(y-1)*width+ x-1] +
		(int)(unsigned int)input[(y+1)*width+ x-1];
	return ret;
}

int inline sobely2(byte input[], int width, int y, int x)
{
	int ret = -2.0*(int)(unsigned int)input[(y-1)*width+ x] +
		2.0*(int)(unsigned int)input[(y+1)*width+ x];
	return ret;
}

int inline sobely3(byte input[], int width, int y, int x)
{
	int ret = -(int)(unsigned int)input[(y-1)*width+ x+1] +
		(int)(unsigned int)input[(y+1)*width+ x+1];
	return ret;
}

int inline sobelx1(byte input[], int width, int y, int x)
{
	int ret = -(int)(unsigned int)input[(y-1)*width+ x-1] +
		(int)(unsigned int)input[(y-1)*width+ x+1];
	return ret;
}

int inline sobelx2(byte input[], int width, int y, int x)
{
	int ret = -2.0*(int)(unsigned int)input[(y)*width+ x-1] +
		2.0*(int)(unsigned int)input[(y)*width+ x+1];
	return ret;
}

int inline sobelx3(byte input[], int width, int y, int x)
{
	int ret = -(int)(unsigned int)input[(y+1)*width+ x-1] +
		(int)(unsigned int)input[(y+1)*width+ x+1];
	return ret;
}

void sobel_task(void *_args)
{
	byte *out, *input;
	int width, height, row;
	unsigned int x, y;
	int i, j;
	sobel_args_t *args = (sobel_args_t*) _args;

	out = args->out;
	input = args->input;
	width = args->width;
	height = args->height;
	row = args->row;
	int end = row+STEP < height-1 ? row+STEP : height-1;

	for ( i=row; i<end; ++i )
	{
		for ( j=1; j<width-1; ++j)
		{
			x = sobelx3(input, width, i, j) + sobelx1(input, width, i, j) 
				+  sobelx2(input, width, i, j);
			y = sobely3(input, width, i, j) + sobely1(input, width, i, j) 
				+  sobely2(input, width, i, j);
			x = x*x + y * y;
			x = sqrt(x);
			if ( x > 255)
				x = 255;
			out[i*width+j] = x;
		}
	}

}

void sobel_task_approx(void *_args)
{
	byte *out, *input;
	int width, height, row;
	int x, y;
	int i, j;
	sobel_args_t *args = (sobel_args_t*) _args;

	out = args->out;
	input = args->input;
	width = args->width;
	height = args->height;
	row = args->row;

	int end = row+STEP < height-1 ? row+STEP : height-1;
	
	for ( i=row; i<end; ++i )
	{
		for ( j=1; j<width-1; ++j)
		{
			x = sobelx2(input, width, i, j);
			y = sobely2(input, width, i, j);
			x = abs(x) + abs(y);
			x *= 1.45;
			if ( x > 255)
				x = 255;
			
			out[i*width+j] = x;
		}
	}
}

double calc_psnr(byte *output_image, byte *sobel_image,
		unsigned int width, unsigned int height)
{
	double PSNR,t ;
	unsigned int p, row, column;
	PSNR = 0.0;
	for (row=1; row<height-1; row++) {
		for ( column=1; column<width-1; column++ ) {
			t = ((int)((unsigned int)output_image[row*width+column]) - (int)((unsigned int)sobel_image[row*width+column]));
			t = t *  t;
			PSNR += t;
		}
	}

	PSNR = PSNR/ (double)(width * height);
	PSNR = 10.0*log10(65536.0/PSNR);

	return PSNR;

}

void sobel(byte *output_image, byte *input_image,
		unsigned int width, unsigned int height, double ratio )
{
	unsigned int p, row, column;
	sobel_args_t args;
	task_t *task;
	static int cur = 0;
	char group_name[1024];

	sprintf(group_name, "sobel_%d", ++cur);

	args.out = output_image;
	args.input = input_image;
	args.width = width;
	args.height = height;

	for (row=1; row<height-1; row+=STEP) {
		args.row = row;
		task = new_task(sobel_task, &args, sizeof(args), sobel_task_approx, 50 + (row/STEP)%10 );
		push_task(task, group_name);
	}

	wait_group(group_name, NULL, NULL, SYNC_RATIO, 0, 0, ratio, 0);
}



int main(int argc, char* argv[])
{
	double PSNR, ratio;
	byte *output_image, *input_image, *sobel_image;
	unsigned int width, height, row, column, i;
	unsigned char c, *pic;
	long start, end;
	FILE *f;

	if ( argc != 5 )
	{
		printf("%s width height ratio threads\n", argv[0]);
		return 1;
	}
	
	width = atoi(argv[1]);
	height = atoi(argv[2]);
	ratio = atof(argv[3]);
	
	pic = (char*) malloc(sizeof(char)*512*512);
	input_image= (byte*) malloc(sizeof(byte)*width*height);
	sobel_image= (byte*) malloc(sizeof(byte)*width*height);
	output_image= (byte*) malloc(sizeof(byte)*width*height);

	f = fopen("peppers_512x512_25Hz_8bit.bw", "rb");
	fread(pic, sizeof(char), 512*512, f);
	fclose(f);
	for ( row=0; row<height; ++row )
	{
		for (column=0; column<width; ++column )
		{
			input_image[row*width+column] = pic[(row%512)*512+column%512];
		}
	}


	f = fopen("golden_peppers_512x512_25Hz_8bit.bw", "rb");
	for ( row=0; row<height; ++row )
	{
		for (column=0; column<width; ++column )
		{
			fread(&c, sizeof(unsigned char), 1, f);
			sobel_image[row*width+column] = c;
		}
	}
	fclose(f);

	init_system(atoi(argv[4]));

	start = this_time();
	for ( i=0; i<50; ++i )
	{
		sobel(output_image, input_image, width, height, ratio);
	}
	end = this_time();
	shutdown_system();
	PSNR = calc_psnr(output_image, sobel_image, width, height);
	printf("PSNR=%lg\n", PSNR);
#if 0
	f = fopen("sobel_output_512x512_25Hz_8bit.bw", "wb");
	for ( row=0; row<height; ++row )
	{
		for (column=0; column<width; ++column )
		{
			c = (unsigned char)output_image[row*width+column];
			fwrite(&c, sizeof(unsigned char), 1, f);
		}
	}
	fclose(f);
#endif
	return 0;
}

