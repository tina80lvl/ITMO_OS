#include <stdio.h>
char* stat_lib(int a, int b, char* str) {
	sprintf(str, "Static library: %d + %d = %d", a, b, a + b);
	return str;
}