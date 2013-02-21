#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "circular_buffer.h"
#include "bin_tree.h"
#include "algo.h"
#include "diagonal.h"

#define TAG "simul: "
#include "debug.h"

/*
 * Varialbles
 */
#ifdef DEBUG
static int debug_flag = DEBUG_BINTREE | DEBUG_S_LINK;
#endif

/* Input maze from file */
char maze_file[MAZEMAX];

int load_maze(char *filename, char *maze)
{

	FILE *fd = NULL;
	memset(maze, 0, sizeof(maze));

	fd = fopen(filename, "r+");

	if (NULL == fd) {
		print_error("fopen() Error!!!\n");
		return 1;
	}

	if (MAZEMAX != fread(maze, 1, MAZEMAX, fd)) {
		print_error("fread() failed\n");
		return 1;
	}

	fclose(fd);
	return 0;
}

int main(int argc, char *argv[])
{
	unsigned char cur_mouse_pos = 0x00;
	unsigned char cur_mouse_dir = NI;
	struct s_link *mouse_path;
	unsigned char f_path[128], i;

	print_dbg(DEBUG_PRINT, "Start main\n");

	if (argc < 2)
		print_exit("Usage: %s [maz file]\n", argv[0]);

	if (load_maze(argv[1], maze_file))
		exit(EXIT_FAILURE);

	print_info("loaded maze file\n");
	print_map(maze_file);

	diagonal_pattern_tree_init(default_load_time);

	print_info("Initialized maze\n");
	initialize_maze(maze_search);
	print_map(maze_search);

#if 0
	draw_contour(maze_search, contour_map, TO_GOAL_16X16, cur_mouse_pos);
	mouse_path = gen_bin_tree(maze_search,
			contour_map, cur_mouse_pos, cur_mouse_dir);
#else
	draw_contour(maze_file, contour_map, TO_GOAL_16X16, cur_mouse_pos);
	mouse_path = gen_bin_tree(maze_file,
			contour_map, cur_mouse_pos, cur_mouse_dir);
#endif

	find_fastest_path(mouse_path, f_path);

	for (i = 0; f_path[i] != 0xff; i++)
		printf("%02X", f_path[i]);
	printf("\n");

	exit(EXIT_SUCCESS);
}
