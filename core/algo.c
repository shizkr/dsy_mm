/*
 * Solve maze problem by contour map
 *
 * Copyright (c) 2013 Denny Yang <denny.ds.yang@gmail.com>
 *
 */

#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "memory.h"
#include "algo.h"
#include "diagonal.h"

#define TAG "ALGO: "
#include "debug.h"

/*
 * Varialbles
 */
#ifdef DEBUG
static int debug_flag;
/* = DEBUG_SEARCH | DEBUG_BINTREE | DEBUG_S_LINK; */
#endif

/* circular ring buffer max definition */
#define CONTOUR_CB_BUFFER_MAX   128

/* Memorize maze information */
char maze_search[MAZEMAX];
char contour_map[MAZEMAX];

/* Delta index of NESW from current x, y index */
static const char maze_dxy[4] = { 0x01, 0x10, -0x01, -0x10 };

static struct btree_node *contour_tree;

/* Temporary circular ring buffer for contour map */
static unsigned int contour_cb_buffer[CONTOUR_CB_BUFFER_MAX];

/*
 * Functions
 */

/* Initialize maze memory to have known maze information */
void initialize_maze(char *maze)
{
	int x, y;

	for (x = 0; x < MAX_X; x++) {
		for (y = 0; y < MAX_Y; y++) {
			if (x == 0)
				maze[get_index(x, y)] |= (KNOWN_WEST|WEST);
			if (y == 0)
				maze[get_index(x, y)] |= (KNOWN_SOUTH|SOUTH);
			if (x == MAX_X - 1)
				maze[get_index(x, y)] |= (KNOWN_EAST|EAST);
			if (y == MAX_Y - 1)
				maze[get_index(x, y)] |= (KNOWN_NORTH|NORTH);
		}
	}

	maze[get_index(0, 0)] |= (0xf0|EAST);
	maze[get_index(1, 0)] |= (KNOWN_WEST|WEST);
}

#ifdef DEBUG
/* Draw maze information or contour map.
 * Map must be MAZEMAX bytes.
 */
void print_map(char *map)
{
	int x, y;

	for (y = MAX_Y - 1; y >= 0; y--) {
		for (x = 0; x < MAX_X; x++)
			printf("%02X", map[get_index(x, y)]&0xf);
		printf("\n");
	}
	printf("\n");
}
#else
void print_map(char *map) { }
#endif

/* Draw maze information or contour map.
 * Map must be MAZEMAX bytes.
 */
