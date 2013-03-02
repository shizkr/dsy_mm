#include <gtk/gtk.h>
#include "algo.h"
#include "drawmaze.h"
#include "maze.h"

/* x : poll x index 0 ~ 15
 * y : poll y index 0 ~ 15
 */
static void draw_poll(struct components *this, GdkGC *gc, int x, int y)
{
	int real_x = START_WIN_X + x * BLOCK_LEN;
	int real_y = START_WIN_Y + (16-y) * BLOCK_LEN;

	struct dot lt, rt, lb, rb;

	lt.x = real_x - (POLL_LEN/2);
	lt.y = real_y - (POLL_LEN/2);
	rt.x = real_x + (POLL_LEN/2);
	rt.y = real_y - (POLL_LEN/2);
	lb.x = real_x - (POLL_LEN/2);
	lb.y = real_y + (POLL_LEN/2);
	rb.x = real_x + (POLL_LEN/2);
	rb.y = real_y + (POLL_LEN/2);

	gdk_draw_line(this->pixMap, gc,
			lt.x, lt.y, rt.x, rt.y);
	gdk_draw_line(this->pixMap, gc,
			lt.x, lt.y, lb.x, lb.y);
	gdk_draw_line(this->pixMap, gc,
			rt.x, rt.y, rb.x, rb.y);
	gdk_draw_line(this->pixMap, gc,
			lb.x, lb.y, rb.x, rb.y);
	gdk_draw_line(this->pixMap, gc,
			lt.x, lt.y, rb.x, rb.y);
	gdk_draw_line(this->pixMap, gc,
			rt.x, rt.y, lb.x, lb.y);
}

static void draw_wall_north(struct components *this, GdkGC *gc,
		int x, int y, char fill)
{
	int real_x = START_WIN_X + x*BLOCK_LEN + BLOCK_LEN/2;
	int real_y = START_WIN_Y + (15-y)*BLOCK_LEN + BLOCK_LEN/2;

	struct dot lt, rt, lb, rb;

	lt.x = real_x - (WALL_LEN/2);
	lt.y = real_y - (WALL_LEN/2) - POLL_LEN;
	rt.x = real_x + (WALL_LEN/2);
	rt.y = real_y - (WALL_LEN/2) - POLL_LEN;
	lb.x = real_x - (WALL_LEN/2);
	lb.y = real_y - (WALL_LEN/2);
	rb.x = real_x + (WALL_LEN/2);
	rb.y = real_y - (WALL_LEN/2);

	/* fill the wall when fill is set */
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
	int real_x = START_WIN_X + x*BLOCK_LEN + BLOCK_LEN/2;
	int real_y = START_WIN_Y + (15-y)*BLOCK_LEN + BLOCK_LEN/2;

	struct dot lt, rt, lb, rb;

	lt.x = real_x + (WALL_LEN/2);
	lt.y = real_y - (WALL_LEN/2);
	rt.x = real_x + (WALL_LEN/2) + POLL_LEN;
	rt.y = real_y - (WALL_LEN/2);
	lb.x = real_x + (WALL_LEN/2);
	lb.y = real_y + (WALL_LEN/2);
	rb.x = real_x + (WALL_LEN/2) + POLL_LEN;
	rb.y = real_y + (WALL_LEN/2);

	/* fill the wall when fill is set */
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

/* x : x index value, usually 0~15
 * y : y index value, usually 0~15
 */
static void draw_wall_south(struct components *this, GdkGC *gc,
		int x, int y, char fill)
{
	int real_x = START_WIN_X + x*BLOCK_LEN + BLOCK_LEN/2;
	int real_y = START_WIN_Y + (15-y)*BLOCK_LEN + BLOCK_LEN/2;

	struct dot lt, rt, lb, rb;

	lt.x = real_x - (WALL_LEN/2);
	lt.y = real_y + (WALL_LEN/2);
	rt.x = real_x + (WALL_LEN/2);
	rt.y = real_y + (WALL_LEN/2);
	lb.x = real_x - (WALL_LEN/2);
	lb.y = real_y + (WALL_LEN/2) + POLL_LEN;
	rb.x = real_x + (WALL_LEN/2);
	rb.y = real_y + (WALL_LEN/2) + POLL_LEN;

	/* fill the wall when fill is set */
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
	int real_x = START_WIN_X + x*BLOCK_LEN + BLOCK_LEN/2;
	int real_y = START_WIN_Y + (15-y)*BLOCK_LEN + BLOCK_LEN/2;

	struct dot lt, rt, lb, rb;

	lt.x = real_x - (WALL_LEN/2) - POLL_LEN;
	lt.y = real_y - (WALL_LEN/2);
	rt.x = real_x - (WALL_LEN/2);
	rt.y = real_y - (WALL_LEN/2);
	lb.x = real_x - (WALL_LEN/2) - POLL_LEN;
	lb.y = real_y + (WALL_LEN/2);
	rb.x = real_x - (WALL_LEN/2);
	rb.y = real_y + (WALL_LEN/2);

	/* fill the wall when fill is set */
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
	GdkGC *gc = gui_set_color("#961A6B");

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
	GdkGC *gc = gui_set_color("#C20C0C");

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

/* when mouse reads an wall, make the walls to be filled */
void fill_wall(unsigned char index, unsigned char wall)
{
	GdkGC *gc = gui_set_color("#521154");

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
