#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "circular_buffer.h"
#include "bin_tree.h"
#include "algo.h"
#include "diagonal.h"

#define TAG "ALGO: "
#include "debug.h"

/*
 * Varialbles
 */
#ifdef DEBUG
static int debug_flag;

/* DEBUG_BINTREE | DEBUG_S_LINK; */
#endif

/* Memorize maze information */
char maze_search[MAZEMAX];
char contour_map[MAZEMAX];

/* delta index of NESW from current x, y index */
static const char maze_dxy[4] = { 0x01, 0x10, -0x01, -0x10 };

static struct btree_node *contour_tree;

/*
 * Functions
 */

/* Initialize maze memory to have known maze information */
void initialize_maze(char *maze)
{
	int x, y;

	for (x = 0; x <= MAX_X; x++) {
		for (y = 0; y <= MAX_Y; y++) {
			if (x == 0)
				maze[get_index(x, y)] |= WEST;
			if (y == 0)
				maze[get_index(x, y)] |= SOUTH;
			if (x == MAX_X)
				maze[get_index(x, y)] |= EAST;
			if (y == MAX_Y)
				maze[get_index(x, y)] |= NORTH;

		}
	}

	maze[get_index(0, 0)] |= EAST;
}

#ifdef DEBUG
/* Draw maze information or contour map.
 * Map must be 256 bytes.
 */
void print_map(char *map)
{
	int x, y;

	for (y = MAX_Y; y >= 0; y--) {
		for (x = 0; x <= MAX_X; x++)
			printf("%02X", map[get_index(x, y)]);
		printf("\n");
	}
	printf("\n");
}
#else
void print_map(char *map) { }
#endif

/* function name: draw_contour
 * Input Parameter
 *   maze : maze array wich has maze information
 *   map  : array to draw contour map
 *   type : maze type to run mouse
 *   pos  : current mouse location in the maze
 */
void draw_contour(char *maze, char *map,
		enum SEARCH_TYPE type, unsigned char pos)
{
	#define BUFFER_SIZE 64
	int i, contour_lvl = 1;
	int index, item;
	char found_mouse = 0;
	int buffer[BUFFER_SIZE];
	struct circular_buffer *cb, contour_buffer;

	cb = &contour_buffer;
	circular_buffer_init(cb, buffer, BUFFER_SIZE);

	/* Uninitialized contour map */
	memset(map, 0, MAZEMAX);

	/* Seed value 1 as a goal */
	switch (type) {
	case TO_GOAL_4X4:
		map[get_index(3, 3)] = contour_lvl;
		circular_buffer_write(cb, gen_contour_pos(contour_lvl, 0x33));
		break;
	case TO_GOAL_8X8:
		map[get_index(7, 7)] = contour_lvl;
		circular_buffer_write(cb, gen_contour_pos(contour_lvl, 0x77));
		break;
	case TO_GOAL_16X16:
		map[get_index(7, 7)] = contour_lvl;
		map[get_index(8, 7)] = contour_lvl;
		map[get_index(7, 8)] = contour_lvl;
		map[get_index(8, 8)] = contour_lvl;

		/* Add list of same level contour value */
		circular_buffer_write(cb, gen_contour_pos(contour_lvl, 0x77));
		circular_buffer_write(cb, gen_contour_pos(contour_lvl, 0x78));
		circular_buffer_write(cb, gen_contour_pos(contour_lvl, 0x87));
		circular_buffer_write(cb, gen_contour_pos(contour_lvl, 0x88));
		break;
	case TO_START_4X4:
	case TO_START_8X8:
	case TO_START_16X16:
		map[get_index(0, 0)] = contour_lvl;
		circular_buffer_write(cb, gen_contour_pos(contour_lvl, 0x00));
		break;
	default:
		print_exit("Unsupported mouse search type %d!!",
				type);
		break;
	}

