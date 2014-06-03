CC=gcc
CFLAGS=-c -Wall -g
LDFLAGS= -lpthread  -lrt -fPIC
SOURCES=list.c group.c task.c coordinator.c  accelerator.c
OBJECTS=$(SOURCES:.c=.o)
EXECUTABLE=librtsrel.so

all: $(SOURCES) $(EXECUTABLE)
	
$(EXECUTABLE): $(OBJECTS) 
	$(CC) -shared $(OBJECTS) -o $@ $(LDFLAGS) 

.c.o:
	$(CC) $(LDFLAGS) $(CFLAGS) $< -o $@
	
clean:
	rm -rf *.o $(EXECUTABLE)