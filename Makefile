CC=gcc
AR=ar
CFLAGS=-c -Wall -g -DDEBUG -I include/ -O3
LDFLAGS= -lpthread -lOpenCL -ldl -lrt -fPIC 
SOURCES=list.c group.c task.c coordinator.c  accelerator.c opencl_nvidia_wrapper.c
OBJECTS=$(SOURCES:.c=.o)
EXECUTABLE=librtsrel.a

all: $(SOURCES) $(EXECUTABLE)
	
$(EXECUTABLE): $(OBJECTS) 
	$(AR)  rcs $(EXECUTABLE) $(OBJECTS)

.c.o:
	$(CC) $(LDFLAGS) $(CFLAGS) $(FLAGS)  $< -o $@
	
clean:
	rm -rf *.o $(EXECUTABLE)
	
help:
	echo "make -FLAGS=\"-DDEPENDENCIES\" to compile with dependencies or if you want them disable just write make"
