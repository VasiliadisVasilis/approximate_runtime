override CFLAGS+= -pthread -Wall  --static -O3 # -DDEBUG
#use CFLAGS = -DGEMFI to enable gemfi faults and -DSANITY to enable trc
LDFLAGS=-lrtsrel -lm -lrt -lm5
LIB=-L../../ -L../../fi
INCLUDE=-I../../include/ -I../../fi
CC?=clang

kmeans: cluster.o getopt.o kmeans.o kmeans_clustering.o runtime
	gcc $(CFLAGS) $(LIB) $(INCLUDE) cluster.o getopt.o kmeans.o kmeans_clustering.o  -o kmeans $(LDFLAGS)

runtime:
	make -C ../../ "CFLAGS=$(CFLAGS)"

%.o: %.[ch]
	$(CC) $(CFLAGS) $(LIB) $(INCLUDE) $< -c

cluster.o: cluster.c 
	$(CC) $(CFLAGS) $(LIB) $(INCLUDE)  cluster.c -c
	
getopt.o: getopt.c 
	$(CC) $(CFLAGS)  $(LIB) $(INCLUDE) getopt.c -c
	
kmeans.o: kmeans.c 
	$(CC) $(CFLAGS)  $(LIB) $(INCLUDE) kmeans.c -c

kmeans_clustering.o: kmeans_clustering.c kmeans.h
	$(CC) $(CFLAGS)  $(LIB) $(INCLUDE) kmeans_clustering.c -c

clean:
	rm -f *.o *~ kmeans 
