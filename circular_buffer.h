#ifndef CIRCULAR_BUFFER_H
#define CIRCULAR_BUFFER_H
struct circular_buffer
{
	int size;
	int start;
	int end;
	int *items;
};

void circular_buffer_init(struct circular_buffer *cb, int *items, int size);
int circular_buffer_full(struct circular_buffer *cb);
inline void circular_buffer_read(struct circular_buffer *cb, int *item);
inline void circular_buffer_write(struct circular_buffer *cb, int item);
int circular_buffer_empty(struct circular_buffer *cb);
#endif
