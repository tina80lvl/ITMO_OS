CC = gcc

all: server client

server: libbufio.so
	$(CC) server.c -L../lib -I../lib -Wl,-rpath=../lib -lbufio -o server

client: libbufio.so
	$(CC) -std=c11 client.c -L../lib -I../lib -Wl,-rpath=../lib -lbufio -o client

libbufio.so: ../lib
	$(MAKE) -C $<
clean:
	$(MAKE) -C ../lib clean
	rm -f server
	rm -f client