# C compiler
CC = gcc
CFLAGS=-g -pthread -Wall -lm # -O3 # -DDEBUG
LDFLAGS=-lrtsrel -lm -lrt
LIB=-L../../
INCLUDE=-I../../include/

kmeans: cluster.o getopt.o kmeans.o kmeans_clustering.o runtime
	$(CC) $(CFLAGS) $(LIB) $(INCLUDE) cluster.o getopt.o kmeans.o kmeans_clustering.o  -o kmeans $(LDFLAGS)

runtime:
	make -C ../../

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
