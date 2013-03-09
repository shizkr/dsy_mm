/*
 * Support drawing polls and walls and maze
 *
 * Copyright (c) 2013 Denny Yang <denny.ds.yang@gmail.com>
 *
 */

#include <gtk/gtk.h>
#include "algo.h"
#include "draw_maze.h"
#include "maze.h"

/*
 * Set color definition
 */
#define WALL_COLOR       "#961A6B"
#define WALL_FILL_COLOR  "#521154"
#define POLL_COLOR       "#C20C0C"
#define MOUSE_COLOR      "#52B154"
#define MOUSE_FP_COLOR   "#121194"

/* Draw a poll of maze
 * x : poll x index 0 ~ 15
 * y : poll y index 0 ~ 15
 *
 *      +---+
 *      |\ /|
 *      |/ \|
 *      +---+
 */
static void draw_poll(struct components *this, GdkGC *gc, int x, int y)
{
	int real_x = WIN_X_OFFSET + x * BLOCK_LEN;
	int real_y = WIN_Y_OFFSET + (16-y) * BLOCK_LEN;
	struct rectangle poll;

	poll.xl = real_x - (POLL_LEN/2);
	poll.xr = real_x + (POLL_LEN/2);
	poll.yt = real_y - (POLL_LEN/2);
	poll.yb = real_y + (POLL_LEN/2);

	gdk_draw_line(this->pixMap, gc,      /* X-X  */
			poll.xl, poll.yt, poll.xr, poll.yt);
	gdk_draw_line(this->pixMap, gc,      /* X| X */
			poll.xl, poll.yt, poll.xl, poll.yb);
	gdk_draw_line(this->pixMap, gc,      /* X |X */
			poll.xr, poll.yt, poll.xr, poll.yb);
	gdk_draw_line(this->pixMap, gc,      /* X_X  */
			poll.xl, poll.yb, poll.xr, poll.yb);
	gdk_draw_line(this->pixMap, gc,      /* X\X  */
			poll.xl, poll.yt, poll.xr, poll.yb);
	gdk_draw_line(this->pixMap, gc,      /* X/X  */
			poll.xl, poll.yb, poll.xr, poll.yt);
}

static void draw_wall_north(struct components *this, GdkGC *gc,
		int x, int y, char fill)
{
	int real_x = WIN_X_OFFSET + x*BLOCK_LEN + BLOCK_LEN/2;
	int real_y = WIN_Y_OFFSET + (15-y)*BLOCK_LEN + BLOCK_LEN/2;

	struct dot lt, rt, lb, rb;

	lt.x = real_x - (WALL_LEN/2);
	lt.y = real_y - (WALL_LEN/2) - POLL_LEN;
	rt.x = real_x + (WALL_LEN/2);
	rt.y = real_y - (WALL_LEN/2) - POLL_LEN;
	lb.x = real_x - (WALL_LEN/2);
	lb.y = real_y - (WALL_LEN/2);
	rb.x = real_x + (WALL_LEN/2);
	rb.y = real_y - (WALL_LEN/2);

	/* Fill the wall when fill is set */
	if (fill) {
		gdk_draw_rectangle(this->pixMap, gc,
			TRUE, lt.x+1, lt.y+1, WALL_LEN-1, POLL_LEN-1);
	} else {
		gdk_draw_line(this->pixMap, gc,
					lt.x, lt.y, rt.x, rt.y);
		gdk_draw_line(this->pixMap, gc,
					lb.x, lb.y, rb.x, rb.y);
	}
}

static void draw_wall_east(struct components *this, GdkGC *gc,
		int x, int y, char fill)
{
	int real_x = WIN_X_OFFSET + x*BLOCK_LEN + BLOCK_LEN/2;
	int real_y = WIN_Y_OFFSET + (15-y)*BLOCK_LEN + BLOCK_LEN/2;

	struct dot lt, rt, lb, rb;

	lt.x = real_x + (WALL_LEN/2);
	lt.y = real_y - (WALL_LEN/2);
	rt.x = real_x + (WALL_LEN/2) + POLL_LEN;
	rt.y = real_y - (WALL_LEN/2);
	lb.x = real_x + (WALL_LEN/2);
	lb.y = real_y + (WALL_LEN/2);
	rb.x = real_x + (WALL_LEN/2) + POLL_LEN;
	rb.y = real_y + (WALL_LEN/2);

	/* Fill the wall when fill variable is set */
	if (fill) {
		gdk_draw_rectangle(this->pixMap, gc,
			TRUE, lt.x+1, lt.y+1, POLL_LEN-1, WALL_LEN-1);
	} else {
		gdk_draw_line(this->pixMap, gc,
					lt.x, lt.y, lb.x, lb.y);
		gdk_draw_line(this->pixMap, gc,
					rt.x, rt.y, rb.x, rb.y);
	}
}

