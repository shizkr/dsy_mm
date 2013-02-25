#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "bin_tree.h"
#include "circular_buffer.h"

#define TAG "BIN_TREE: "
#include "debug.h"

#ifdef DEBUG
static int debug_flag;
/* = DEBUG_BINTREE; */
#endif

#ifdef DEBUG
/* simple check of alloc/free for bt_node and s_link */
static int bt_node_cnt;
static int s_link_cnt;
#endif

struct btree_node *bt_node_alloc(unsigned char pos, unsigned char abs_dir)
{
	struct btree_node *node = malloc(sizeof(struct btree_node));

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

void add_bt_node(struct btree_node *node, struct btree_node *new_node)
{
	if (node->left == NULL) {
		node->left = new_node;
		new_node->parent = node;
	} else if (node->right == NULL) {
		node->right = new_node;
		new_node->parent = node;
	} else {
		print_exit("binary tree can only have 2 child node!!!");
	}
}

/* free the generated contour bin tree */
void free_bt_node_list(struct btree_node *head)
{
	struct btree_node *node;
	unsigned int *buffer;
	struct circular_buffer *cb, bt_node_buffer;

	buffer = malloc(sizeof(unsigned int) * MAX_BT_NODE_SIZE);
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

		free(node);
#ifdef DEBUG
		bt_node_cnt--;
		print_dbg(DEBUG_BINTREE, "Existing node %d\n",
			bt_node_cnt);
#endif
	}

	free(buffer);
}

struct s_link *s_link_alloc(struct btree_node *bt_node)
{
	struct s_link *node = malloc(sizeof(struct s_link));
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
		free(head);
#ifdef DEBUG
		s_link_cnt--;
#endif
		head = node;
	}
	print_dbg(DEBUG_S_LINK, "Existing s_link node %d\n",
			s_link_cnt);
}
