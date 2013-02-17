#ifndef BIN_TREE_H
#define BIN_TREE_H

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
	struct btree_node *parent;
	struct btree_node *left;
	struct btree_node *right;
};

struct s_link {
	struct btree_node *bt_node;
	struct s_link *node;
};

struct btree_node *bt_node_alloc(unsigned char pos);
void add_bt_node(struct btree_node *node, struct btree_node *new_node);
struct s_link *s_link_alloc(struct btree_node *bt_node);
void add_sl_node(struct s_link **list, struct s_link *node);
void sl_node_free(struct s_link *head);
void debug_sl_node(struct s_link *list);
#endif
