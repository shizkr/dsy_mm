#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "algo.h"
#include "diagonal.h"
#include "memory.h"

#define TAG "DIAGONAL: "
#include "debug.h"

#ifdef DEBUG
static int debug_flag;
/* = DEBUG_DIAGNODE; */
#endif

unsigned int *diagonal_weight_time_table;

/* This structure defines the patterns of
 * diagonal pathes. Totoal 17 patterns including forward
 * pattern can be used to find the diagonal patterns.
 */
static unsigned char short_pattern_table[P_MAX][4] = {
	{ FD, FD, },
	{ FD, LD, RD, },
	{ FD, LD, FD, },     /* -90'  in  */
	{ FD, LD, LD, RD}, /* -135' in  */
	{ FD, LD, LD, FD}, /* -180' in  */
	{ FD, RD, LD, },     /*  45'  out */
	{ FD, RD, FD, },     /*  90'  out */
	{ FD, RD, RD, LD},   /*  135' out */
	{ FD, RD, RD, FD},   /*  180' out */
	{ LD, FD,},          /* -45'  out */
	{ LD, LD, RD,},      /* -90'  in  */
	{ LD, LD, FD,},      /* -135' out */
	{ RD, FD,},          /*  45'  out */
	{ RD, RD, LD,},      /*  90'  in  */
	{ RD, RD, FD,},      /*  135  out */
	{ LD, RD,},          /* One diagonal*/
	{ RD, LD,},
};

static struct diagonal_type pattern_table[P_MAX] = {
	{&short_pattern_table[0][0], 2},
	{&short_pattern_table[1][0], 3},
	{&short_pattern_table[2][0], 3},     /* -90'  in  */
	{&short_pattern_table[3][0], 4}, /* -135' in  */
	{&short_pattern_table[4][0], 4}, /* -180' in  */
	{&short_pattern_table[5][0], 3},     /*  45'  out */
	{&short_pattern_table[6][0], 3},     /*  90'  out */
	{&short_pattern_table[7][0], 4},   /*  135' out */
	{&short_pattern_table[8][0], 4},   /*  180' out */
	{&short_pattern_table[9][0], 2},          /* -45'  out */
	{&short_pattern_table[10][0], 3},      /* -90'  in  */
	{&short_pattern_table[11][0], 3},      /* -135' out */
	{&short_pattern_table[12][0], 2},          /*  45'  out */
	{&short_pattern_table[13][0], 3},      /*  90'  in  */
	{&short_pattern_table[14][0], 3},      /*  135  out */
	{&short_pattern_table[15][0], 2},          /* One diagonal*/
	{&short_pattern_table[16][0], 2},
};

unsigned int default_load_time[SL_MAX] = {
	97, /* SL_F2 */
	118, /* SL_FLR */
	178, /* SL_FLF */
	146, /* SL_FLLR */
	257, /* SL_FLLF */
	118, /* SL_FRL */
	178, /* SL_FRF */
	146, /* SL_FRRL */
	257, /* SL_FRRF */
	118, /* SL_LF */
	111, /* SL_LLR */
	146, /* SL_LLF */
	118, /* SL_RF */
	111, /* SL_RRL */
	146, /* SL_RRF */
	69, /* SL_LR */
	69, /* SL_RL */
	189, /* SL_F3 */
	278, /* SL_F4 */
	363, /* SL_F5 */
	445, /* SL_F6 */
	523, /* SL_F7 */
	600, /* SL_F8 */
	673, /* SL_F9 */
	745, /* SL_F10 */
	815, /* SL_F11 */
	883, /* SL_F12 */
	949, /* SL_F13 */
	1014, /* SL_F14 */
	1077, /* SL_F15 */
	1087, /* SL_F16 */
	136,  /* SL_D2 */
	200,  /* SL_D3 */
	263,  /* SL_D4 */
	324,  /* SL_D5 */
	383,  /* SL_D6 */
	440,  /* SL_D7 */
	497,  /* SL_D8 */
	551,  /* SL_D9 */
	605, /* SL_D10 */
	657, /* SL_D11 */
	708, /* SL_D12 */
	759, /* SL_D13 */
	808, /* SL_D14 */
	856, /* SL_D15 */
	904, /* SL_D16 */
	950, /* SL_D17 */
	996, /* SL_D18 */
	1041, /* SL_D19 */
	1086, /* SL_D20 */
	1130, /* SL_D21 */
	1173, /* SL_D22 */
	1215, /* SL_D23 */
	1257, /* SL_D24 */
	1299, /* SL_D25 */
	1339, /* SL_D26 */
	1380 /* SL_D27 */
};

