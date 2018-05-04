#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <sys/wait.h>
#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <unistd.h>
#include <string>

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

class PatternMatcher {
private:
    int inum;
    int nlink;
    off_t size;
    int typeSize;
    char *func{""};
    std::string name = {};

    bool compare(off_t current_size) {
        if (typeSize == 0 && size == current_size) {
            return true;
        }
        if (typeSize == 1 && size < current_size) {
            return true;
        }
        return typeSize == -1 && size > current_size;
    }


public:
    PatternMatcher() {
        inum = -1;
        nlink = -1;
        size = static_cast<off_t>(-1);
        typeSize = 0;
        name = ".";
    }

    char *get_func() {
        return func;
    }

    void set_inum(int value) {
        inum = value;
    }

    void set_nlink(int value) {
        nlink = value;
    }

    void set_size(off_t value) {
        size = value;
    }

    void set_typeSize(int value) {
        typeSize = value;
    }

    void set_exec(char *value) {
        func = value;
    }

    void set_name(char *value) {
        name = std::string(value);
    }

    bool check_pattern(ino_t current_inum,
                       nlink_t current_nlink,
                       off_t current_size,
                       std::string current_name) {

        if (name != "." && name != current_name) {
            return false;
        }
        if (inum != -1 && inum != current_inum) {
            return false;
        }

        if (nlink != -1 && nlink != current_nlink) {
            return false;
        }
        return !(size != -1 && !compare(current_size));

    }

};

auto pattern = PatternMatcher();

void exec(const char *pathName) {
    if (strcmp(pattern.get_func(), "") == 0) {
        printf("%s\n", pathName);
        return;
    }
    char *argv[3] = {pattern.get_func(), (char *) pathName, '\0'};
    execute(argv);
}

void visiting(const char *nameDir) {
    DIR *dir = opendir(nameDir);
    if (dir == NULL) {
        perror("ERROR. When opening directory");
        return;
    }
    struct dirent *entry = NULL;

    while ((entry = readdir(dir))) {
        struct stat info{};
        if (strcmp(entry->d_name, ".") == 0 ||
            strcmp(entry->d_name, "..") == 0) {
            continue;
        }
        std::string path_to_execute = std::string(nameDir);
        path_to_execute.append("/");
        path_to_execute.append(entry->d_name);
        if (stat(path_to_execute.data(), &info) == 0) {
            if (S_ISDIR(info.st_mode)) {
                visiting(path_to_execute.data());
            } else if (S_ISREG(info.st_mode)) {
                if (pattern.check_pattern(info.st_ino,
                                          info.st_nlink,
                                          info.st_size,
                                          std::string(entry->d_name))) {
                    exec(path_to_execute.data());
                }
            }
        } else {
            perror("ERROR. Can't read statistic");
        }
    }
    closedir(dir);
}

void initPattern(int argc, char *argv[]) {
    for (int i = 2; i < argc; i += 2) {
        if (i + 1 == argc) {
            fprintf(stderr, "ERROR. Excepted more arguments.");
            exit(1);
        }

        if (strcmp("-inum", argv[i]) == 0) {
            pattern.set_inum(atol(argv[i + 1]));
        } else if (strcmp("-name", argv[i]) == 0) {
            pattern.set_name(argv[i + 1]);
        } else if (strcmp("-size", argv[i]) == 0) {
            switch (argv[i + 1][0]) {
                case '-':
                    pattern.set_typeSize(-1);
                    break;
                case '+':
                    pattern.set_typeSize(1);
                    break;
                case '=':
                    pattern.set_typeSize(0);
                    break;
                default:
                    break;
            }
            pattern.set_size(atol(argv[i + 1] + 1));
        } else if (strcmp("-nlinks", argv[i]) == 0) {
            pattern.set_nlink(atoi(argv[i + 1]));
        } else if (strcmp("-exec", argv[i]) == 0) {
            pattern.set_exec(argv[i + 1]);
        } else {
            fprintf(stderr, "ERROR. Unexpected token -%s", argv[i]);
            exit(1);
        }
    }
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "ERROR. Excepted argument `path` but nothing given.");
        return 1;
    }
    initPattern(argc, argv);
    visiting(argv[1]);
    return 0;
}