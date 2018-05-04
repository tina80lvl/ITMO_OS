#include <stdio.h>
char* dyn_linked_lib(int a, int b, char* str) {
	sprintf(str, "Dynamically linked library: %d * %d = %d", a, b, a * b);
	return str;
}