/* This tree is to find diagonal path pattern.
 * The end node on the branch will have total estimated
 * time. Each node can have maximum 3 nodes besides B(back)
 * node.
 */
static struct diagonal_node *diagonal_node_tree_ptr;

#ifdef DEBUG
static int diag_node_cnt;
#endif

static struct diagonal_node *diag_node_alloc(void)
{
	struct diagonal_node *node =
		mmalloc(sizeof(struct diagonal_node));
	if (node == NULL)
		print_exit("%s: malloc failure!\n", __func__);
	memset(node, 0, sizeof(struct diagonal_node));

#ifdef DEBUG
	diag_node_cnt++;
	print_dbg(DEBUG_DIAGNODE, "Allocated diag node %d:%08X\n",
			diag_node_cnt, (unsigned int)node);
	node->node_num = diag_node_cnt;
#endif
	return node;
}

static void diagonal_pattern_tree_add(struct diagonal_node **list,
	struct diagonal_type *diag_item,
	unsigned int *time_table, int idx)
{
	struct diagonal_node *diag_node, *new_diag_node;
	int i, j;

#ifdef DEBUG
	for (i = 0; i < diag_item->size; i++) {
		print_dbg(DEBUG_DIAGNODE, "%C",
			(diag_item->pttn[i] == FD) ? 'F' : \
			((diag_item->pttn[i] == RD) ? 'R' : \
			((diag_item->pttn[i] == BD) ? 'B' : \
			((diag_item->pttn[i] == LD) ? 'L' : 'X'))));
	}
	print_dbg(DEBUG_DIAGNODE, "req time: %d\n", time_table[idx]);
#endif

	/* Allocate node and add to the list */
	if (*list == NULL) {
		new_diag_node = diag_node_alloc();
		if (new_diag_node == NULL)
			print_exit("%s diag_node_alloc failure!\n", __func__);

		*list = new_diag_node;
	}

	diag_node = *list;
	for (i = 0; i < diag_item->size; i++) {
#ifdef DEBUG
		print_dbg(DEBUG_DIAGNODE, "%C",
				(diag_item->pttn[i] == FD) ? 'F' : \
				((diag_item->pttn[i] == RD) ? 'R' : \
				((diag_item->pttn[i] == BD) ? 'B' : \
				((diag_item->pttn[i] == LD) ? 'L' : 'X'))));
#endif
		new_diag_node = NULL;
		for (j = FD; j <= LD; j++) {
			if (j == BD)
				continue;
			if (diag_item->pttn[i] == j && diag_node->link[j]) {
				diag_node = new_diag_node = diag_node->link[j];
#ifdef DEBUG
				print_dbg(DEBUG_DIAGNODE, "node #%d\n",
				new_diag_node->node_num);
#endif
			}
		}

		/* if pattern doesn't exist, create new node */
		if (new_diag_node == NULL) {
			new_diag_node = diag_node_alloc();
			if (new_diag_node == NULL)
				print_exit("%s diag_node_alloc failure!\n",
						__func__);

			if (i+1 == diag_item->size) {
				diag_node->link[diag_item->pttn[i]] =
					new_diag_node;
				new_diag_node->pttn.time = time_table[idx];
				new_diag_node->pttn.pttn = idx;

				print_dbg(DEBUG_DIAGNODE, "Found pattern,time: %d\n",
				time_table[idx]);

			} else {
				diag_node->link[diag_item->pttn[i]] =
					new_diag_node;
				diag_node = new_diag_node;
			}
		}
	}
}

