#include <stdio.h>
#include <assert.h>
#include <math.h>

int main(int argc, char* argv[])
{
	FILE *f1, *f2;
	int i;

	if ( argc != 3 )
	{
		printf("%s file1 file2\n", argv[0]);
		return 1;
	}

	f1 = fopen(argv[1], "r");
	f2 = fopen(argv[2], "r");

	int num1, num2;
	double en1, en2;

	int ret;
	
	ret = fread(&num1, sizeof(int), 1, f1); assert(ret==1);
	ret = fread(&num2, sizeof(int), 1, f2); assert(ret==1);
	assert(num1==num2);
	/*
	ret = fread(&en1, sizeof(double), 1, f1); assert(ret==1);
	ret = fread(&en2, sizeof(double), 1, f2); assert(ret==1);
	
	printf("ERROR=%32g\n", fabs(en1-en2)/fabs(en1));
	*/
	#if 0
	double total = 0, diff = 0;
	
	float p1, p2;
	printf("Num%d\n", num1);
	for ( i=0; i<num1*6; ++i )
	{
		ret = fread(&p1, sizeof(float), 1, f1); assert(ret==1);
		ret = fread(&p2, sizeof(float), 1, f2); assert(ret==1);
		diff += fabs(p1-p2);
		total += fabs(p1);
	}

	printf("ERROR=%.32g\n", diff/total);
	#else
	double t= 0, diff = 0,total_square=0, total=0;
	
	double p1, p2;
	printf("Num%d\n", num1);
	for ( i=0; i<num1; ++i )
	{
		ret = fread(&p1, sizeof(double), 1, f1); assert(ret==1);
		ret = fread(&p2, sizeof(double), 1, f2); assert(ret==1);
		t = p1-p2;
		t = t * t;
		diff += t;

		total += p1;
		total_square += p1*p1;
	}
	double average = total/(double)(num1*7);
	total = 2*total*average + total_square + total;


	printf("ERROR=%.32g\n", diff/total);

	#endif
	return 0;
}