	while (!circular_buffer_empty(cb) && !found_mouse) {
		circular_buffer_read(cb, &item);
		index = get_contour_index(item);
		contour_lvl = get_contour_lvl(item) + 1;

		/* Calculate contour lvl around current cube */
		for (i = NI; i <= WI; i++) {
			if (!(maze[index] & wall_bit(i)) &&
				(map[index + maze_dxy[i]] == 0)) {
				map[index + maze_dxy[i]] = contour_lvl;
				circular_buffer_write(cb,
					gen_contour_pos(contour_lvl,
					index + maze_dxy[i]));
				if (index + maze_dxy[i] == pos)
					found_mouse = 1;
			}
		}

#ifdef DEBUG
		if (debug_flag & DEBUG_CONTOUR) {
			print_map(map);
			usleep(20000);
		}
#endif
	}

	if (!found_mouse) {
		/* Mouse alorighm should never hit this location */
		print_exit("%s couldn't find mouse location\n",
				__func__);
	}
}

static int gen_bin_tree_tail(char *maze, char *map,
		struct s_link **head)
{
	struct btree_node *bt_new_node, *bt_node;
	struct s_link *sl_node;
	struct s_link *sl_new_node, *tail_new_list = NULL;
	unsigned char i, index, abs_dir;
	char is_goal = 0;

	if (!maze || !map || !head)
		print_exit("NULL pointer error!");

	print_dbg(DEBUG_S_LINK, "Check input linked list\n");
	debug_sl_node(*head);

	print_dbg(DEBUG_S_LINK, "Find next lower contour level\n");
	/* Find next lower contour cube arond current location */
	for (sl_node = *head; sl_node; sl_node = sl_node->node) {
		index = sl_node->bt_node->pos;
		abs_dir = sl_node->bt_node->abs_dir;
		bt_node = sl_node->bt_node;
		for (i = NI; i <= WI; i++) {
			if (!(maze[index] & wall_bit(i)) &&
				(map[index] == map[index + maze_dxy[i]] + 1)) {
				/* create new bt_node and save next mouse index
				 * and absolute direction of mouse at next block
				 */
				bt_new_node =
					bt_node_alloc(index+maze_dxy[i], i);

				/* Next bt_node->dir is to save how mouse made
				 * a turn to come the block.
				 */
				bt_new_node->dir =
					relative_direction(abs_dir, i);

				add_bt_node(bt_node, bt_new_node);

				sl_new_node = s_link_alloc(bt_new_node);
				add_sl_node(&tail_new_list, sl_new_node);

				/*
				 * debug_sl_node(tail_new_list);
				 */
			}

			if (map[index] == 1 && !is_goal)
				is_goal = 1;
		}
	}
	if (tail_new_list)
		debug_sl_node(tail_new_list);

#ifdef DEBUG
	if (is_goal) {
		i = 0;
		print_dbg(DEBUG_BINTREE,
				"Found maze goal!!!\n");
		print_dbg(DEBUG_BINTREE,
				"tail_new_list for tail: 0x%08X\n",
				(unsigned int)tail_new_list);
		print_dbg(DEBUG_BINTREE,
				"*head pointer: %08X\n",
				(unsigned int)*head);
		if (tail_new_list)
			print_error("tail_new_list must be NULL!\n");
		for (sl_node = *head; sl_node; sl_node = sl_node->node) {
			index = sl_node->bt_node->pos;
			bt_node = sl_node->bt_node;
			i++;
		}
		print_dbg(DEBUG_BINTREE, "Total path: %d\n", i);
	}
#endif

	/* free s_link nodes */
	if (!is_goal) {
		sl_node_free(*head);
		*head = tail_new_list;
	}

	return is_goal;
}

/* maze: maze array pointer
 * map: contour maze array pointer
 * pos_st: current mouse position x/y 8 bit index
 */
