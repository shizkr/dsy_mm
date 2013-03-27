#include <stdio.h>
#include <stdlib.h>

/* Below function is test only. */
int memory_alloc_test(void)
{
	int *buf = NULL;
	int size = 0;

	while (1) {
		buf = malloc(1024);
		if (!buf) {
			printf("FREE MEM SIZE:%d\n", size);
			return -1;
		}

		size += 1024;
	}

	return 0;
}

