# C compiler
CC = gcc
CFLAGS=-g -pthread -Wall -lm  -O3 # -DDEBUG
LDFLAGS=-lrtsrel -lm -lrt
LIB=-L../../
INCLUDE=-I../../include/ -I.

ldpc: timer.o main.o kernel.o runtime
	$(CC) $(CFLAGS) $(LIB) $(INCLUDE) timer.o main.o kernel.o -o ldpc $(LDFLAGS)

runtime:
	make -C ../../

%.o: %.[ch]
	$(CC) $(CFLAGS) $(LIB) $(INCLUDE) $< -c

clean:
	rm -f *.o *~ ldpc
