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
#include "timer.h"
#endif

#define TAG "runmouse: "
#include "debug.h"

/*
 * Varialbles
 */
#ifdef DEBUG
static int debug_flag = DEBUG_SEARCH;
#endif

/* Read maze block according to current mouse position and
 * direction. This has to be replaced in the real world.
 * index : mouse is assumed that it's already moved to next
 * blcok and read the wall information.
 */
static unsigned char read_maze_block(char *maze_file, unsigned char index)
{
	return maze_file[index];
}

/* Find fastest path from known maze information
 * and return the FRBL turn array to the caller.
 * if first turn is one of RBL, it means that the
 * mouse has to make back turn or smooth L/R turn.
 * In that case, no diagonal turns.
 */
unsigned char *find_maze_fastest_path(char *maze_file,
	unsigned char cur_mouse_pos, char cur_mouse_dir,
	enum SEARCH_TYPE search_type, struct s_link **f_node)
{
	unsigned char *path;
	struct s_link *mouse_path;
#ifdef DEBUG
	int i;
#endif

	/* draw contour map from the goal */
	draw_contour(maze_search, contour_map,
			search_type, cur_mouse_pos);
	/* generate full binary tree with all pathes */
	mouse_path = gen_bin_tree(maze_search,
			contour_map, cur_mouse_pos, cur_mouse_dir);
	/* find the fastest path and get the node */
	*f_node = find_fastest_path(mouse_path);

	/* generate FRBL array for the fastest path */
	path = gen_frbl_from_node(*f_node);

#ifdef DEBUG
	for (i = 0; path[i] != 0xff; i++)
		printf("%C",
			(path[i] == FD) ? 'F' : \
			((path[i] == RD) ? 'R' : \
			((path[i] == BD) ? 'B' : \
			((path[i] == LD) ? 'L' : 'X'))));
	printf("\n");
#endif

	return path;
}

static unsigned char cur_mouse_pos;
static char cur_mouse_dir = NI;

static void simul_mouse_search_goal(char *maze_file, char target)
{
	struct s_link *f_node;
	unsigned char *path_array;
	enum SEARCH_TYPE search_type;

	print_dbg(DEBUG_SEARCH, "%s\n", __func__);

	if (target == 0)
		search_type = TO_GOAL_16X16;
	else if (target == 1)
		search_type = TO_START_16X16;
	else
		return;

	/* find next block */
	path_array = find_maze_fastest_path(maze_file,
		cur_mouse_pos, cur_mouse_dir, search_type, &f_node);

	cur_mouse_dir = absolute_direction(cur_mouse_dir, *path_array);

	print_dbg(DEBUG_SEARCH, "Move mouse\n");
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

	if ((maze_search[cur_mouse_pos]&0xF0) != 0xF0)
		save_wallinfo_to_maze(cur_mouse_pos,
			read_maze_block(maze_file, cur_mouse_pos));

	print_dbg(DEBUG_SEARCH, "new position X:%d,Y:%d\n",
			pos_x(cur_mouse_pos), pos_y(cur_mouse_pos));
	print_map(maze_search);

	/* this must be called before starting new mapping */
	free_top_node_contour_tree();
#ifdef MAZE_GUI
	draw_mouse(pos_x(cur_mouse_pos)*48,
			pos_y(cur_mouse_pos)*48, cur_mouse_dir * 90);
#endif
}

int simul_mouse_search_run(char *maze_file)
{
	static char current_target;

	/* start looking for goal with search run */
	if (current_target == 0) {
		simul_mouse_search_goal(maze_file, current_target);
		if (is_goal(cur_mouse_pos)) {
			current_target = 1;
			return 1;
		}
	} else if (current_target == 1) {
		/* return to start with search run */
		simul_mouse_search_goal(maze_file, current_target);
		if (cur_mouse_pos == 0) {
			current_target = 2;
			return 2;
		}
	}

	return 0;
}
