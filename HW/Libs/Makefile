all: link.o stat_lib.a dyn_lib.so dyn_linked_lib.so
	gcc -fsanitize=address -Wall -o link link.o -L. stat_lib.a -ldl dyn_lib.so -Wl,-rpath,.

link.o: link.c
	gcc -c link.c

stat_lib.a: stat_lib.o
	ar cr stat_lib.a stat_lib.o

stat_lib.o: stat_lib.c
	gcc -c stat_lib.c

dyn_lib.so: dyn_lib.o
	gcc -shared -o dyn_lib.so dyn_lib.o

dyn_lib.o: dyn_lib.c
	gcc -c -fPIC dyn_lib.c

dyn_linked_lib.so: dyn_linked_lib.o
	gcc -shared -o dyn_linked_lib.so dyn_linked_lib.o

dyn_linked_lib.o: dyn_linked_lib.c
	gcc -c -fPIC dyn_linked_lib.c

clean:
	-rm -f *.o *.so *.a