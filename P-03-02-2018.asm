//линковка
subl
man gcc
gcc -help
gcc -o hello main.o hello.o
./hello
strace ./hello
syscall table //write
man syscalls

//c
pid_t pid = fork();
if (pid == 0) {
	printf("child process");
}
else if (pid > 0){
	printf("parent process");
	printf("child pid: %d", pid);
}
else {
	printf("for failed");
}

execve();
waitpid();


//hello.s
//assembler
.LFBO
	mov $1 %rax
	mov $1 %rdi
	lea message 

//HW
терминал примитивный
