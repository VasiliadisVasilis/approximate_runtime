# C compiler
CC = gcc
CFLAGS=-g -pthread -Wall -lm  -O3 # -DDEBUG
LDFLAGS=-lrtsrel -lm -lrt
LIB=-L../../
INCLUDE=-I../../include/ -I.

jacobi: runtime jacobi.o
	$(CC) $(CFLAGS) $(LIB) $(INCLUDE) jacobi.o -o jacobi $(LDFLAGS)

runtime:
	make -C ../../

%.o: %.[ch]
	$(CC) $(CFLAGS) $(LIB) $(INCLUDE) $< -c

clean:
	rm -f *.o *~ jacobi
