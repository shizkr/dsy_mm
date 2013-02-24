#ifndef DRAWMAZE_H
#define DRAWMAZE_H

#include "maze.h"

#define START_WIN_X 20
#define START_WIN_Y 20

#define BLOCK_LEN   48
#define POLL_LEN    6
#define WALL_LEN    (BLOCK_LEN-POLL_LEN)

#define WIN_MAX_X   820
#define WIN_MAX_Y   820

void draw_full_maze(struct components *this, char *maze);
#endif
