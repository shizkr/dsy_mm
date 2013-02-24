#include <gtk/gtk.h>
#include "algo.h"
#include "drawmaze.h"
#include "maze.h"

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

/* x : poll x index 0 ~ 16
 * y : poll y index 0 ~ 16
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

static void draw_wall_north(struct components *this,
		int x, int y)
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

	gdk_draw_line(this->pixMap, this->drawingArea->style->black_gc,
				lt.x, lt.y, rt.x, rt.y);
	gdk_draw_line(this->pixMap, this->drawingArea->style->black_gc,
				lb.x, lb.y, rb.x, rb.y);
}

static void draw_wall_east(struct components *this,
		int x, int y)
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

	gdk_draw_line(this->pixMap, this->drawingArea->style->black_gc,
				lt.x, lt.y, lb.x, lb.y);
	gdk_draw_line(this->pixMap, this->drawingArea->style->black_gc,
				rt.x, rt.y, rb.x, rb.y);
}

static void draw_wall_south(struct components *this,
		int x, int y)
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

	gdk_draw_line(this->pixMap, this->drawingArea->style->black_gc,
				lt.x, lt.y, rt.x, rt.y);
	gdk_draw_line(this->pixMap, this->drawingArea->style->black_gc,
				lb.x, lb.y, rb.x, rb.y);
}

static void draw_wall_west(struct components *this,
		int x, int y)
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

	gdk_draw_line(this->pixMap, this->drawingArea->style->black_gc,
				lt.x, lt.y, lb.x, lb.y);
	gdk_draw_line(this->pixMap, this->drawingArea->style->black_gc,
				rt.x, rt.y, rb.x, rb.y);
}

static void draw_wall(struct components *this,
		int x, int y, unsigned char wall)
{
	if (wall & NORTH)
		draw_wall_north(this, x, y);
	if (wall & EAST)
		draw_wall_east(this, x, y);
	if (wall & SOUTH)
		draw_wall_south(this, x, y);
	if (wall & WEST)
		draw_wall_west(this, x, y);
}

static void draw_full_poll(struct components *this)
{
	int x, y;
	GdkGC *gc = this->drawingArea->style->fg_gc[GTK_WIDGET_STATE(this->drawingArea)];
	GdkVisual *visual = gtk_widget_get_visual(this->drawingArea);
	GdkColormap *colour_map = gdk_colormap_new(visual, TRUE);
	GdkColor background_colour, light_edge, dark_edge, text_colour;

	gdk_color_parse("#F5DEB3", &background_colour);
	gdk_colormap_alloc_color(colour_map, &background_colour, TRUE, TRUE);
	gdk_color_parse("#FF0000", &light_edge);
	gdk_colormap_alloc_color(colour_map, &light_edge, TRUE, TRUE);
	gdk_color_parse("#606060", &dark_edge);
	gdk_colormap_alloc_color(colour_map, &dark_edge, TRUE, TRUE);
	gdk_color_parse("#000000", &text_colour);
	gdk_colormap_alloc_color(colour_map, &text_colour, TRUE, TRUE);

	gdk_gc_set_foreground(gc, &background_colour);
	gdk_gc_set_fill(gc, GDK_SOLID);
	gdk_gc_set_foreground(gc, &light_edge);

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

