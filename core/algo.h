#ifndef ALGO_H
#define ALGO_H

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

#define FD	0
#define RD	1
#define BD	2
#define LD	3

#define MAX_X	15
#define MAX_Y	15

/* direction bit through direction index */
#define wall_bit(direct) (0x01 << (direct))

#define pos_x(index)	((index>>4)&0xf)
#define pos_y(index)	(index&0xf)
#define get_index(x, y)	(((x)<<4)|(y))

#define gen_contour_pos(value, index) ((value << 8)|(index))
#define get_contour_lvl(map)	(map >> 8)
#define get_contour_index(map)  (map & 0xff)

/*
 * next_dir: next absolute direction index(0~3) to go
 * curr_dir: current mouse absolute direction index(0~3)
 * result: forward(0), right(1), back(2) and left(3)
 */
#define relative_direction(curr, next) \
	((4+(next))-(curr) & 3)

/* r_dir: relative direction of FRBL
 * curr_dir: current mouse direction
 * result: absolute direction of NESW(0~3)
 */
#define absolute_direction(curr_dir, rel_dir) \
	(((curr_dir)+(rel_dir)) & 3)

extern char maze_search[MAZEMAX];
extern char contour_map[MAZEMAX];

/*
 * Functions
 */

/* Initialize maze memory to have known maze information */
void initialize_maze(char *maze);

void print_map(char *map);

void draw_contour(char *maze, char *map);

/* maze: maze array pointer
 * map: contour maze array pointer
 * pos_st: current mouse position x/y 8 bit index
 */
void gen_bin_tree(char *maze, char *map, unsigned char pos_st);
#endif
