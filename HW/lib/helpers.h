#ifndef HELPERS_H
#define HELPERS_H
#include <stdlib.h>
struct execargs_t {
    const char* file;
    const char** args;
};

ssize_t read_(int fd, void *buf, size_t count);
ssize_t write_(int fd, void *buf, size_t count);
ssize_t read_until(int fd, void * buf, size_t count, char delimiter);
int spawn(const char * file, char * const argv []);
struct execargs_t* new_execargs_t(const char* file, const char* argv[], int nArgs);
struct execargs_t* new_execargs_t_from_string(const char* l, const char* r);

int exec(struct execargs_t* args);
int runpiped(struct execargs_t** programs, size_t n);

#endif