static int diagonal_pattern_tree_verify(void)
{
	int i, idx;
	struct diag_pttn_time_type *pttn;
	unsigned int time;
	unsigned char pattern[32];

	for (i = 0; i < P_MAX; i++) {
		pttn = diagonal_pattern_search(pattern_table[i].pttn,
				pattern_table[i].size);
		time = pttn->time;
		if (time != 0)
			print_dbg(DEBUG_DIAGNODE, "Found: time: %d\n", time);
		else
			print_exit("Failed to find diagonal path pattern!!!\n");
	}

	/* Search forward path as maximum 16 */
	memset(pattern, FD, sizeof(pattern));
	for (idx = SL_F3, i = 3; idx <= SL_F16;
			idx++, i++) {
		pttn = diagonal_pattern_search(pattern, i);
		time = pttn->time;
		if (time != 0)
			print_dbg(DEBUG_DIAGNODE, "Found: time: %d\n", time);
		else
			print_exit("Failed to find diagonal path pattern!!!\n");
	}

	/* Search diagonal forward path  RLRL... as maximum 27 */
	for (i = 0; i < 32; i++) {
		if (i & 1)
			pattern[i] = LD;
		else
			pattern[i] = RD;
	}
	for (idx = SL_D2, i = 3; idx <= SL_D27;
			idx++, i++) {
		pttn = diagonal_pattern_search(pattern, i);
		time = pttn->time;
		if (time != 0)
			print_dbg(DEBUG_DIAGNODE, "Found: time: %d\n", time);
		else
			print_exit("Failed to find diagonal path pattern!!!\n");
	}

	/* Search diagonal forward path  LRLR... as maximum 27 */
	for (i = 0; i < 32; i++) {
		if (i & 1)
			pattern[i] = RD;
		else
			pattern[i] = LD;
	}
	for (idx = SL_D2, i = 3; idx <= SL_D27;
			idx++, i++) {
		pttn = diagonal_pattern_search(pattern, i);
		time = pttn->time;
		if (time != 0)
			print_dbg(DEBUG_DIAGNODE, "Found: time: %d\n", time);
		else
			print_exit("Failed to find diagonal path pattern!!!\n");
	}
	print_info("%s OK\n", __func__);
	return 0;
}

static int diagonal_pattern_node_remove(struct diagonal_node *node)
{
	int i, j, is_child, is_grandchild;

	/* It's like single list and no way to know the parent node
	 * from child node. So node itself can't be removed.
	 * if input node doesn't have any child nodes it means it's top node.
	 * It'll check child node and remove child node when there is
	 * no grand children nodes.
	 */
	is_child = 0;
	for (i=FD; i<=LD; i++) {
		/* check child nodes */
		if (node->link[i]) {
			is_grandchild = 0;
			for (j=FD; j<=LD; j++) {
				if (node->link[i]->link[j])
					is_grandchild++;
			}

			if (is_grandchild)
				diagonal_pattern_node_remove(node->link[i]);
			else {
				mfree(node->link[i]);
				node->link[i] = NULL;
			}

			is_child++;
		}
	}

	return is_child;
}
void diagonal_pattern_tree_remove(void)
{
	struct diagonal_node *list = diagonal_node_tree_ptr;
	int ret;

	/* Allocate node and add to the list */
	if (list == NULL) {
		print_exit("%s list error!\n", __func__);
	}

	do {
		ret = diagonal_pattern_node_remove(list);
	} while (ret);

	mfree(list);
	diagonal_node_tree_ptr = NULL;

/*
#ifdef DEBUG_MEMORY
	dump_alloc_memory_info();
#endif
*/
}

