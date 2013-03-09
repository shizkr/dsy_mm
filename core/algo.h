/*
 * Solve maze problem by contour map
 *
 * Copyright (c) 2013 Denny Yang <denny.ds.yang@gmail.com>
 *
 */

#ifndef ALGO_H
#define ALGO_H

#include "diagonal.h"

/* Consider to support 1024(32X32) map */
#define MAZEMAX 256

/*
 * Macro definition
 */
#define NI	0
#define EI	1
#define SI	2
#define WI	3

#define NORTH	1
#define EAST	2
#define SOUTH	4
#define WEST	8

#define KNOWN_NORTH	0x10
#define KNOWN_EAST	0x20
#define KNOWN_SOUTH	0x40
#define KNOWN_WEST	0x80

#define FD	0
#define RD	1
#define BD	2
#define LD	3
#define NOD 4  /* Invalid direction */

#define MAX_X	16
#define MAX_Y	16

/* direction bit through direction index */
#define wall_bit(direct) (0x01 << (direct))

#define pos_x(index)	((index>>4)&0xf)
#define pos_y(index)	(index&0xf)
#define get_index(x, y)	(((x)<<4)|(y))
#define dec_x_index(index) (index-=0x10)
#define inc_x_index(index) (index+=0x10)
#define dec_y_index(index) (index-=0x01)
#define inc_y_index(index) (index+=0x01)

#define gen_contour_pos(value, index) ((value << 8)|(index))
#define get_contour_lvl(map)	(map >> 8)
#define get_contour_index(map)  (map & 0xff)

/*
 * next_dir: next absolute direction index(0~3) to go
 * curr_dir: current mouse absolute direction index(0~3)
 * result: forward(0), right(1), back(2) and left(3)
 */
#define relative_direction(curr, next) \
	(((4+(next))-(curr))&3)

/* r_dir: relative direction of FRBL
 * curr_dir: current mouse absolute direction
 * result: absolute direction of NESW(0~3)
 */
#define absolute_direction(curr_dir, rel_dir) \
	(((curr_dir)+(rel_dir)) & 3)

/* MAZE SEARCH TYPE */
enum SEARCH_TYPE {
	TO_GOAL_4X4 = 0,
	TO_START_4X4,
	TO_GOAL_5X5,
	TO_START_5X5,
	TO_GOAL_8X8,
	TO_START_8X8,
	TO_GOAL_9X9,
	TO_START_9X9,
	TO_GOAL_16X16,
	TO_START_16X16,
	TO_GOAL_32X32,
	TO_START_32X32,
};

extern char maze_search[MAZEMAX];
extern char contour_map[MAZEMAX];

void initialize_maze(char *maze);
void print_map(char *map);
void draw_contour(char *maze, char *map,
		enum SEARCH_TYPE type, unsigned char pos);
struct s_link *gen_bin_tree(char *maze, char *map, unsigned char pos_st,
		unsigned char abs_dir);
unsigned char *gen_frbl_from_node(struct s_link *sl_node);
int get_diag_path_from_node(struct s_link *node, int *diag_path);
int get_diag_path_from_turn(unsigned char *path, int *diag_path);
int calculate_path_time(unsigned char *path);
struct s_link *find_fastest_path(struct s_link *pathes);
void save_wallinfo_to_maze(unsigned char index, unsigned char wall);
int is_goal(unsigned char index);
void free_top_node_contour_tree(void);

#endif
