#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "memory.h"

#define TAG "MEMORY: "
#include "debug.h"

#ifdef DEBUG
static int debug_flag = DEBUG_S_LINK;
/* DEBUG_BINTREE */
#endif

#ifdef DEBUG
/* simple check of alloc/free for bt_node and s_link */
static int bt_node_cnt;
static int s_link_cnt;
#endif

#ifdef DEBUG_MEMORY
#define MAGIC 0x3412ABCD
struct alloc_head {
	int size;
	int magic;
};

static int memory_alloc_size;
static int max_alloc_size;

void *malloc_debug(size_t size, const char *func, int line)
{
	struct alloc_head *ptr;

	memory_alloc_size += size;
	ptr = malloc(size+sizeof(struct alloc_head));
	ptr->size = size;
	ptr->magic = MAGIC;

	if (max_alloc_size < memory_alloc_size)
		max_alloc_size = memory_alloc_size;

	printf("%s:%d:%s %08X s:%d max:%d\n",
			func, line, __func__,
			(unsigned int)ptr, memory_alloc_size,
			max_alloc_size);
	return (void *)((unsigned int)ptr + sizeof(struct alloc_head));
}

void free_debug(void *ptr, const char *func, int line)
{
	struct alloc_head *head;

	head = (struct alloc_head *)(ptr - sizeof(struct alloc_head));
	if (head->magic != MAGIC) {
		print_exit("%s: pointer %08X is not valid memory!\n",
				__func__, (unsigned int)ptr);
	}

	memory_alloc_size -= head->size;
	printf("%s:%d:%s: %08X s:%d\n",
			func, line, __func__,
			(unsigned int)head, memory_alloc_size);
	free(head);
}
#endif

struct btree_node *bt_node_alloc(unsigned char pos, unsigned char abs_dir)
{
	struct btree_node *node = mmalloc(sizeof(struct btree_node));

	if (node == NULL)
		print_exit("malloc failure!\n");

	node->pos = pos;
	node->abs_dir = abs_dir;
	node->parent = NULL;
	node->left = NULL;
	node->right = NULL;
	node->time = 0;

#ifdef DEBUG
	bt_node_cnt++;
	node->node_num = bt_node_cnt;
	print_dbg(DEBUG_BINTREE, "Allocated bt node %d\n",
			bt_node_cnt);
#endif
	return node;
}

void bt_node_free(struct btree_node *node)
{
	if (node == NULL)
		print_exit("malloc failure!\n");

#ifdef DEBUG
	bt_node_cnt--;
	print_dbg(DEBUG_BINTREE, "Free bt node %d\n",
		bt_node_cnt);
#endif
	mfree(node);
}

/* nagative return means that there is more than 2 child on the contour
 * block. It doesn't matter to find the fastest path and 2 is enough as
 * rarely it has the case with known pathes.
 */
int add_bt_node(struct btree_node *node, struct btree_node *new_node)
{
	if (node->left == NULL) {
		node->left = new_node;
		new_node->parent = node;
	} else if (node->right == NULL) {
		node->right = new_node;
		new_node->parent = node;
	} else {
		print_info("binary tree can only have 2 child node!!!");
		return -1;
	}
	return 0;
}

/* free the generated contour bin tree */
void free_bt_node_list(struct btree_node *head)
{
	struct btree_node *node;
	unsigned int *buffer;
	struct circular_buffer *cb, bt_node_buffer;

	buffer = mmalloc(sizeof(unsigned int) * MAX_BT_NODE_SIZE);
	if (!buffer)
		print_exit("%s: malloc failure.\n", __func__);

	cb = &bt_node_buffer;
	circular_buffer_init(cb, buffer, MAX_BT_NODE_SIZE);
	circular_buffer_write(cb, (unsigned int)head);

	while (!circular_buffer_empty(cb)) {
		circular_buffer_read(cb, (unsigned int *)&node);

		if (node->left)
			circular_buffer_write(cb, (unsigned int)node->left);
		if (node->right)
			circular_buffer_write(cb, (unsigned int)node->right);

		mfree(node);
#ifdef DEBUG
		bt_node_cnt--;
		print_dbg(DEBUG_BINTREE, "Existing node %d\n",
			bt_node_cnt);
#endif
	}

	mfree(buffer);
}

struct s_link *s_link_alloc(struct btree_node *bt_node)
{
	struct s_link *node = mmalloc(sizeof(struct s_link));
	if (node == NULL)
		print_exit("malloc failure!\n");
	node->bt_node = bt_node;
	node->node = NULL;

#ifdef DEBUG
	s_link_cnt++;
	print_dbg(DEBUG_S_LINK, "Allocated s_link node %d\n",
			s_link_cnt);
#endif
	return node;
}

void add_sl_node(struct s_link **list, struct s_link *node)
{
	static struct s_link *tail;

	if (*list == NULL) {
		*list = node;
		tail = node;
	} else {
		tail->node = node;
		tail = node;
	}
}

#ifdef DEBUG
void debug_sl_node(struct s_link *list)
{
	struct s_link *node;

	if (!list) {
		print_exit("%s:list NULL\n", __func__);
		return;
	}

	node = list;
	do {
		print_dbg(DEBUG_S_LINK,
				"node: %08X ->node:0x%08X ->bt_node: %08X" \
				"->pos: %02X\n",
				(unsigned int)node,
				(unsigned int)node->node,
				(unsigned int)node->bt_node,
				node->bt_node->pos);
		node = node->node;
	} while (node);
	print_dbg(DEBUG_S_LINK, "Verification is done\n");
}
#else
void debug_sl_node(struct s_link *list) { }
#endif

void sl_node_free(struct s_link *head)
{
	struct s_link *node;

	while (head) {
		node = head->node;
		mfree(head);
#ifdef DEBUG
		s_link_cnt--;
#endif
		head = node;
	}
	print_dbg(DEBUG_S_LINK, "Existing s_link node %d\n",
			s_link_cnt);
}

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
