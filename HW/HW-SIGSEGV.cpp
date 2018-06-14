#ifndef SIGSEGV_H
#define SIGSEGV_H

#include <string>
#include <iostream>
#include <signal.h>
#include <errno.h>
#include <stdlib.h>
#include <malloc.h>
#include <unistd.h>
#include <sys/mman.h>
#include <stdio.h>
#include <setjmp.h>
#include <ucontext.h>
// #include "sigsegv_dump.h"

class sigsegv_dump {
public:
    const int range_of_memory = 4;

    void* address_fault;
    ucontext_t context;

    void print();

    static int fd[2];
    static jmp_buf env;

    static void handler_sigsegv(int sig, siginfo_t *si, void* unused);

    static void handler_sigsegv_check(int sig, siginfo_t *si, void* unused);

    static void init();

private:
    int print_byte(uint8_t *ptr, int offset);

    void print_st(_libc_fpxreg st, size_t index);
};

int sigsegv_dump::fd[2];
jmp_buf sigsegv_dump::env;

#define HANDLER_ERROR(msg) \
    perror(msg); \
    exit(EXIT_FAILURE);

using std::cout;
using std::endl;

void sigsegv_dump::init() {
    struct sigaction act;

    act.sa_flags = SA_SIGINFO;
    sigemptyset(&act.sa_mask);
    act.sa_sigaction = handler_sigsegv;
    if (sigaction(SIGSEGV, &act, NULL) == -1) {
        HANDLER_ERROR("sigaction");
    }

    pipe(fd);

    if(sigsetjmp(env, SIGSEGV) != 0) {
        // handling signal
        sigsegv_dump dump;
        if(read(fd[0], reinterpret_cast<void *>(&dump), sizeof(sigsegv_dump)) == 0) {
            cout << "dump broken\n";
            exit(EXIT_FAILURE);
        }
        dump.print();
        close(fd[0]);
        exit(EXIT_FAILURE);
    }
}

void sigsegv_dump::handler_sigsegv_check(int, siginfo_t *, void*) {
    siglongjmp(env, 1);
}

void sigsegv_dump::handler_sigsegv(int, siginfo_t *si, void*) {
    // write to pipe log
    sigsegv_dump dump;

    dump.address_fault = si->si_addr;

    if(getcontext(&(dump.context))) {
        HANDLER_ERROR("getcontext");
    }

    write(fd[1], reinterpret_cast<void *>(&dump), sizeof(sigsegv_dump));

    close(fd[1]);

    siglongjmp(env, 1);
}

void sigsegv_dump::print() {

    cout << "catch SIGSEGV\n";
    cout << "================================\n";
    cout.flags(std::ios::hex | std::ios::showbase);
    cout << "address fault: " << reinterpret_cast<size_t>(address_fault) << "\n";


    cout << "memory: ";
    int j = 8; // len("memory: ")
    uint8_t *byte_ptr = reinterpret_cast<uint8_t*>(address_fault);
    for(int i = range_of_memory; i > 0; --i) {
        j += print_byte(byte_ptr, -i);
    }
    print_byte(byte_ptr, 0);
    for(int i = 0; i < range_of_memory; ++i) {
        print_byte(byte_ptr, i);
    }
    cout << "\n";
    for(int i = 0; i < j; ++i) {
        cout << " ";
    }
    cout << "^\n";

    cout << "dump register:\n";
    cout << "rax: " << context.uc_mcontext.gregs[REG_RAX] << "\n";
    cout << "rbx: " << context.uc_mcontext.gregs[REG_RBX] << "\n";
    cout << "rcx: " << context.uc_mcontext.gregs[REG_RCX] << "\n";
    cout << "rdx: " << context.uc_mcontext.gregs[REG_RDX] << "\n";
    cout << "rsi: " << context.uc_mcontext.gregs[REG_RSI] << "\n";
    cout << "rdi: " << context.uc_mcontext.gregs[REG_RDI] << "\n";
    cout << "rbp: " << context.uc_mcontext.gregs[REG_RBP] << "\n";
    cout << "rsp: " << context.uc_mcontext.gregs[REG_RSP] << "\n";
    cout << "r8: " << context.uc_mcontext.gregs[REG_R8] << "\n";
    cout << "r9: " << context.uc_mcontext.gregs[REG_R9] << "\n";
    cout << "r10: " << context.uc_mcontext.gregs[REG_R10] << "\n";
    cout << "r11: " << context.uc_mcontext.gregs[REG_R11] << "\n";
    cout << "r12: " << context.uc_mcontext.gregs[REG_R12] << "\n";
    cout << "r13: " << context.uc_mcontext.gregs[REG_R13] << "\n";
    cout << "r14: " << context.uc_mcontext.gregs[REG_R14] << "\n";
    cout << "r15: " << context.uc_mcontext.gregs[REG_R15] << "\n";
    cout << "rip: " << context.uc_mcontext.gregs[REG_RIP] << "\n";
    cout << "flags: " << context.uc_mcontext.gregs[REG_EFL] << "\n";

    auto st = context.__fpregs_mem._st;
    for(size_t i = 0; i < 8; ++i) {
        print_st(st[0], i);
    }

    cout << endl;
}

void sigsegv_dump::print_st(_libc_fpxreg st, size_t index) {
    cout.flags(std::ios::dec | std::ios::showbase);
    cout << "st[" << index << "]:\n"
         << "  exp: " << st.exponent << "\n";
    cout.flags(std::ios::hex);
    cout << "  significand: "   << st.significand[0] << st.significand[1] << st.significand[2] << st.significand[3] << "\n";
}

int sigsegv_dump::print_byte(uint8_t *ptr, int offset) {
    struct sigaction act;
    act.sa_flags = SA_SIGINFO;
    sigemptyset(&act.sa_mask);
    act.sa_sigaction = handler_sigsegv_check;
    if (sigaction(SIGSEGV, &act, NULL) == -1) {
        HANDLER_ERROR("sigaction");
    }

    if (offset < 0 && reinterpret_cast<size_t>(ptr) < static_cast<size_t>(-offset)) {
        return 0;
    } else {
        ptr += offset;
        if(sigsetjmp(env, SIGSEGV) == 0) {
            cout.width(2);
            cout.flags(std::ios::hex);
            cout << static_cast<int>(*ptr) << " ";
            return 3;
        } else {
            cout.width(2);
            cout.flags(std::ios::dec);
            cout << -1 << " ";
            return 3;
        }
    }
}

int main() {
    sigsegv_dump::init();

    int *ptr = (int*)(&main);
    ptr[0] = 0;

    return 0;
}