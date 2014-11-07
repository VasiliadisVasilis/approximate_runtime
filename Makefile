#make "CFLAGS=-DDUAL_TASKS -DENABLE_CONTEXT -DENABLE_SIGNALS -DDOUBLE_QUEUES" for a scorpio like runtime
#make "CFLAGS=-DGEMFI" to enable fault injection to tasks
#make "CFLAGS=-DGEMFI -DDUAL_TASKS" injects faults only to non-reliable tasks
CC=gcc
AR=ar
override CFLAGS+= --static -c -Wall -g -I include/ -O3 #-DDEBUG 
LDFLAGS= -lpthread -lOpenCL -ldl -lrt -fPIC -lm5
SOURCES=list.c group.c task.c coordinator.c  accelerator.c opencl_nvidia_wrapper.c
INCLUDE=-I./fi
LIB=-L./fi
OBJECTS=$(SOURCES:.c=.o)
EXECUTABLE=librtsrel.a

all: $(SOURCES) $(EXECUTABLE)
	
$(EXECUTABLE): $(OBJECTS) 
	$(AR)  rcs $(EXECUTABLE) $(OBJECTS)

.c.o:
	$(CC) $(INCLUDE) $(LIB) $(LDFLAGS) $(CFLAGS) $(FLAGS)  $< -o $@
	
clean:
	rm -rf *.o $(EXECUTABLE)
	
help:
	echo "make -FLAGS=\"-DDEPENDENCIES\" to compile with dependencies or if you want them disable just write make"
