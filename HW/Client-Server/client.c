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

int sock;
char message[1024];
struct sockaddr_in addr;

int connectServer(char home []) {
    int sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (sock == -1)
        return -1;

    struct sockaddr_in server_addr = {
        .sin_family = AF_INET,
        .sin_port = htons(8080), 
        .sin_addr = {.s_addr = inet_addr(home)}};

    if (connect(sock, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        return -1;
    }

    return sock;
}


int main(int argc, char* argv[]) {
    if (argc != 2) {
        printf("Usage!\n");
        return -1;
    }
    memset(message, '0', sizeof(message));
    int sock = connectServer(argv[1]);

    int n = 0, len = 0, maxlen = 1024;
    char buffer[1024];
    char *pbuffer = buffer;
    while ((n = recv(sock, pbuffer, maxlen, 0)) > 0 && maxlen > 0) {
        pbuffer += n;
        maxlen -= n;
        len += n;
        buffer[len] = '\0';
        printf("recived: %s\n", buffer);
    }

    // while (1) {
    //     for (int i = 0; i < 15; i++) {
    //         printf("💜");
    //     }
    //     printf("\n");
    //     for (int i = 0; i < 15; i++) {
    //         printf("💛");
    //     }
    //     printf("\n");
    // }

    close(sock);

    return 0;
}



















