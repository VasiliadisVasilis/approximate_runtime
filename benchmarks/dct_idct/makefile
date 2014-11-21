override CFLAGS+= -pthread -Wall  -O3 --static# -DDEBUG
#use CFLAGS = -DGEMFI to enable gemfi faults and -DSANITY to enable trc
LDFLAGS=-lrtsrel -lm -lrt -lm5
LIB=-L../../ -L../../fi
INCLUDE=-I../../include/ -I../../fi

all: runtime dct

runtime:
	make -C ../../

dct: dct_idct.c
	gcc ${CFLAGS} ${LIB} ${INCLUDE} -DSTEP_C=64 dct_idct.c ${LDFLAGS} -o dct

clean:
	rm -f decoded_image.raw decoded_image.jpg PSNR.txt dct_idct_omp  approx_bddt exact_bddt perfor_bddt dct

image: decoded_image.raw
	convert -depth 8 -size 512x512 GRAY:decoded_image.raw decoded_image.jpg
