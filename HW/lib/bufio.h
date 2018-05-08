#ifndef BUFIO_H
#define BUFIO_H
#include <stdlib.h>
#include <sys/types.h>

struct buf_t {
    char* data;
    size_t size;
    size_t capacity;
};
typedef int fd_t;

struct buf_t *buf_new(size_t capacity);
void buf_free(struct buf_t *buf);
size_t buf_capacity(struct buf_t *buf);
size_t buf_size(struct buf_t *);
ssize_t buf_fill(fd_t fd, struct buf_t *buf, size_t required);
ssize_t buf_flush(fd_t fd, struct buf_t *buf, size_t required);
#endif