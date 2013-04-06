/*
 * Support drawing polls and walls and maze
 *
 * Copyright (c) 2013 Denny Yang <denny.ds.yang@gmail.com>
 *
 */

#ifndef DRAWMAZE_H
#define DRAWMAZE_H

#include "maze.h"

#define WIN_X_OFFSET 20
#define WIN_Y_OFFSET 20

#define BLOCK_LEN   48
#define POLL_LEN    6
#define WALL_LEN    (BLOCK_LEN-POLL_LEN)

#define WIN_MAX_X   820
#define WIN_MAX_Y   820

struct dot {
	int x;
	int y;
};

struct line {
	int start_x;
	int start_y;
	int end_x;
	int end_y;
};

struct rectangle {
	int xl;
	int xr;
	int yt;
	int yb;
};

void draw_full_maze(struct components *this, unsigned char *maze);
void fill_wall(unsigned char index, unsigned char wall);

#endif
