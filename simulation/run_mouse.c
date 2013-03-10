#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include "memory.h"
#include "algo.h"
#include "diagonal.h"

#ifdef MAZE_GUI
#include "draw_mouse.h"
#include "draw_maze.h"
#include "timer.h"
#endif

#define TAG "runmouse: "
#include "debug.h"

/* Consider this file to be same on embedded build system
 * by configuration change. Basic run control shouldn't be
 * changed or make it same as much as possible.
 */

/*
 * Varialbles
 */
#ifdef DEBUG
static int debug_flag = DEBUG_SEARCH;
#endif

/* save current mouse position */
static unsigned char cur_mouse_pos;
/* save current mouse absolute direction */
static unsigned char cur_mouse_dir = NI;

#ifdef MAZE_GUI
double cur_mouse_x, cur_mouse_y, cur_mouse_angle, cur_mouse_speed;
#endif

/* mouse upload time from start to goal and back to start.
 * mouse time is caculated every timer expiration.
 */
static double mouse_total_time;
/* periodic simulation timer total time. it's compared with mouse
 * total time and decide when draw and algorithm would be triggered.
 */
static double simul_timer_time;

static void run_mouse_search(double *mouse_x, double *mouse_y,
		double *mouse_angle, double *mouse_speed,
		unsigned char mouse_pos, unsigned char direction);

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

static void simul_mouse_search_goal(char *maze_file, char target)
{
	struct s_link *f_node;
	unsigned char *path_array, wall, direction;
	enum SEARCH_TYPE search_type;

	print_dbg(DEBUG_SEARCH, "%s\n", __func__);

	/* FIXME: make below to be common map configuration */
	if (target == 0)
		search_type = TO_GOAL_16X16;
	else if (target == 1)
		search_type = TO_START_16X16;
	else
		return;

	/* find next block from the algorithm based on current
	 * maze information. reurn array will have FRBL type.
	 */
	path_array = find_maze_fastest_path(maze_file,
		cur_mouse_pos, cur_mouse_dir, search_type, &f_node);

	/* smooth turn only at this point. Search run won't have diagonal
	 * path run until certain point on my plan. Not so long.
	 */
	direction = *path_array;
	cur_mouse_dir = absolute_direction(cur_mouse_dir, direction);

	print_dbg(DEBUG_SEARCH, "Move mouse: %C\n",
				(direction == FD) ? 'F' : \
				((direction == RD) ? 'R' : \
				((direction == BD) ? 'B' : \
				((direction == LD) ? 'L' : 'X'))));
#ifdef MAZE_GUI
	/* Decide the distance and velocity of mouse to run.
	 * All the turns, speed and distance are simulated.
	 */
	if (cur_mouse_pos == 0x00) {
		/* Initialize mouse state */
		cur_mouse_x = 0.0;
		cur_mouse_y = 0.0;
		cur_mouse_angle = 0.0;
		cur_mouse_speed = 0.0;
		cur_mouse_dir = NI;
	}
	/* FIXME: modify to have known path in below function.
	 * Just make one block turns at this point.
	 */
	run_mouse_search(&cur_mouse_x, &cur_mouse_y, &cur_mouse_angle,
			&cur_mouse_speed, cur_mouse_pos, direction);
#endif

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

	/* cur_mouse_dir has moved to next block */
	if ((maze_search[cur_mouse_pos]&0xF0) != 0xF0) {
		wall = read_maze_block(maze_file, cur_mouse_pos);
		save_wallinfo_to_maze(cur_mouse_pos, wall);
#ifdef MAZE_GUI
		/* fill the wall color to check that the wall has read */
		fill_wall(cur_mouse_pos, wall);
#endif
	}

	print_dbg(DEBUG_SEARCH, "new position X:%d,Y:%d\n",
			pos_x(cur_mouse_pos), pos_y(cur_mouse_pos));
	print_map(maze_search);

	/* this must be called before starting new mapping.
	 * it's to free contour tree nodes.
	 */
	free_top_node_contour_tree();

#ifdef MAZE_GUI
	if (cur_mouse_pos == 0x00) {
		/* Initialize mouse state */
		cur_mouse_x = 0.0;
		cur_mouse_y = 0.0;
		cur_mouse_angle = 0.0;
		cur_mouse_speed = 0.0;
		cur_mouse_dir = NI;
	}
#endif
}

