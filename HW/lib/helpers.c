#include "helpers.h"
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <string.h>

size_t read_(int fd, void *buf, size_t count) {
    size_t bytesRead = 0;
    while (bytesRead < count) {
        ssize_t actualRead = read(fd, buf + bytesRead, count - bytesRead);
        if (actualRead == 0)
            break;
        if (actualRead == -1 && errno != EAGAIN && errno != EWOULDBLOCK)
            return -1;
        else if (actualRead != -1)
            bytesRead += actualRead;
    }
    return bytesRead;
}

size_t write_(int fd, void *buf, size_t count) {
    size_t bytesWritten = 0;
    while (bytesWritten < count) {
        size_t actualWritten = write(fd, buf + bytesWritten, count - bytesWritten);
        if (actualWritten == -1 && errno != EAGAIN && errno != EWOULDBLOCK)
            return -1;
        else if (actualWritten != -1)
            bytesWritten += actualWritten;
    }
    return bytesWritten;
}    

#define MAX_SIZE 1024
struct CharQueue {
    char data[2 * MAX_SIZE];
    size_t l, r;
};
void ensure(struct CharQueue* q) {
    if (q->l + MAX_SIZE > 2 * MAX_SIZE) {
        size_t i = 0;
        for (; i < q->r - q->l; ++i)
            q->data[i] = q->data[i + q->l];
        q->r -= q->l;
        q->l = 0;
    }
}
size_t push(struct CharQueue* q, void* bufTo, size_t count, char delimiter) {
    if (count == 0) 
        return 0;
    size_t pushed = 0;
    char *bufToChr = (char*)bufTo;
    for (;pushed < count && q->data[q->l + pushed] != delimiter; ++pushed)
        bufToChr[pushed] = q->data[q->l + pushed];
    if (pushed < count) {
        bufToChr[pushed] = q->data[q->l + pushed];
        ++pushed;
    }
    q->l += pushed;
    ensure(q);
    return pushed;
}
size_t size(struct CharQueue* q) {
    return q->r - q->l;
}

size_t read_until(int fd, void * buf, size_t count, char delimiter) {
    if (count == 0)
        return 0;
    static struct CharQueue* q;
    if (q == NULL) {
        q = malloc(sizeof (struct CharQueue));
        q->l = 0;
        q->r = 0;
    }
    size_t ptr = 0;
    int hasDelimiter = 0;
    char *bufToChr = (char*)buf;
    if (size(q) > 0) {
        size_t mn = size(q) < count - ptr ? size(q) : count - ptr;
        size_t pushed = push(q, buf + ptr, mn, delimiter);
        ptr += pushed;
        if (bufToChr[ptr - 1] == delimiter) hasDelimiter = 1;
    }

    while (ptr < count && !hasDelimiter) {
        size_t actualRead = -2;
        if (size(q) != MAX_SIZE) 
            actualRead = read(fd, q->data + q->r, MAX_SIZE - size(q));
        if (actualRead > 0)
            q->r += actualRead;
        if (actualRead == 0 || actualRead == -1 && errno != EAGAIN && errno != EWOULDBLOCK)
            break;
        if (size(q) > 0) {
            size_t mn = size(q) < count - ptr ? size(q) : count - ptr;
            size_t pushed = push(q, buf + ptr, mn, delimiter);
            ptr += pushed;
            if (bufToChr[ptr - 1] == delimiter) hasDelimiter = 1;
        }
    }
    return ptr;
}

int spawn(const char * file, char * const argv []) {
    int proc = fork();
    if (proc != 0) {//in parent process
        int res;
        waitpid(proc, &res, 0);
        return res;
    } else {//in child process
        int fd = open("/dev/null",  O_WRONLY);
        dup2(fd, STDOUT_FILENO);
        dup2(fd, STDERR_FILENO);
        close(fd);
        return execvp(file, argv);
    }
}

struct execargs_t* new_execargs_t_from_string(const char* l, const char* r) {
    int args = 0;
    const char* i = l;
    for (; i < r; ++i) 
        if (*i != ' ') {
            while (i < r && *i != ' ') ++i;
            args++;
        }
    if (args == 0) return NULL;
    const char** a = NULL;
    if (args > 1)
        a = malloc(args - 1);
    const char* file;
    i = l;
    args = 0;
    for (; i < r; ++i)
        if (*i != ' ') {
            const char *j = i;
            while (i < r && *i != ' ') ++i;
            const char *str = strndup(j, i - j);
            if (args == 0) file = str;
            else a[args - 1] = str;
            args++;
        }
    return new_execargs_t(file, a, args - 1);
}

struct execargs_t* new_execargs_t(const char* file,  const char* argv[], int nArgs) {
    struct execargs_t* program;
    program  = malloc(sizeof (struct execargs_t));
    program->file = file;
    program->args = malloc((nArgs + 2) * sizeof(const char*));
    program->args[nArgs + 1] = NULL;
    program->args[0] = file;
    int i = 1;
    for (; i <= nArgs; ++i)
        program->args[i] = argv[i - 1];
    return program;
}

int exec(struct execargs_t* program) {
    int proc = fork();
    if (proc != 0) {//in parent process
        int res;
        waitpid(proc, &res, 0);
        return res;
    } else //in child process
        return execvp(program->file, (char*const*)program->args);
}

int _wasSigInt;
void sigHandler(int sig) {
    _wasSigInt = 1;
}

#define fp fprintf
int runpiped(struct execargs_t** programs, size_t n) {
    _wasSigInt = 0;
    signal(SIGINT, sigHandler);
    int prevOutput = -1;
    //1 - for write
    //0 - for read
    ssize_t i = 0;
    for (; i < n && !_wasSigInt; ++i) {
        int pipefd[2];
        int er = pipe(pipefd);
        if (er == -1) {
            kill(0, SIGINT);
            return -1;
        }
        int childId = fork();
        if (childId == -1) {
            kill(0, SIGINT);
            return -1;
        } 
        if (childId != 0) {//in par proc 
            close(pipefd[1]);
            if (prevOutput != -1) close(prevOutput);
            prevOutput = pipefd[0];
        } else {//in child
            close(pipefd[0]);
            if (i != n - 1) dup2(pipefd[1], STDOUT_FILENO);
            close(pipefd[1]);
            if (prevOutput != -1) {
                dup2(prevOutput, STDIN_FILENO);
                close(prevOutput);
            }
            if (!_wasSigInt) exit(execvp(programs[i]->file, (char*const*)programs[i]->args));
        }
    }
    if (_wasSigInt) return 0;
    int error = 0;
    while (1) {
        int status = 0;
        pid_t done = wait(&status);
        if (done == -1 && errno == ECHILD) break;
        else if (WEXITSTATUS(status) != 0)
            error = 1;
    }
    if (error != 0) return -1;
    return 0;
}