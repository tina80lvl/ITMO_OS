#include <stdio.h>
char* dyn_lib(int a, int b, char* str) {
	sprintf(str, "Dynamic library: %d - %d = %d", a, b, a - b);
	return str;
}