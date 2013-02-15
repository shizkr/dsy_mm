#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "circular_buffer.h"

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

#define MAX_X	15
#define MAX_Y	15

#define wall_bit(direc) (0x01 << (direc))

#define pos_x(index)	((index>>4)&0xf)
#define pos_y(index)	(index&0xf)
#define get_index(x, y)	(((x)<<4)|(y))

#define gen_contour_pos(value, index) ((value << 8)|(index))
#define get_contour_lvl(map)	(map >> 8)
#define get_contour_index(map)  (map & 0xff)

#define DEBUG

/*
 * Varialbles
 */

/* Input maze from file */
char maze_file[MAZEMAX];
/* Memorize maze information */
char maze_search[MAZEMAX];

char contour_map[MAZEMAX];

/* delta index of NESW from current x, y index */
static const char maze_dxy[4] = { 0x01, 0x10, -0x01, -0x10 };

/*
 * Functions
 */
int load_maze(char *filename, char *maze)
{

	FILE *fd = NULL;
	memset(maze, 0, sizeof(maze));

	fd = fopen(filename, "r+");

	if (NULL == fd) {
		printf("\n fopen() Error!!!\n");
		return 1;
	}

	if (MAZEMAX != fread(maze, 1, MAZEMAX, fd)) {
		printf("\n fread() failed\n");
		return 1;
	}

	fclose(fd);
	return 0;
}

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

void draw_contour(char *maze, char *map)
{
	#define BUFFER_SIZE 64
	int i, contour_lvl = 1;
	int index, item;
	int buffer[BUFFER_SIZE];
	struct circular_buffer *cb, contour_buffer;

	cb = &contour_buffer;
	circular_buffer_init(cb, buffer, BUFFER_SIZE);

	/* Uninitialized contour map */
	memset(map, 0, MAZEMAX);

	/* Seed value 1 as a goal */
	map[get_index(7, 7)] = contour_lvl;
	map[get_index(8, 7)] = contour_lvl;
	map[get_index(7, 8)] = contour_lvl;
	map[get_index(8, 8)] = contour_lvl;

	/* Add list of same level contour value */
	circular_buffer_write(cb, gen_contour_pos(contour_lvl, 0x77));
	circular_buffer_write(cb, gen_contour_pos(contour_lvl, 0x78));
	circular_buffer_write(cb, gen_contour_pos(contour_lvl, 0x87));
	circular_buffer_write(cb, gen_contour_pos(contour_lvl, 0x88));

	while (!circular_buffer_empty(cb)) {
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
			}
		}
#ifdef DEBUG
		print_map(map);
		usleep(20000);
#endif
	}

}

int main(int argc, char *argv[])
{
	printf("%s\n", argv[0]);
	printf("%s\n", argv[1]);
	load_maze(argv[1], maze_file);

	printf("load maze\n");
	print_map(maze_file);

	printf("Initialized maze\n");
	initialize_maze(maze_search);
	print_map(maze_search);

	draw_contour(maze_search, contour_map);
	draw_contour(maze_file, contour_map);
	exit(EXIT_SUCCESS);
}
