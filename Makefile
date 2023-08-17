
CC = g++ -Wno-write-strings
SERVER_FILE = simple_server.cpp
HTTP_SERVER_FILE = http_server.cpp
LOAD_GEN_FILE = load_gen.cpp


all: server
all: load_gen

server: $(SERVER_FILE) $(HTTP_SERVER_FILE)
	$(CC) $(SERVER_FILE) $(HTTP_SERVER_FILE) -o server
load_gen: $(LOAD_GEN_FILE)
	$(CC) $(LOAD_GEN_FILE) -pthread -o load_gen

clean:
	rm -f server
	rm -f load_gen server *.log
