#make "CFLAGS=-DDUAL_TASKS -DENABLE_CONTEXT -DENABLE_SIGNALS -DGEMFI" for a DSN2015
#make "CFLAGS=-DDUAL_TASKS -DENABLE_CONTEXT -DENABLE_SIGNALS -DDOUBLE_QUEUES" for a scorpio like runtime
#make "CFLAGS=-DGEMFI" to enable fault injection to tasks
#make "CFLAGS=-DGEMFI -DDUAL_TASKS" injects faults only to non-reliable tasks
CC=gcc
AR=ar
# -DDEBUG -O3
override CFLAGS+= --static -c -Wall -g -I include/ -pthread -O3
LDFLAGS= -lpthread -ldl -lrt -lm
SOURCES=list.c queue.c group.c task.c coordinator.c  accelerator.c
INCLUDE=-I./fi
LIB=-L./fi
OBJECTS=$(SOURCES:.c=.o)
EXECUTABLE=librtsrel.a

all: $(SOURCES) $(EXECUTABLE)
	
$(EXECUTABLE): $(OBJECTS) 
	$(AR)  rcs $(EXECUTABLE) $(OBJECTS)

.c.o:
	$(CC) $(INCLUDE) $(LIB) $(CFLAGS) $(FLAGS)  $< -o $@ $(LDFLAGS) 
	
clean:
	rm -rf *.o $(EXECUTABLE)
	
help:
	echo "make -FLAGS=\"-DDEPENDENCIES\" to compile with dependencies or if you want them disable just write make"
