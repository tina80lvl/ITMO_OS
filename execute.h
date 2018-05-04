#pragma once

#include <cstdio>
#include <zconf.h>
#include <cstdlib>
#include <wait.h>

void execute(char **argv) {
    pid_t pid;
    int status;
    pid = fork();
    if (pid == 0) {
        if (execvp(argv[0], argv) < 0) {
            perror("ERROR. Execute command failed");
            exit(-1);
        }
    } else if (pid > 0) {
        if (waitpid(pid, &status, 0) == -1) {
            perror("ERROR. Caught error in execution");
        };
    } else {
        perror("ERROR. Fork failed");
    }
}