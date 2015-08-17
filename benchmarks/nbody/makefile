CXX?= g++
CC?= gcc

OPT= -O3 -g # -DDEBUG
#use CFLAGS = -DGEMFI to enable gemfi faults and -DSANITY to enable trc

CFLAGS = $(OPT) -pthread 

INCLUDE = -I../../include/ 

LFLAGS = -L../../

LIBS += -lm -lrt -lrtsrel -lpthread -lglut -lGLEW -lGLU

all: nbody diff

diff:compare.c
	gcc -O3 -o diff compare.c -lm

nbody:nbody.o simulation.o
	$(CXX) -o nbody $(CFLAGS) $(INCLUDE) $(LFLAGS) *.o $(LIBS) 

nbody.o: nbody.cpp float4.hpp
	$(CXX) $(CFLAGS) $(INCLUDE) nbody.cpp -c 
 
simulation.o: simulation.cpp float4.hpp
	$(CXX) $(CFLAGS) $(INCLUDE) simulation.cpp -c 

clean:
	rm -f *.o nbody
