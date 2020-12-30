CC=g++
CFLAGS=-c -Wall -Werror
LDFLAGS=-std=c99
CLIENT_SOURCES=client.cpp
CLIENT_OBJECTS=$(CLIENT_SOURCES:.cpp=.o)
CLIENT_EXECUTABLE=udp_client

SERVER_SOURCES=server.cpp
SERVER_OBJECTS=$(SERVER_SOURCES:.cpp=.o)
SERVER_EXECUTABLE=udp_server

build-client: $(CLIENT_SOURCES) $(CLIENT_EXECUTABLE)

$(CLIENT_EXECUTABLE): $(CLIENT_OBJECTS) 
	$(CC) $(LDFLAGS) $(CLIENT_OBJECTS) -o $@

build-server: $(SERVER_SOURCES) $(SERVER_EXECUTABLE)

$(SERVER_EXECUTABLE): $(SERVER_OBJECTS) 
	$(CC) $(LDFLAGS) $(SERVER_OBJECTS) -o $@
	
.cpp.o:
	$(CC) $(CFLAGS) $< -o $@

all:
	make build-client && make build-server
	
clean:
	rm -rf *.o $(CLIENT_EXECUTABLE) $(SERVER_EXECUTABLE)
