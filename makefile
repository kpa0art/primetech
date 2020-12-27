CC=g++
CFLAGS=-c -Wall
LDFLAGS=
SOURCES=parameters.cpp
OBJECTS=$(SOURCES:.cpp=.o)
EXECUTABLE=params

all: $(SOURCES) $(EXECUTABLE)
	
$(EXECUTABLE): $(OBJECTS) 
	$(CC) $(LDFLAGS) $(OBJECTS) -o $@
	
run:
	./$(EXECUTABLE)

.cpp.o:
	$(CC) $(CFLAGS) $< -o $@
	
clean:
	rm -rf *.o $(EXECUTABLE)
