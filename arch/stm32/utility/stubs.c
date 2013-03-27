#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

//these are just dummies except _sbrk which is essential for malloc

#if defined(__cplusplus) || defined(c_plusplus)
extern "C" {
#endif
__attribute__((weak)) void _kill(void) {}
__attribute__((weak)) void _getpid(void) {}
__attribute__((weak)) void _fstat(void) {}
__attribute__((weak)) void _lseek(void) {}
__attribute__((weak)) int _isatty(int i) {return 0;}
__attribute__((weak)) int _open(const char *pathname, int flags, mode_t mode) {return -1;}
__attribute__((weak)) void _close(void) {}
__attribute__((weak)) void _exit(int i) {}

/*appropriate definition of _heap and _heap_end has to be done in the linker script*/ 
extern int _heap;
extern int _heap_end;

caddr_t  _sbrk ( int incr )
{
static unsigned char *heap = NULL;
void *prev_heap;
void *heap_end = &_heap_end;

    if (heap == NULL)  prev_heap = heap = (unsigned char *)&_heap; 

    if ((prev_heap + incr) < heap_end)
    	{
      	prev_heap = heap;
      	heap += incr;
    	}
    else return NULL;
return (caddr_t) prev_heap;
}

#pragma weak _write
ssize_t _write(int fd, const void *buf, size_t count) {return -1;} 

//#pragma weak _read
 __attribute__((weak))
ssize_t _read(int fd, const void *buf, size_t count) {return -1;}


#if defined(__cplusplus) || defined(c_plusplus)
}
#endif
