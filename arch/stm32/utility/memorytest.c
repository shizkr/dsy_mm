#include <stdio.h>
#include <stdlib.h>

/* appropriate definition of _heap and _heap_end has to be done in the linker
 * script*/
extern int _heap;
extern int _heap_end;
int free_memory;
int memory_alloc_test(void)
{
	int *buf = NULL;

	//buf = malloc(4096);
	if (!buf)
		return -1;

	//free(buf);

	return 0;
}

