// This will apply the sobel filter and return the PSNR between the golden sobel and the produced sobel
// sobelized image
#include <stdio.h>
#include <math.h>
#include <string.h>
#include <stdlib.h>
#include <sys/time.h>
#include <runtime.h>

#define STEP 8

long this_time()
{
	struct timeval tv;
	gettimeofday(&tv, NULL);
	return tv.tv_sec*1000000+tv.tv_usec;
}

typedef struct SOBEL_ARGS
{
	double	*out, *input;
	int width, height, row;
} sobel_args_t;


double sobely1(double input[], int width, int y, int x)
{
	double ret = -input[(y-1)*width+ x-1] +
		input[(y+1)*width+ x-1];
	return ret;
}

double sobely2(double input[], int width, int y, int x)
{
	double ret = -2.0*input[(y-1)*width+ x] +
		2.0*input[(y+1)*width+ x];
	return ret;
}

double sobely3(double input[], int width, int y, int x)
{
	double ret = -input[(y-1)*width+ x+1] +
		input[(y+1)*width+ x+1];
	return ret;
}

double sobelx1(double input[], int width, int y, int x)
{
	double ret = -input[(y-1)*width+ x-1] +
		input[(y-1)*width+ x+1];
	return ret;
}

double sobelx2(double input[], int width, int y, int x)
{
	double ret = -2.0*input[(y)*width+ x-1] +
		2.0*input[(y)*width+ x+1];
	return ret;
}

double sobelx3(double input[], int width, int y, int x)
{
	double ret = -input[(y+1)*width+ x-1] +
		input[(y+1)*width+ x+1];
	return ret;
}

void sobel_task(void *_args)
{
	double *out, *input;
	int width, height, row;
	double  x, y;
	int i, j;
	sobel_args_t *args = (sobel_args_t*) _args;

	out = args->out;
	input = args->input;
	width = args->width;
	height = args->height;
	row = args->row;

	for ( i=row; i<row+STEP && i < height-1; ++i )
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
	double *out, *input;
	int width, height, row;
	double  x, y;
	int i, j;
	sobel_args_t *args = (sobel_args_t*) _args;

	out = args->out;
	input = args->input;
	width = args->width;
	height = args->height;
	row = args->row;

	for ( i=row; i<row+STEP && i < height-1; ++i )
	{
		for ( j=1; j<width-1; ++j)
		{
			x = sobelx2(input, width, i, j);
			y = sobely2(input, width, i, j);
			x = x*x + y * y;
			x = sqrt(x);
			if ( x > 255)
				x = 255;

			out[i*width+j] = x;
		}
	}
}

double calc_psnr(double *output_image, double *sobel_image,
		unsigned int width, unsigned int height)
{
	double PSNR,t ;
	unsigned int p, row, column;
	PSNR = 0.0;
	for (row=1; row<height-1; row++) {
		for ( column=1; column<width-1; column++ ) {
			t = ((int)output_image[row*width+column] - (int)sobel_image[row*width+column]);
			t = t *  t;
			PSNR += t;
		}
	}

	PSNR = PSNR/ (double)(width * height);
	PSNR = 10.0*log10(65536.0/PSNR);

	return PSNR;

}

void sobel(double *output_image, double *input_image,
		unsigned int width, unsigned int height, double ratio )
{
	unsigned int p, row, column;
	sobel_args_t args;
	task_t *task;

	args.out = output_image;
	args.input = input_image;
	args.width = width;
	args.height = height;

	for (row=1; row<height-1; row+=STEP) {
		args.row = row;
		task = new_task(sobel_task, &args, sizeof(args), sobel_task_approx, 50);
		push_task(task, "sobel");
	}

	wait_group("sobel", NULL, NULL, SYNC_RATIO, 0, 0, ratio, 0);
}

double mmax(double a, double b)
{
	if ( a > b )
		return a;
	return b;
}


double mmin(double a, double b)
{
	if ( a < b )
		return a;
	return b;
}


int main(int argc, char* argv[])
{
	double PSNR, ratio;
	double *output_image, *input_image, *sobel_image;
	unsigned int width, height, row, column, k;
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
	input_image= (double*) malloc(sizeof(double)*width*height);
	sobel_image= (double*) malloc(sizeof(double)*width*height);
	output_image= (double*) malloc(sizeof(double)*width*height);

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
	sobel(output_image, input_image, width, height, ratio);
	end = this_time();
	shutdown_system();
	PSNR = calc_psnr(output_image, sobel_image, width, height);
	printf("PSNR,%lg\n", PSNR);
	printf("Duration,%lg\n", (end-start)/1000.0);

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

	return 0;
}

