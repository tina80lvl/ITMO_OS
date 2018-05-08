#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/ip.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <errno.h>
#include "../lib/bufio.h"
#include <string.h>
#include <poll.h>

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

#define MAX_SIZE_BUFFER 4096
#define MAX_CONNECTION 256

struct pollfd pollf[MAX_CONNECTION];
struct buf_t* buffers[MAX_CONNECTION];
int numConnection;
int closed[MAX_CONNECTION];

void myAccept(int sock, int* fd) {
    struct sockaddr_in client;
    socklen_t sz = sizeof(client);
    *fd = accept(pollf[sock].fd, (struct sockaddr*)&client, &sz);
    pollf[sock].events = 0;
    pollf[sock^1].events = POLLIN;
}

#define swap(type, i, j) {type t = i; i = j; j = t;}//zbs, nah

void checkClose(int i) {
    if (closed[i] && closed[i + 1]) {
        close(pollf[i].fd);
        close(pollf[i + 1].fd);
        swap(struct buf_t*, buffers[i], buffers[numConnection - 2]);
        swap(struct buf_t*, buffers[i + 1], buffers[numConnection - 1]);
        swap(struct pollfd, pollf[i], pollf[numConnection - 2]);
        swap(struct pollfd, pollf[i + 1], pollf[numConnection - 1]);
        closed[i] = closed[i + 1] = 0;
        swap(int, closed[i], closed[numConnection - 2]);
        swap(int, closed[i + 1], closed[numConnection - 1]);
        numConnection -= 2;
        pollf[0].events = POLLIN;
        pollf[1].events = POLLIN;
    }
}

int main(int argc, char* argv[]) {
    if (argc != 3) {
        printf("Input 3 agruments!\n");
        return 0;
    }
    int port1 = atoi(argv[1]);
    int port2 = atoi(argv[2]);
    int sock1 = startServer(port1);
    int sock2 = startServer(port2);
    pollf[0].fd = sock1;
    pollf[0].events = POLLIN;
    pollf[1].fd = sock2;
    pollf[1].events = 0;
    numConnection = 2;
    int fd1 = -1, fd2 = -1;
    while (1) {
        poll(pollf, numConnection, -1);
        for (int i = 0; i < numConnection; ++i)
            if (pollf[i].revents & POLLIN) {
                if (i == 0)
                    myAccept(0, &fd1);
                else if (i == 1)
                    myAccept(1, &fd2);
                else {
                    struct buf_t* buf = buffers[i];
                    int prevSize = buf->size;
                    int bytes = read(pollf[i].fd, buf->data + buf->size, buf->capacity - buf->size);
//                    fprintf(stderr, "read = %d\n", bytes);
                    buf->size += bytes;
                    if (prevSize == buf->size && buf->capacity != prevSize) {
                        shutdown(fd1, SHUT_RD);
                        closed[i] = 1;
                        pollf[i].events = 0;
                        if (i&1) checkClose(i - 1);
                        else checkClose(i);
                    }
                    if (buf->size)
                         pollf[i^1].events = POLLOUT;
                }
            } else if (pollf[i].revents & POLLOUT) {
                int bytes = write(pollf[i].fd, buffers[i^1]->data, buffers[i^1]->size);
//                fprintf(stderr, "byte = %d\n", bytes);
                buffers[i^1]->size -= bytes;
                pollf[i].events = (!closed[i] ? POLLIN : 0);
            } else if (pollf[i].revents & POLLHUP) {
                shutdown(fd1, SHUT_RD);
                closed[i] = 1;
                pollf[i].events = 0;
                if (i&1) checkClose(i - 1);
                else checkClose(i);
            } else if (pollf[i].revents & EINTR) {
                shutdown(fd1, SHUT_RD);
                closed[i] = 1;
                pollf[i].events = 0;
                if (i&1) checkClose(i - 1);
                else checkClose(i);
            }

            if (fd1 != -1 && fd2 != -1) {
                pollf[numConnection].fd = fd1;
                buffers[numConnection] = buf_new(64*1024);
                pollf[numConnection++].events = POLLIN;

                pollf[numConnection].fd = fd2;
                buffers[numConnection] = buf_new(64*1024);
                pollf[numConnection++].events = POLLIN;

                if (numConnection == MAX_CONNECTION) {
                    pollf[0].events = 0;
                    pollf[1].events = 0;
                }
                fd1 = fd2 = -1;
            }
    }
    return 0;
}