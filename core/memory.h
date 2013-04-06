#ifndef MEMORY_H
#define MEMORY_H

#define MAX_BT_NODE_SIZE   512

#ifdef DEBUG_MEMORY
#define mmalloc(x) malloc_debug(x, __func__, __LINE__)
#define mfree(x) free_debug(x, __func__, __LINE__)
#else
#define mmalloc(x) malloc(x)
#define mfree(x) free(x)
#endif

struct circular_buffer
{
	int size;
	int start;
	int end;
	unsigned int *items;
};

/*
 * bt_node description
 * dir: direction of left and right child tree	(0~3)
 *      bbbb bbbb: upper 4 bit for left tree direction
 *                 lower 4 bit for right tree direction
 * when left or right pointer has child node, dir variable
 * will have the mouse direction to go to next node.
 */
struct btree_node {
	unsigned char pos;		/* maze map position */
	unsigned char dir;		/* direction of left and right child tree */
	unsigned char abs_dir;	/* current mouse absolute direction */
	struct btree_node *parent;
	struct btree_node *left;
	struct btree_node *right;
	int time;
#ifdef DEBUG
	int node_num;
#endif
};

struct s_link {
	struct btree_node *bt_node;
	struct s_link *node;
};

struct btree_node *bt_node_alloc(unsigned char pos, unsigned char abs_dir);
void bt_node_free(struct btree_node *node);
int add_bt_node(struct btree_node *node, struct btree_node *new_node);
void free_bt_node_list(struct btree_node *head);

struct s_link *s_link_alloc(struct btree_node *bt_node);
void add_sl_node(struct s_link **list, struct s_link *node);
void sl_node_free(struct s_link *head);
void debug_sl_node(struct s_link *list);

void circular_buffer_init(struct circular_buffer *cb, unsigned int *items, int size);
int circular_buffer_full(struct circular_buffer *cb);
inline void circular_buffer_read(struct circular_buffer *cb, unsigned int *item);
inline void circular_buffer_write(struct circular_buffer *cb, unsigned int item);
int circular_buffer_empty(struct circular_buffer *cb);

#ifdef DEBUG_MEMORY
void *malloc_debug(size_t size, const char *func, int line);
void free_debug(void *ptr, const char *func, int line);
void dump_alloc_memory_info(void);
#endif
#endif
