override CFLAGS+= -pthread -Wall  --static -O3 # -DDEBUG
#use CFLAGS = -DGEMFI to enable gemfi faults and -DSANITY to enable trc
LDFLAGS=-lrtsrel -lm -lrt -lm5
LIB=-L../../ -L../../fi
INCLUDE=-I../../include/ -I../../fi
CC?=clang
all: runtime dct

runtime:
	make -C ../../

dct: dct_idct.o
	gcc ${CFLAGS} ${LIB} dct_idct.o -o dct  ${LDFLAGS}
dct_idct.o: dct_idct.c
	$(CC) ${CFLAGS} ${LIB} ${INCLUDE} -DSTEP_C=4 dct_idct.c -c 

clean:
	rm -f decoded_image.raw decoded_image.jpg PSNR.txt dct_idct_omp  approx_bddt exact_bddt perfor_bddt dct dct_idct.o

image: decoded_image.raw
	convert -depth 8 -size 512x512 GRAY:decoded_image.raw decoded_image.jpg
