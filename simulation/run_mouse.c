#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "circular_buffer.h"
#include "bin_tree.h"
#include "algo.h"
#include "diagonal.h"

#ifdef MAZE_GUI
#include "drawmouse.h"
#endif

#define TAG "runmouse: "
#include "debug.h"

/*
 * Varialbles
 */
#ifdef DEBUG
static int debug_flag = DEBUG_BINTREE | DEBUG_S_LINK;
#endif

/* read maze block according to current mouse position and
 * direction. This has to be replaced in the real world.
 * index : mouse is assumed that it's already moved to next
 * blcok and read the wall information.
 */
static unsigned char read_maze_block(char *maze_file, unsigned char index)
{
	return maze_file[index];
}

void simul_mouse_search_goal(char *maze_file)
{
	unsigned char cur_mouse_pos = 0x00;
	char cur_mouse_dir = NI;
	struct s_link *mouse_path, *f_node;
	unsigned char *path_array;

	while (!is_goal(cur_mouse_pos)) {
		/* find next block */
		draw_contour(maze_search, contour_map,
				TO_GOAL_16X16, cur_mouse_pos);
		mouse_path = gen_bin_tree(maze_search,
				contour_map, cur_mouse_pos, cur_mouse_dir);
		f_node = find_fastest_path(mouse_path);

		path_array = gen_frbl_from_node(f_node);

		cur_mouse_dir = absolute_direction(cur_mouse_dir, *path_array);

		if (cur_mouse_dir == NI)
			inc_y_index(cur_mouse_pos);
		else if (cur_mouse_dir == EI)
			inc_x_index(cur_mouse_pos);
		else if (cur_mouse_dir == SI)
			dec_y_index(cur_mouse_pos);
		else if (cur_mouse_dir == WI)
			dec_x_index(cur_mouse_pos);
		else
			print_exit("%s:Unknown direction %d!\n",
					__func__, cur_mouse_dir);

		save_wallinfo_to_maze(cur_mouse_pos,
				read_maze_block(maze_file, cur_mouse_pos));

		printf("X:%d,Y:%d\n", pos_x(cur_mouse_pos), pos_y(cur_mouse_pos));
		print_map(maze_search);
#ifdef MAZE_GUI
		draw_mouse(pos_x(cur_mouse_pos) * 48,
				pos_y(cur_mouse_pos) * 48, 0);
#endif
	}
}
