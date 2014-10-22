CFLAGS=-g -pthread -Wall -lm -O3 -DDEBUG
LDFLAGS=-lrtsrel -lm -lrt
LIB=-L../../
INCLUDE=-I../../include/

all: runtime dct

runtime:
	make -C ../../

dct: dct_idct.c
	gcc ${CFLAGS} ${LIB} ${INCLUDE} -DSTEP_C=4 dct_idct.c ${LDFLAGS} -o dct

clean:
	rm -f decoded_image.raw decoded_image.jpg PSNR.txt dct_idct_omp  approx_bddt exact_bddt perfor_bddt dct

image: decoded_image.raw
	convert -depth 8 -size 512x512 GRAY:decoded_image.raw decoded_image.jpg
