override CFLAGS+= -pthread -Wall  --static -O3  # -DDEBUG
#use CFLAGS = -DGEMFI to enable gemfi faults and -DSANITY to enable trc
LDFLAGS=-lrtsrel -lm -lrt -lm5
LIB=-L../../ -L../../fi
INCLUDE=-I../../include/ -I../../fi

jacobi:jacobi.o
	gcc $(CFLAGS) $(LIB) $(INCLUDE) jacobi.o -o jacobi $(LDFLAGS)

jacobi.o: jacobi.c
	$(CC) $(CFLAGS) $(OPT) $(LIB) $(INCLUDE) jacobi.c -c 


runtime:
	make -C ../../

%.o: %.[ch]
	$(CC) $(CFLAGS) $(LIB) $(INCLUDE) $< -c

clean:
	rm -f *.o *~ jacobi