struct s_link *gen_bin_tree(char *maze, char *map, unsigned char pos_st,
		unsigned char abs_dir)
{
	struct btree_node *bt_node;
	struct s_link *sl_node, *tail_list = NULL;

	/* Init first node from current mouse location */
	contour_tree = bt_node_alloc(pos_st, abs_dir);
	tail_list = s_link_alloc(contour_tree);

	/* Generate binary tree until it finds goals of maze */
	while (1) {
		if (gen_bin_tree_tail(maze, map, &tail_list))
			break;
	}

#ifdef DEBUG
	/* verify output of binary tree, each path has unique
	 * path to current mouse location.
	 */
	printf("Possible path from start to goal\n");
	for (sl_node = tail_list; sl_node; sl_node = sl_node->node) {
		bt_node = sl_node->bt_node;
		while (bt_node->parent) {
			printf("%02X", bt_node->pos);
			bt_node = bt_node->parent;
		}
		printf("%02X\n", bt_node->pos);
	}

	/* Print of the direction of mouse path. All pathes has
	 * LRBF only from start to goal and it'll be used for pattern
	 * analysis to find the fastest path.
	 */
	for (sl_node = tail_list; sl_node; sl_node = sl_node->node) {
		bt_node = sl_node->bt_node;
		while (bt_node->parent) {
			printf("%C",
					(bt_node->dir == FD) ? 'F' : \
					((bt_node->dir == RD) ? 'R' : \
					((bt_node->dir == BD) ? 'B' : \
					((bt_node->dir == LD) ? 'L' : 'X'))));
			bt_node = bt_node->parent;
		}
		printf("\n");
	}
#endif
	return tail_list;
}

void find_fastest_path(struct s_link *pathes,
		unsigned char *f_path)
{
	struct btree_node *bt_node, *tail_bt_node;
	struct s_link *sl_node;
	int idx, size, pttn_size;
	unsigned int time, total_time, fast_time = 0xffffffff;
	unsigned char path[256];
	int i, diag_idx;
	enum speed_load_enum diag_path[128];
	struct diag_pttn_time_type *pttn;

	for (sl_node = pathes; sl_node; sl_node = sl_node->node) {
		tail_bt_node = bt_node = sl_node->bt_node;
		idx = 255;
		/* diagonal path has to go 1 more block in the goal */
		path[idx--] = FD;
		while (bt_node->parent) {
			if (bt_node->dir <= LD)
				path[idx--] = bt_node->dir;
			else
				print_exit("Invalid direction in bt_node!\n");
			bt_node = bt_node->parent;
		}

		total_time = 0;
		size = 2;
		idx++;
		diag_idx = 0;
		while (idx < 255) {
			pttn_size = 256 - idx;
			if (pttn_size < size)
				print_exit("Invalid diag pattern size\n");

			do {
				/* generate FF... patthern */
				if (path[idx] == FD &&
					path[idx+1] == FD) {
					i = 1;
					while (path[idx+1+i] == FD) {
						size++;
						i++;
					}
				}
				pttn = diagonal_pattern_search(
						&path[idx], size);
				time = pttn->time;

				if (!time) {
					size++;
					if (size > 30)
						print_exit(
							"Error on path finding!\n");
				} else {
					total_time += time;

					diag_path[diag_idx++] = pttn->pttn;
					if (diag_idx >= 128)
						print_exit("%s:Increase diag path " \
								"array size!\n", __func__);
					idx += (size - 1);
					size = 2;
					break;
				}
			} while (1);
		}
		tail_bt_node->time = total_time;

		if (total_time < fast_time) {
			fast_time = total_time;
			for (i = 0; i < diag_idx; i++)
				f_path[i] =
					(unsigned char)diag_path[i];
			f_path[i] = 0xff;
		}

		printf("Total_time: %d\n", total_time);
	}

	if (fast_time == 0xffffffff)
		print_exit("%s: Couldn't find the fastest path!\n",
				__func__);
}