void print_full(char *map)
{
	int x, y;

	for (y = MAX_Y - 1; y >= 0; y--) {
		for (x = 0; x < MAX_X; x++)
			printf("%02X", (unsigned char)map[get_index(x, y)]);
		printf("\n");
	}
	printf("\n");
}

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
	int i, contour_lvl = 1;
	int index;
	unsigned int item;
	char found_mouse = 0;
	struct circular_buffer *cb, contour_buffer;

	cb = &contour_buffer;
	circular_buffer_init(cb, contour_cb_buffer,
			CONTOUR_CB_BUFFER_MAX);

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

	/* Get one contour number from circular buffer.
	 * Put next higher value in next block if there is
	 * no wall to there and save it inti circular buffer.
	 * If contour map reaches to current mouse or
	 * circular buffer is empty then it's done.
	 */
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
	struct btree_node bt_node_backup;
	char dir, is_goal = 0, found_node;

	if (!maze || !map || !head || !*head)
		print_exit("NULL pointer error! %X %X %X %X\n",
				(unsigned int)maze, (unsigned int)map,
				(unsigned int)head, (unsigned int)*head);

	print_dbg(DEBUG_S_LINK, "Check input linked list: %08X\n",
			(unsigned int)*head);
	debug_sl_node(*head);

	print_dbg(DEBUG_S_LINK, "Find next lower contour level\n");
	/* Find next lower contour cube arond current location */
	for (sl_node = *head; sl_node; sl_node = sl_node->node) {
		index = sl_node->bt_node->pos;
		abs_dir = sl_node->bt_node->abs_dir;
		bt_node = sl_node->bt_node;

		/* when it's set, found turn. on the unkonwn blocks,
		 * there may be too many pathes to calculate.
		 **/
		found_node = 0;
		bt_node_backup.time = 0; /* use to check FD detection */

		for (i = NI; i <= WI; i++) {
			if (!(maze[index] & wall_bit(i)) &&
				(map[index] == map[index + maze_dxy[i]] + 1)) {

				dir = relative_direction(abs_dir, i);

				/* if block is unknown, then take turn prior
				 * to straight path. It's to save memory
				 * and calculation time.
				 */
				if (dir == FD && ((maze[index]&0xF0) != 0xF0)) {
					bt_node_backup.pos = index+maze_dxy[i];
					bt_node_backup.dir = dir;
					bt_node_backup.abs_dir = i;
					/* to mark block has meaningful data */
					bt_node_backup.time = 1;
					continue;
				}

				found_node = 1;

				/* create new bt_node and save next mouse index
				 * and absolute direction of mouse at next block
				 */
				bt_new_node =
					bt_node_alloc(index+maze_dxy[i], i);

				/* Next bt_node->dir is to save how mouse made
				 * a turn to come the block.
				 */
				bt_new_node->dir = dir;

				add_bt_node(bt_node, bt_new_node);

				sl_new_node = s_link_alloc(bt_new_node);
				add_sl_node(&tail_new_list, sl_new_node);

				/*
				 * debug_sl_node(tail_new_list);
				 */
			}
		}

		/* if forward is the only one in known block,
		 * then use it
		 */
		if (!found_node && bt_node_backup.time) {
			bt_new_node =
				bt_node_alloc(bt_node_backup.pos,
						bt_node_backup.abs_dir);
			bt_new_node->dir = bt_node_backup.dir;

			add_bt_node(bt_node, bt_new_node);

			sl_new_node = s_link_alloc(bt_new_node);
			add_sl_node(&tail_new_list, sl_new_node);
		}

		if (map[index] == 1 && !is_goal)
			is_goal = 1;
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

	/* critical error to find next contour maps */
	if (!is_goal && tail_new_list == NULL) {
		printf("contour map\n");
		print_full(map);
		printf("maze map\n");
		print_map(maze);
		for (sl_node = *head; sl_node; sl_node = sl_node->node) {
			index = sl_node->bt_node->pos;
			abs_dir = sl_node->bt_node->abs_dir;
			bt_node = sl_node->bt_node;
			printf("mouse:(%d,%d),abs_dir:%d\n", pos_x(index),
					pos_y(index), abs_dir);
		}
	}

	/* free s_link nodes */
	if (!is_goal) {
		sl_node_free(*head);
		*head = tail_new_list;
	}

	return is_goal;
}

void free_top_node_contour_tree(void)
{
	if (!contour_tree)
		print_exit("%s:Do not call this before generating tree.\n",
				__func__);
	free_bt_node_list(contour_tree);
	contour_tree = NULL;
}

/* maze: maze array pointer
 * map: contour maze array pointer
 * pos_st: current mouse position x/y 8 bit index
 */
struct s_link *gen_bin_tree(char *maze, char *map, unsigned char pos_st,
		unsigned char abs_dir)
{
	struct s_link *tail_list = NULL;
#ifdef DEBUG
	struct btree_node *bt_node;
	struct s_link *sl_node;
	int idx;
	unsigned char path[MAZEMAX + 1];
#endif

	/* Init first node from current mouse location */
	if (contour_tree != NULL)
		print_exit("%s:Please free the binary tree before creating!\n",
				__func__);
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

		idx++;
		while (idx < MAZEMAX) {
			printf("%C",
				(path[idx] == FD) ? 'F' : \
				((path[idx] == RD) ? 'R' : \
				((path[idx] == BD) ? 'B' : \
				((path[idx] == LD) ? 'L' : 'X'))));
			idx++;
		}
		printf("\n");
	}
