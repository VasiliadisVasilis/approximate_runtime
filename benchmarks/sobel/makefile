OPT= -O3 -g # -DDEBUG
#use CFLAGS = -DGEMFI to enable gemfi faults and -DSANITY to enable trc

LIKWID_BASE=/opt/likwid_v3.1
CFLAGS=-Wno-format -std=c99   -D_GNU_SOURCE -DPAGE_ALIGNMENT=4096 -DLIKWID_MONITOR_LOCK -DDEBUGLEV=0 -DVERSION=3 -DRELEASE=1 -DCFGFILE=/etc/likwid.cfg -DMAX_NUM_THREADS=263 -DMAX_NUM_NODES=4 -DHASH_TABLE_SIZE=20 -DLIBLIKWIDPIN=/usr/local/lib/liblikwidpin.so -DLIKWIDFILTERPATH=/usr/local/share/likwid -DCOLOR=BLUE -DHAS_MEMPOLICY -DHAS_RDTSCP -DHAS_SCHEDAFFINITY -DACCESSDAEMON=/usr/local/sbin/likwid-accessD -DACCESSMODE=1 $(OPT)

INCLUDE = -I$(LIKWID_BASE)/src/includes  -I$(LIKWID_BASE)/GCC  -I../../include/ -I../../fi -pthread 

OBJ_BENCH += $(LIKWID_BASE)/GCC/clcopy.o  $(LIKWID_BASE)/GCC/clload.o  $(LIKWID_BASE)/GCC/clstore.o  $(LIKWID_BASE)/GCC/copy_mem.o  $(LIKWID_BASE)/GCC/copy.o  $(LIKWID_BASE)/GCC/load.o  $(LIKWID_BASE)/GCC/peakflops.o  $(LIKWID_BASE)/GCC/peak.o  $(LIKWID_BASE)/GCC/store_mem.o  $(LIKWID_BASE)/GCC/store.o  $(LIKWID_BASE)/GCC/stream_mem.o  $(LIKWID_BASE)/GCC/stream.o  $(LIKWID_BASE)/GCC/sum_avx.o  $(LIKWID_BASE)/GCC/sum_plain.o  $(LIKWID_BASE)/GCC/sum.o  $(LIKWID_BASE)/GCC/triad_mem.o  $(LIKWID_BASE)/GCC/triad.o  $(LIKWID_BASE)/GCC/update.o 

LFLAGS = -L$(LIKWID_BASE)/ -L../../ -L../../fi

LIBS += -lm -lrt -lrtsrel  -llikwid  -lpthread 


sobel:sobel.o
	$(CC) -o sobel $(CFLAGS) $(INCLUDE) $(LFLAGS) sobel.o $(LIBS) 

sobel.o: sobel.c
	$(CC) $(CFLAGS) $(INCLUDE) sobel.c -c 

image:
	convert -depth 8 -size 512x512 GRAY:decoded_image.raw decoded_image.jpg

clean:
	rm -f *.o *~ sobelimage: decoded_image.raw
