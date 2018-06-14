#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/ip.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <time.h>

#define MAX_SIZE_BUFFER 64 * 1024
#define MAX_SIZE_CONNECTION 1024 * 64

struct buf_t* buf; 
char str[MAX_SIZE_BUFFER + 1];

int startServer(char home []) {
    int sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (sock == -1)
        return -1;

    struct sockaddr_in server_addr = {
        .sin_family = AF_INET,
        .sin_port = htons(8080), 
        .sin_addr = {.s_addr = inet_addr(home)}};

    if (bind(sock, (struct sockaddr*)&server_addr, sizeof(server_addr)) == -1)
        return -1;

    if (listen(sock, 100) == -1)
        return -1;

    return sock;
}


int main(int argc, char* argv[]) {
    if (argc != 2) {
        printf("Usage!\n");
        return -1;
    }

    int sock = startServer(argv[1]);
    int n_connections = 0;
    int connection;
    struct sockaddr_in client_addr;
    int len; 
    while (1) {
        if ((connection = accept(sock, (struct sockaddr *)&client_addr, &len)) < 0) {
            return -1;
        }
        n_connections++;
        char buffer[1024];
        time_t tick = time(NULL);
        sprintf(buffer, "connection is %d\n time: %.24s\n", n_connections, ctime(&tick));
        send(connection, buffer, 1025, 0);
        close(connection);
    }

    close(sock);

    return 0;
}