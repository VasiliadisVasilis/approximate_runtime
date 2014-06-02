CC=gcc
CFLAGS=-c -Wall -g
LDFLAGS= -lpthread  -lrt
SOURCES=main.c list.c group.c task.c coordinator.c  accelerator.c
OBJECTS=$(SOURCES:.c=.o)
EXECUTABLE=debug

all: $(SOURCES) $(EXECUTABLE)
	
$(EXECUTABLE): $(OBJECTS) 
	$(CC) $(OBJECTS) -o $@ $(LDFLAGS) 

.c.o:
	$(CC) $(LDFLAGS) $(CFLAGS) $< -o $@
	
clean:
	rm -rf *.o $(EXECUTABLE)