#endif
	return tail_list;
}

unsigned char *gen_frbl_from_node(struct s_link *sl_node)
{
	int idx;
	struct btree_node *bt_node;
	static unsigned char path[MAZEMAX + 1];

	bt_node = sl_node->bt_node;
	idx = MAZEMAX;

	/*
	 * Generate FRBL pattern and save it to array
	 */

	/* diagonal path has to go 1 more block in the goal */
	path[idx--] = 0xff; /* end of path */
	path[idx--] = FD;
	while (bt_node->parent) {
		if (bt_node->dir <= LD)
			path[idx--] = bt_node->dir;
		else
			print_exit("Invalid direction in bt_node!\n");
		bt_node = bt_node->parent;
	}
	return &path[++idx];
}

int get_diag_path_from_node(struct s_link *node, int *diag_path)
{
	unsigned char *path;

	path = gen_frbl_from_node(node);

	return get_diag_path_from_turn(path, diag_path);
}

int get_diag_path_from_turn(unsigned char *path, int *diag_path)
{
	int idx = 0, size;
	unsigned int time;
	int i, diag_idx;
	struct diag_pttn_time_type *pttn;

	/*
	 * Start searching diagonal pattern from FRBL array
	 */
	for (idx = 0, size = 2, diag_idx = 0;
		(path[idx] <= LD) && (path[idx+1]) <= LD;) {
		do {
			/* generate FF... upto 15F pattern */
			if (path[idx] == FD &&
				path[idx+1] == FD) {
				i = 1;
				while (path[idx+1+i] == FD) {
					size++;
					i++;
				}
			}
			/* generate LR...upto 27D pattern */
			if (path[idx] == LD &&
				path[idx+1] == RD) {
				i = 1;
				do {
					if ((i&1) && (path[idx+1+i] != LD))
						break;
					if (!(i&1) && (path[idx+1+i] != RD))
						break;
					size++;
					i++;
				} while (1);
			}

			/* generate RL...upto 27D pattern */
			if (path[idx] == RD &&
				path[idx+1] == LD) {
				i = 1;
				do {
					if ((i&1) && (path[idx+1+i] != RD))
						break;
					if (!(i&1) && (path[idx+1+i] != LD))
						break;
					size++;
					i++;
				} while (1);
			}

			pttn = diagonal_pattern_search(
					&path[idx], size);
			time = pttn->time;

			if (!time) {
				size++;
				if (size > MAZEMAX/9 + 4)
					print_exit(
					"Error on path finding!\n");
			} else {
				/*
				total_time += time;
				*/
				diag_path[diag_idx++] = pttn->pttn;
				if (diag_idx >= 128)
					print_exit(
					"%s:Increase diag path " \
					"array size!\n", __func__);
				idx += (size - 1);
				size = 2;
				break;
			}
		} while (1);
	}

	diag_path[diag_idx] = 0xff;
	if (diag_idx == 0)
		return -1;

	return 0;
}

