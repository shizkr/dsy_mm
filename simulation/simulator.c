#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "memory.h"
#include "algo.h"
#include "diagonal.h"
#include "run_mouse.h"
#include "timer.h"

#ifdef MAZE_GUI
#include <gtk/gtk.h>
#include "maze.h"
#include "draw_maze.h"
#include "draw_mouse.h"
#endif

#define TAG "simul: "
#include "debug.h"

/*
 * Varialbles
 */
#ifdef DEBUG
static int debug_flag;
/* = DEBUG_BINTREE | DEBUG_S_LINK; */
#endif

/* Input maze from file */
unsigned char maze_file[MAZEMAX];

int load_maze(char *filename, unsigned char *maze)
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

#ifndef MAZE_GUI
static void run_mouse(unsigned char *maze_file)
{
	/*
	while (simul_mouse_search_run(maze_file) < 2)
			;
	*/
	simul_mouse_search_run(maze_file);
}
#endif

int main(int argc, char *argv[])
{
#ifdef CAL_PATHES
	unsigned char cur_mouse_pos = 0x00;
	unsigned char cur_mouse_dir = NI;
	struct s_link *mouse_path, *f_node;
	int diag_path[128], i;
#endif

	print_dbg(DEBUG_PRINT, "Start main\n");

	if (argc < 2)
		print_exit("Usage: %s [maz file]\n", argv[0]);

	if (load_maze(argv[1], maze_file))
		exit(EXIT_FAILURE);

	print_info("loaded maze file\n");
	print_map(maze_file);

	diagonal_pattern_tree_init(load_time_1);

	print_info("Initialized maze\n");

	maze_search = mmalloc(MAZEMAX);
	memset(maze_search, 0, MAZEMAX);

	initialize_maze(maze_search);
	/* Make sure input file */
	initialize_maze(maze_file);
	print_map(maze_search);
	print_map(maze_search);

#ifdef MAZE_GUI
	gui_init(argc, argv);
	init_draw_maze(maze_file);
	draw_mouse(0, 0, 0);

	run_timer(maze_file);
#else
	run_mouse(maze_file);
	/*
	printf("Time: %d ms\n", get_total_path_time());
	*/
#endif

#ifdef CAL_PATHES
	draw_contour(maze_file, contour_map, TO_GOAL_16X16, cur_mouse_pos);
	mouse_path = gen_bin_tree(maze_file,
			contour_map, cur_mouse_pos, cur_mouse_dir);

	f_node = find_fastest_path(mouse_path);

	get_diag_path_from_node(f_node, diag_path);

	for (i = 0; diag_path[i] != 0xff; i++)
		printf("%02X", diag_path[i]);
	printf("\n");
#endif

#ifdef MAZE_GUI
	gtk_main();
#endif

	/* destroy used resources */
	mfree(maze_search);
	diagonal_pattern_tree_remove();

#ifdef DEBUG_MEMORY
	dump_alloc_memory_info();
#endif
	exit(EXIT_SUCCESS);
}
