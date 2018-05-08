#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/ip.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <errno.h>
#include "../lib/bufio.h"
#include <string.h>

int startServer(int port) {
    int sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (sock == -1)
        return -1;
	int one = 1;
	if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &one, sizeof(int)) == -1)
        return -1;
	struct sockaddr_in addr = {
		.sin_family = AF_INET,
		.sin_port = htons(port), 
		.sin_addr = {.s_addr = INADDR_ANY}};
	if (bind(sock, (struct sockaddr*)&addr, sizeof(addr)) == -1)
        return -1;
	if (listen(sock, 1) == -1)
        return -1;
    return sock;
}

#define MAX_SIZE_BUFFER 64 * 1024
#define MAX_SIZE_CONNECTION 1024*64

struct buf_t* buf; 
char str[MAX_SIZE_BUFFER + 1];

void pipedWrite(int fd1, int fd2) {
    while (1) {
        ssize_t rb = read(fd1, str, MAX_SIZE_BUFFER);
        if (rb <= 0) break;
        str[rb] = '\n';
        ssize_t bytes = write(fd2, str, rb + 1);
    }
    close(fd1);
    close(fd2);
}
int childs[MAX_SIZE_CONNECTION], numChild;

int main(int argc, char* argv[]) {
    if (argc != 3) {
        printf("Input 3 agruments!\n");
        return 0;
    }
    int port1 = atoi(argv[1]);
    int port2 = atoi(argv[2]);
    int sock1 = startServer(port1);
    int sock2 = startServer(port2);
    buf = buf_new(MAX_SIZE_BUFFER);
    while (1) {
    	struct sockaddr_in client;
    	socklen_t sz = sizeof(client);
    	int fd1 = accept(sock1, (struct sockaddr*)&client, &sz);
    	int fd2 = accept(sock2, (struct sockaddr*)&client, &sz);
        int pipef[2];
        int proc = fork();
        if (proc == 0) {
            close(sock1);
            close(sock2);
            pipedWrite(fd1, fd2);
            return 0;
        } else {
            proc = fork();
            if (proc == 0) {
                close(sock1);
                close(sock2);
                pipedWrite(fd2, fd1);
                return 0;
            } else {
                close(fd1);
                close(fd2);
            }
        }
    }
    close(sock1);
    close(sock2);
    int error = 0;
    while (1) {
        int status = 0;
        pid_t done = wait(&status);
        if (done == -1 && errno == ECHILD) break;
        else if (WEXITSTATUS(status) != 0)
            error = 1;
    }
    printf("error = %d\n", error);
    return 0;
}