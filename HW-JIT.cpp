#include <iostream>
#include <cstdio>
#include <vector>
#include <cstring>
#include <sys/mman.h>
#include <sstream>

#define SIZE 4096

unsigned char code[] = {
        0x48, 0x89, 0xf8,
        0x48, 0x83, 0xc0, 0x04,
        0xc3,
};

typedef char (*JIT_func)(char);

void *alloc_writable_memory(size_t size) {
    void *ptr = mmap(0, size,
                     PROT_READ | PROT_WRITE,
                     MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    if (ptr == MAP_FAILED) {
        perror("ERROR. Can't allocate by mmap failed");
        return NULL;
    }
    return ptr;
}

char free_memory(void *ptr, size_t size) {
    if (munmap(ptr, size) == -1) {
        perror("ERROR. Can't deallocate by munmap");
        return -1;
    }
    return 0;
}

char switch_memory_type(void *ptr, size_t size, int prot) {
    if (mprotect(ptr, size, PROT_READ | prot) == -1) {
        perror("ERROR. Can't switch PROT by mprotect");
        return -1;
    }
    return 0;
}

void emit_code_into_memory(void *memory) {
    memcpy(memory, code, sizeof(code));
}

char execute_function(void *memory, char value) {
    auto function = (JIT_func) memory;
    switch_memory_type(memory, SIZE, PROT_EXEC);
    char result = function(value);
    switch_memory_type(memory, SIZE, PROT_WRITE);
    return result;
}

void runtime_change_code(void *memory, char x) {
    char *to_change = (char *) ((char *)memory + sizeof(code) - 2); 
    memcpy(to_change, &x, 1);
}

std::pair<char, char> get_parameters(std::string &command) {
    std::istringstream iss(command);
    int x, y;
    iss >> x >> y;
    return {x, y}; 
}

int main() {
    void *memory = alloc_writable_memory(SIZE);
    emit_code_into_memory(memory);

    std::string command;
        std::cout << "HW-JIT $ ";
    while (std::getline(std::cin, command)) {
        if (command == "exit")
            break;
        auto parameters = get_parameters(command);
        char x = parameters.first;
        char y = parameters.second;

        runtime_change_code(memory, x);
        char result = execute_function(memory, y);
        printf("result = %d\n", result);
        std::cout << "HW-JIT $ ";
    }

    free_memory(memory, SIZE);

    return 0;
}