int calculate_path_time(unsigned char *path)
{
	int idx = 0, size;
	unsigned int time, total_time = 0;
	int i;
	struct diag_pttn_time_type *pttn;

	/*
	 * Start searching diagonal pattern from FRBL array
	 */
	for (idx = 0, size = 2;
		(path[idx] <= LD) && (path[idx+1]) <= LD;) {
		do {
			/* generate FF... upto 15F pattern */
			if (path[idx] == FD &&
				path[idx+1] == FD) {
				i = 1;
				while (path[idx+1+i] == FD) {
					size++;
					i++;
				}
			}
			/* generate LR...upto 27D pattern */
			if (path[idx] == LD &&
				path[idx+1] == RD) {
				i = 1;
				do {
					if ((i&1) && (path[idx+1+i] != LD))
						break;
					if (!(i&1) && (path[idx+1+i] != RD))
						break;
					size++;
					i++;
				} while (1);
			}

			/* generate RL...upto 27D pattern */
			if (path[idx] == RD &&
				path[idx+1] == LD) {
				i = 1;
				do {
					if ((i&1) && (path[idx+1+i] != RD))
						break;
					if (!(i&1) && (path[idx+1+i] != LD))
						break;
					size++;
					i++;
				} while (1);
			}

			pttn = diagonal_pattern_search(
					&path[idx], size);
			time = pttn->time;

			if (!time) {
				size++;
				if (size > MAZEMAX/9 + 4)
					print_exit(
					"Error on path finding!\n");
			} else {
				total_time += time;

				idx += (size - 1);
				size = 2;
				break;
			}
		} while (1);
	}
	return total_time;
}

struct s_link *find_fastest_path(struct s_link *pathes)
{
	struct s_link *sl_node, *fast_sl_node;
	unsigned int total_time, fast_time = 0xffffffff;
	unsigned char *path;

	for (sl_node = pathes; sl_node; sl_node = sl_node->node) {
		path = gen_frbl_from_node(sl_node);
		/* back turn is assumed as FD to find the path */
		if (*path == BD)
			*path = FD;
		total_time = calculate_path_time(path);
		sl_node->bt_node->time = total_time;

		/* Keep the fastest path */
		if (total_time < fast_time) {
			fast_time = total_time;
			fast_sl_node = sl_node;
		}
	}

	printf("Total_time: %dmS\n", fast_time);
	if (fast_time == 0xffffffff)
		print_exit("%s: Couldn't find the fastest path!\n",
				__func__);

	return fast_sl_node;
}

/* wall bits are filled by absolute direction.
 * index : current mouse location to save the wall information.
 */
void save_wallinfo_to_maze(unsigned char index, unsigned char wall)
{
	int x, y;

	print_dbg(DEBUG_SEARCH, "%02X, %02X\n", index, wall&0xf);
	/* trying to save know wall to maze_search? */
	if ((maze_search[index]&0xF0) == 0xF0)
		print_exit("%s: %d,%d is already known block!\n",
				__func__, pos_x(index), pos_y(index));

	/* save wall info to current index */
	maze_search[index] |= (wall&0xf);
	maze_search[index] |= 0xF0;

	/* save wall information the wall of other blocks */
	x = pos_x(index);
	y = pos_y(index);

	/* north (it's south wall of next block) */
	if (y+1 < MAX_Y) {
		maze_search[get_index(x, y+1)] |= KNOWN_SOUTH;
		if (wall & NORTH)
			maze_search[get_index(x, y+1)] |= SOUTH;
	}
	/* east (it's west wall of next block) */
	if (x+1 < MAX_X) {
		maze_search[get_index(x+1, y)] |= KNOWN_WEST;
		if (wall & EAST)
			maze_search[get_index(x+1, y)] |= WEST;
	}
	/* south (it's north wall of next block) */
	if (y > 0) {
		maze_search[get_index(x, y-1)] |= KNOWN_NORTH;
		if (wall & SOUTH)
			maze_search[get_index(x, y-1)] |= NORTH;
	}
	/* west (it's east wall of next block) */
	if (x > 0) {
		maze_search[get_index(x-1, y)] |= KNOWN_EAST;
		if (wall & WEST)
			maze_search[get_index(x-1, y)] |= EAST;
	}
	print_dbg(DEBUG_SEARCH, "%s: index:%02X maze_search[index]=%02X\n",
			__func__, index, (unsigned char)maze_search[index]);
}

int is_goal(unsigned char index)
{
	if (index == 0x77 || index == 0x87 ||
			index == 0x78 || index == 0x88)
		return 1;
	return 0;
}