static void draw_wall_south(struct components *this, GdkGC *gc,
		int x, int y, char fill)
{
	int real_x = WIN_X_OFFSET + x*BLOCK_LEN + BLOCK_LEN/2;
	int real_y = WIN_Y_OFFSET + (15-y)*BLOCK_LEN + BLOCK_LEN/2;

	struct dot lt, rt, lb, rb;

	lt.x = real_x - (WALL_LEN/2);
	lt.y = real_y + (WALL_LEN/2);
	rt.x = real_x + (WALL_LEN/2);
	rt.y = real_y + (WALL_LEN/2);
	lb.x = real_x - (WALL_LEN/2);
	lb.y = real_y + (WALL_LEN/2) + POLL_LEN;
	rb.x = real_x + (WALL_LEN/2);
	rb.y = real_y + (WALL_LEN/2) + POLL_LEN;

	/* Fill the wall when fill is set */
	if (fill) {
		gdk_draw_rectangle(this->pixMap, gc,
			TRUE, lt.x+1, lt.y+1, WALL_LEN-1, POLL_LEN-1);
	} else {
		gdk_draw_line(this->pixMap, gc,
					lt.x, lt.y, rt.x, rt.y);
		gdk_draw_line(this->pixMap, gc,
					lb.x, lb.y, rb.x, rb.y);
	}
}

static void draw_wall_west(struct components *this, GdkGC *gc,
		int x, int y, char fill)
{
	int real_x = WIN_X_OFFSET + x*BLOCK_LEN + BLOCK_LEN/2;
	int real_y = WIN_Y_OFFSET + (15-y)*BLOCK_LEN + BLOCK_LEN/2;

	struct dot lt, rt, lb, rb;

	lt.x = real_x - (WALL_LEN/2) - POLL_LEN;
	lt.y = real_y - (WALL_LEN/2);
	rt.x = real_x - (WALL_LEN/2);
	rt.y = real_y - (WALL_LEN/2);
	lb.x = real_x - (WALL_LEN/2) - POLL_LEN;
	lb.y = real_y + (WALL_LEN/2);
	rb.x = real_x - (WALL_LEN/2);
	rb.y = real_y + (WALL_LEN/2);

	/* Fill the wall when fill is set */
	if (fill) {
		gdk_draw_rectangle(this->pixMap, gc,
			TRUE, lt.x+1, lt.y+1, POLL_LEN-1, WALL_LEN-1);
	} else {
		gdk_draw_line(this->pixMap, gc,
					lt.x, lt.y, lb.x, lb.y);
		gdk_draw_line(this->pixMap, gc,
					rt.x, rt.y, rb.x, rb.y);
	}
}

static void draw_wall(struct components *this,
		int x, int y, unsigned char wall)
{
	GdkGC *gc = gui_set_color(WALL_COLOR);

	if (wall & NORTH)
		draw_wall_north(this, gc, x, y, 0);
	if (wall & EAST)
		draw_wall_east(this, gc, x, y, 0);
	if (wall & SOUTH)
		draw_wall_south(this, gc, x, y, 0);
	if (wall & WEST)
		draw_wall_west(this, gc, x, y,  0);
}

static void draw_full_poll(struct components *this)
{
	int x, y;

	/* Poll edge color */
	GdkGC *gc = gui_set_color(POLL_COLOR);

	for (x = 0; x <= 16; x++)
		for (y = 0; y <= 16; y++)
			draw_poll(this, gc, x, y);
}

void draw_full_maze(struct components *this,
		char *maze)
{
	int i;

	draw_full_poll(this);

	for (i = 0; i < MAZEMAX; i++) {
		draw_wall(this, pos_x(i), pos_y(i),
				maze[i]);
	}

	gtk_widget_queue_draw_area(this->drawingArea,
		0, 0, WIN_MAX_X, WIN_MAX_Y);
}

/* When the mouse reads an wall, fill the wall */
void fill_wall(unsigned char index, unsigned char wall)
{
	GdkGC *gc = gui_set_color(WALL_FILL_COLOR);

	if (wall & NORTH)
		draw_wall_north(&this, gc, pos_x(index),
				pos_y(index), 1);
	if (wall & EAST)
		draw_wall_east(&this, gc, pos_x(index),
				pos_y(index), 1);
	if (wall & SOUTH)
		draw_wall_south(&this, gc, pos_x(index),
				pos_y(index), 1);
	if (wall & WEST)
		draw_wall_west(&this, gc, pos_x(index),
				pos_y(index), 1);
}
