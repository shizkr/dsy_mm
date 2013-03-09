#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "circular_buffer.h"

void circular_buffer_init(struct circular_buffer *cb,
		unsigned int *items, int size)
{
	cb->size = size;
	cb->start = 0;
	cb->end = 0;
	cb->items = items;
}

int circular_buffer_full(struct circular_buffer *cb)
{
	return ((cb->end + 1) % cb->size == cb->start);
}

inline void circular_buffer_read(struct circular_buffer *cb, unsigned int *item)
{
	*item = cb->items[cb->start];
	cb->start = (cb->start + 1) % cb->size;
}

inline void circular_buffer_write(struct circular_buffer *cb, unsigned  int item)
{
	cb->items[cb->end] = item;
	cb->end = (cb->end + 1) % cb->size;
	if (cb->end == cb->start) {
		printf("%s: circular buffer over-written.\n", __func__);
		cb->start = (cb->start + 1) % cb->size;
		exit(1);
	}
}

int circular_buffer_empty(struct circular_buffer *cb)
{
	return cb->end == cb->start;
}
