#include <stdio.h>
#include <stdlib.h>

int main(void)
{
	void *ptr = malloc(sizeof(int));
	char *c_ptr;
	// sizeof doesn't evaluate it's arguments
	printf("void*: %p\t sizeof(void*): %zu\t sizeof(*(void*)): %zu\n", ptr, sizeof(ptr), sizeof(*ptr));

	// printf("Access void pointer: %d\n", *ptr); error

	/*
	 * This is fine in GCC!! It allows this as a compiler extension.
	 * As a consequence sizeof(void) is alos allowed.
	 */
	ptr++; 

	printf("void*: %p\t sizeof(void*): %zu\t sizeof(*(void*)): %zu\n", ptr, sizeof(ptr), sizeof(*ptr));
	printf("char*: %p\t sizeof(char*): %zu\t sizeof(*(char*)): %zu\n", c_ptr, sizeof(c_ptr), sizeof(*c_ptr));
}