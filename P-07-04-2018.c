__asm volatile("syscall\n\t" : "=rax"(res) : "rax"(syscall_num), "rdi"(a1), "rsi"(a2))

Makefile
hello-s: hello-s.o
	$(LD) $< -o 

.a //расширение для статических библиотек
ar //для работы с архивами

RTD_LAZY 
