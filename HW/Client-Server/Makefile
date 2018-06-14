CC = gcc

all: server client

server: 
	$(CC) server.c -o server

client: 
	$(CC) -std=c11 client.c -o client

clean:
	$(MAKE) -C ../lib clean
	rm -f server
	rm -f client