void diagonal_pattern_tree_init(unsigned int *load_time)
{
	struct diagonal_type *diag_item;
	struct diagonal_node **list = &diagonal_node_tree_ptr;
	unsigned char pattern[32];
	int i, idx;

	if (load_time == NULL)
		load_time = default_load_time;

	diagonal_weight_time_table = load_time;

	/* Generate diagonal node tree for search */
	for (i = 0; i < P_MAX; i++) {
		diag_item = &pattern_table[i];

		if (diag_item == NULL)
			print_exit("%s diag_item has wrong value!\n",
					__func__);

		/* create nodes and add to the list */
		diagonal_pattern_tree_add(list, diag_item,
				load_time, i);
	}

	/* Add forward path as maximum 15 */
	memset(pattern, FD, sizeof(pattern));
	diag_item = mmalloc(sizeof(struct diagonal_type));
	if (diag_item == NULL)
		print_exit("%s: malloc failure!\n", __func__);
	diag_item->pttn = pattern;
	for (idx = SL_F3, i = 3; idx <= SL_F16;
			idx++, i++) {
		diag_item->size = i;

		/* create nodes and add to the list */
		diagonal_pattern_tree_add(list, diag_item,
				load_time, idx);
	}

	/* Add diagonal forward path  RLRL... as maximum 27 */
	for (i = 0; i < 32; i++) {
		if (i & 1)
			pattern[i] = LD;
		else
			pattern[i] = RD;
	}
	diag_item->pttn = pattern;
	for (idx = SL_D2, i = 3; idx <= SL_D27;
			idx++, i++) {
		diag_item->size = i;

		/* create nodes and add to the list */
		diagonal_pattern_tree_add(list, diag_item,
				load_time, idx);
	}

	/* Add diagonal forward path  LRLR... as maximum 27 */
	for (i = 0; i < 32; i++) {
		if (i & 1)
			pattern[i] = RD;
		else
			pattern[i] = LD;
	}
	diag_item->pttn = pattern;
	for (idx = SL_D2, i = 3; idx <= SL_D27;
			idx++, i++) {
		diag_item->size = i;

		/* create nodes and add to the list */
		diagonal_pattern_tree_add(list, diag_item,
				load_time, idx);
	}
	mfree(diag_item);

	if (diagonal_pattern_tree_verify())
		print_exit("%s is failed!!!\n", __func__);
}

struct diag_pttn_time_type *diagonal_pattern_search(
		unsigned char *pattern, int pttn_size)
{
	int i;
	struct diagonal_node *diag_node, *new_diag_node;

	diag_node = diagonal_node_tree_ptr;

	if (diag_node == NULL)
		print_exit("%s has NULL\n", __func__);

	print_dbg(DEBUG_DIAGNODE, "pttn_size %d\n", pttn_size);

	for (i = 0; i < pttn_size; i++) {
		new_diag_node = NULL;
		print_dbg(DEBUG_DIAGNODE,
				"node:%d, dir:%d, idx:%d ptr:%08X\n",
				diag_node->node_num, pattern[i], i,
				(unsigned int)diag_node);
		if (pattern[i] > 3) {
			for (i=0; i< pttn_size; i++)
				print_error("%d\n", pattern[i]);
			print_exit("%s(): array index of link in diag_node " \
					"can't be lager than 3, v:%d s:%d\n",
					__func__, pattern[i], pttn_size);
		}
		if (diag_node->link[pattern[i]])
			new_diag_node = diag_node->link[pattern[i]];

		if (new_diag_node)
			diag_node = new_diag_node;
		else
			print_exit("Couldn't find your path pattern " \
					"node:%X, 0:%X, 1:%X, 2:%X, 3:%X\n",
				(unsigned int)diag_node,
				(unsigned int)diag_node->link[0],
				(unsigned int)diag_node->link[1],
				(unsigned int)diag_node->link[2],
				(unsigned int)diag_node->link[3]);
	}

	print_dbg(DEBUG_DIAGNODE,
		"node:%X, time:%d, 0:%X, 1:%X, 2:%X, 3:%X\n",
		(unsigned int)diag_node,
		diag_node->pttn.time,
		(unsigned int)diag_node->link[0],
		(unsigned int)diag_node->link[1],
		(unsigned int)diag_node->link[2],
		(unsigned int)diag_node->link[3]);
	return &diag_node->pttn;
}