/* FIXME: 1st search to goal has to priotize diagonal path
 * Only 1st goal search will do search work on TOGOAL run.
 * All returns will check if there is other unknown fastest
 * path.
 */
int simul_mouse_search_run(char *maze_file)
{
	static char current_target;
	static char run_stage = 1;

	simul_timer_time += TIMER_S;

	/* Draw mouse from the buffer */
	draw_mouse_running(simul_timer_time);

	/* if mouse is still running */
	if (mouse_total_time > simul_timer_time) {
		/* draw mouse run from the footprint array */
		return run_stage;
	}

	/* start looking for goal with search run */
	if (current_target == 0) {
		simul_mouse_search_goal(maze_file, current_target);
		if (is_goal(cur_mouse_pos)) {
			current_target = 1;
			return run_stage;
		}
	} else if (current_target == 1) {
		/* return to start with search run */
		simul_mouse_search_goal(maze_file, current_target);
		if (cur_mouse_pos == 0) {
			/* Draw mouse from the buffer */
			do {
				simul_timer_time += TIMER_S;
			} while (draw_mouse_running(simul_timer_time));
			current_target = 0;
			run_stage++;
			return run_stage;
		}
	}

	return 0;
}

#define DRAW_TIME   ((double)0.05) /* sec */
#define SEARCH_TURN_VELOCITY    ((double)800.0)  /* mm/s */
#define SEARCH_ACCEL            ((double)16000.0)  /* mm/s^2 */
#define SIMUL_DIST_RATE         ((double)BLOCK_LEN/(double)BLOCK)
double total_draw_time;
void run_mouse_search(double *mouse_x, double *mouse_y,
		double *mouse_angle, double *mouse_speed,
		unsigned char mouse_pos, unsigned char direction)
{
	double dist;
	double target_v;
	double total_dist = 0.0;
	double total_time = 0.0;
	double delta_dist;
	double x, y, angle;

	if (mouse_pos == 0.0 && direction == FD &&
		*mouse_speed == 0.0 && *mouse_angle == 0.0) {
		dist = BLOCK/2.0;
		target_v = SEARCH_TURN_VELOCITY;
	} else if (direction == FD) {
		dist = BLOCK;
		target_v = SEARCH_TURN_VELOCITY;
	} else if (direction == RD) {
		dist = BLOCK*PI/4.0;
		target_v = SEARCH_TURN_VELOCITY;
	} else if (direction == LD) {
		dist = BLOCK*PI/4.0;
		target_v = SEARCH_TURN_VELOCITY;
		target_v = SEARCH_TURN_VELOCITY;
	} else if (direction == BD) {
		target_v = SEARCH_TURN_VELOCITY;
	} else {
		print_exit("%s: error!\n", __func__);
	}

	/* forward direction */
	if (direction == FD) {
		while (total_dist < dist && *mouse_speed < target_v) {
			total_time += TPERIOD;
			mouse_total_time += TPERIOD;
			delta_dist = (*mouse_speed * TPERIOD +
			   0.5 * SEARCH_ACCEL * TPERIOD * TPERIOD);
			total_dist += delta_dist;
			*mouse_speed += (SEARCH_ACCEL * TPERIOD);

			/* calculate x, y position */
			if (*mouse_angle == 0.0)
				*mouse_y += (delta_dist * SIMUL_DIST_RATE);
			else if (*mouse_angle == 90.0)
				*mouse_x += (delta_dist * SIMUL_DIST_RATE);
			else if (*mouse_angle == 180.0)
				*mouse_y -= (delta_dist * SIMUL_DIST_RATE);
			else if (*mouse_angle == 270.0)
				*mouse_x -= (delta_dist * SIMUL_DIST_RATE);

			if (mouse_total_time >= total_draw_time + DRAW_TIME) {
				/* save footprint to buffer */
				put_mouse_running((int)*mouse_x, (int)*mouse_y,
						(int)*mouse_angle, mouse_total_time);
				total_draw_time += DRAW_TIME;
			}
		}

		if (total_dist >= dist)
			return;
		else {
			while (total_dist < dist) {
				total_time += TPERIOD;
				mouse_total_time += TPERIOD;
				delta_dist = (*mouse_speed * TPERIOD +
				   0.5 * SEARCH_ACCEL * TPERIOD * TPERIOD);
				total_dist += delta_dist;

				if (*mouse_angle == 0.0)
					*mouse_y += (delta_dist * SIMUL_DIST_RATE);
				else if (*mouse_angle == 90.0)
					*mouse_x += (delta_dist * SIMUL_DIST_RATE);
				else if (*mouse_angle == 180.0)
					*mouse_y -= (delta_dist * SIMUL_DIST_RATE);
				else if (*mouse_angle == 270.0)
					*mouse_x -= (delta_dist * SIMUL_DIST_RATE);

				if (mouse_total_time >= total_draw_time + DRAW_TIME) {
					put_mouse_running((int)*mouse_x, (int)*mouse_y,
							(int)*mouse_angle, mouse_total_time);
					total_draw_time += DRAW_TIME;
				}
			}
		}
	} else if (direction == RD) { /* right turn */
		while (total_dist < dist) {
			total_time += TPERIOD;
			mouse_total_time += TPERIOD;
			delta_dist = (*mouse_speed * TPERIOD);
			total_dist += delta_dist;

			if (mouse_total_time >= total_draw_time + DRAW_TIME) {
				angle = 360.0 * total_dist / (BLOCK * PI);

				if (*mouse_angle == 0.0) {
					x = BLOCK/2.0*(1.0-cos(degree_to_radian(angle)));
					y = BLOCK/2.0*sin(degree_to_radian(angle));
					x = *mouse_x + x * SIMUL_DIST_RATE;
					y = *mouse_y + y * SIMUL_DIST_RATE;
				} else if (*mouse_angle == 90.0) {
					x = BLOCK/2.0*sin(degree_to_radian(angle));
					y = BLOCK/2.0*(1.0-cos(degree_to_radian(angle)));
					x = *mouse_x + x * SIMUL_DIST_RATE;
					y = *mouse_y - y * SIMUL_DIST_RATE;
					*mouse_x += (delta_dist * SIMUL_DIST_RATE);
				} else if (*mouse_angle == 180.0) {
					y = BLOCK/2.0*sin(degree_to_radian(angle));
					x = BLOCK/2.0*(1.0-cos(degree_to_radian(angle)));
					x = *mouse_x - x * SIMUL_DIST_RATE;
					y = *mouse_y - y * SIMUL_DIST_RATE;
					*mouse_y -= (delta_dist * SIMUL_DIST_RATE);
				} else if (*mouse_angle == 270.0) {
					x = BLOCK/2.0*sin(degree_to_radian(angle));
					y = BLOCK/2.0*(1.0-cos(degree_to_radian(angle)));
					x = *mouse_x - x * SIMUL_DIST_RATE;
					y = *mouse_y + y * SIMUL_DIST_RATE;
					*mouse_x -= (delta_dist * SIMUL_DIST_RATE);
				} else {
					 printf("%s: error\n", __func__);
				}

				angle += *mouse_angle;
				if (angle > 360.0)
					angle -= 360.0;
				put_mouse_running((int)x, (int)y,
						(int)angle, mouse_total_time);
				total_draw_time += DRAW_TIME;
			}
		}
		/* re-calculate mouse positiona at out */
		if (*mouse_angle == 0.0) {
			*mouse_x +=  (double)BLOCK_LEN/2.0;
			*mouse_y +=  (double)BLOCK_LEN/2.0;
			*mouse_angle += 90.0;
		} else if (*mouse_angle == 90.0) {
			*mouse_x +=  (double)BLOCK_LEN/2.0;
			*mouse_y -=  (double)BLOCK_LEN/2.0;
			*mouse_angle += 90.0;
		} else if (*mouse_angle == 180.0) {
			*mouse_x -=  (double)BLOCK_LEN/2.0;
			*mouse_y -=  (double)BLOCK_LEN/2.0;
			*mouse_angle += 90.0;
		} else if (*mouse_angle == 270.0) {
			*mouse_x -=  (double)BLOCK_LEN/2.0;
			*mouse_y +=  (double)BLOCK_LEN/2.0;
			*mouse_angle = 0.0;
		}
		put_mouse_running((int)*mouse_x, (int)*mouse_y,
			(int)*mouse_angle, mouse_total_time);

	} else if (direction == LD) { /* left turn */
		while (total_dist < dist) {
			total_time += TPERIOD;
			mouse_total_time += TPERIOD;
			delta_dist = (*mouse_speed * TPERIOD);
			total_dist += delta_dist;

			if (mouse_total_time >= total_draw_time + DRAW_TIME) {
				angle = 360.0 * total_dist / (BLOCK * PI);

				if (*mouse_angle == 0.0) {
					x = BLOCK/2.0*(1.0-cos(degree_to_radian(angle)));
					y = BLOCK/2.0*sin(degree_to_radian(angle));
					x = *mouse_x - x * SIMUL_DIST_RATE;
					y = *mouse_y + y * SIMUL_DIST_RATE;
				} else if (*mouse_angle == 90.0) {
					y = BLOCK/2.0*(1.0-cos(degree_to_radian(angle)));
					x = BLOCK/2.0*sin(degree_to_radian(angle));
					x = *mouse_x + x * SIMUL_DIST_RATE;
					y = *mouse_y + y * SIMUL_DIST_RATE;
					*mouse_x += (delta_dist*SIMUL_DIST_RATE);
				} else if (*mouse_angle == 180.0) {
					x = BLOCK/2.0*(1.0-cos(degree_to_radian(angle)));
					y = BLOCK/2.0*sin(degree_to_radian(angle));
					x = *mouse_x + x * SIMUL_DIST_RATE;
					y = *mouse_y - y * SIMUL_DIST_RATE;
					*mouse_y -= (delta_dist*SIMUL_DIST_RATE);
				} else if (*mouse_angle == 270.0) {
					y = BLOCK/2.0*(1.0-cos(degree_to_radian(angle)));
					x = BLOCK/2.0*sin(degree_to_radian(angle));
					x = *mouse_x - x * SIMUL_DIST_RATE;
					y = *mouse_y - y * SIMUL_DIST_RATE;
					*mouse_x -= (delta_dist*SIMUL_DIST_RATE);
				} else {
					 printf("%s: error\n", __func__);
				}

				angle = 360.0 + *mouse_angle - angle;
				if (angle > 360.0)
					angle -= 360.0;
				put_mouse_running((int)x, (int)y,
					(int)angle, mouse_total_time);
				total_draw_time += DRAW_TIME;
			}
		}

		/* re-calculate mouse positiona at out */
		if (*mouse_angle == 0.0) {
			*mouse_x -=  (double)BLOCK_LEN/2.0;
			*mouse_y +=  (double)BLOCK_LEN/2.0;
			*mouse_angle = 270.0;
		} else if (*mouse_angle == 90.0) {
			*mouse_x +=  (double)BLOCK_LEN/2.0;
			*mouse_y +=  (double)BLOCK_LEN/2.0;
			*mouse_angle = 0.0;
		} else if (*mouse_angle == 180.0) {
			*mouse_x +=  (double)BLOCK_LEN/2.0;
			*mouse_y -=  (double)BLOCK_LEN/2.0;
			*mouse_angle = 90.0;
		} else if (*mouse_angle == 270.0) {
			*mouse_x -=  (double)BLOCK_LEN/2.0;
			*mouse_y -=  (double)BLOCK_LEN/2.0;
			*mouse_angle = 180.0;
		}
		put_mouse_running((int)*mouse_x, (int)*mouse_y,
			(int)*mouse_angle, mouse_total_time);

	} else if (direction == BD) { /* left turn */
		*mouse_angle += 180.0;
		if (*mouse_angle >= 360.0)
			*mouse_angle -= 360.0;
		put_mouse_running((int)*mouse_x, (int)*mouse_y,
			(int)*mouse_angle, mouse_total_time);
	}
}


