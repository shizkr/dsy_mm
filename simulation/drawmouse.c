#include <gtk/gtk.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include "drawmaze.h"
#include "drawmouse.h"
#include "maze.h"
#include "circular_buffer.h"

#define TAG "DRAWMOUSE: "
#include "debug.h"

#ifdef DEBUG
static int debug_flag = DEBUG_SEARCH | DEBUG_BINTREE | DEBUG_S_LINK;
#endif

#define M_LENGTH  24
#define M_WIDTH   10

struct mouse_shape {
	double angle;
	int lx;
	int ly;
	int tx;
	int ty;
	int rx;
	int ry;
	int bx;
	int by;
	int asx; /* draw area */
	int asy;
	int aex;
	int aey;
};

/* circular buffer of running mouse points.
 * Draw these by timer caculation.
 */
struct footprint {
	int x;
	int y;
	int angle;
	double ms;  /* ms */
};

#define MAX_MOUSE_FOOTPRINT  1024
struct circular_buffer *mouse_footprint;

void draw_mouse(int x, int y, int angle)
{
	int zero_x = START_WIN_X + BLOCK_LEN/2;
	int zero_y = START_WIN_Y + 15*BLOCK_LEN + BLOCK_LEN/2;
	int rx, ry;
	double cal_x, cal_y;
	static int init;
	static struct mouse_shape m;

	print_dbg(DEBUG_SEARCH, "%s: %d, %d\n", __func__, x, y);
	if (!init)
		init = 1;
	else {
		gdk_draw_line(this.pixMap, this.drawingArea->style->white_gc,
					m.lx, m.ly, m.tx, m.ty);
		gdk_draw_line(this.pixMap, this.drawingArea->style->white_gc,
					m.tx, m.ty, m.rx, m.ry);
		gdk_draw_line(this.pixMap, this.drawingArea->style->white_gc,
					m.tx, m.ty, m.bx, m.by);
		gtk_widget_queue_draw_area(this.drawingArea,
			m.asx, m.asy, m.aex, m.aey);
	}
	rx = zero_x + x;
	ry = zero_y - y;

	cal_x = sin(degree_to_radian((double)angle)) *
		(double)(M_LENGTH/2);
	cal_y = cos(degree_to_radian((double)angle)) *
		(double)(M_LENGTH/2);
	m.tx = rx + (int)cal_x;
	m.ty = ry - (int)cal_y;
	m.bx = rx - (int)cal_x;
	m.by = ry + (int)cal_y;

	cal_x = sin(degree_to_radian((double)angle)+45.0) *
		(double)(M_WIDTH/2);
	cal_y = cos(degree_to_radian((double)angle)+45.0) *
		(double)(M_WIDTH/2);
	m.rx = rx + (int)cal_x;
	m.ry = ry - (int)cal_y;

	cal_x = cos(degree_to_radian((double)angle)+315.0) *
		(double)(M_WIDTH/2);
	cal_y = sin(degree_to_radian((double)angle)+315.0) *
		(double)(M_WIDTH/2);
	m.lx = rx - (int)cal_x;
	m.ly = ry - (int)cal_y;

	m.asx = rx - M_LENGTH;
	m.asy = rx - M_LENGTH;
	m.aex = rx + M_LENGTH;
	m.aey = rx + M_LENGTH;

	gdk_draw_line(this.pixMap, this.drawingArea->style->black_gc,
				m.lx, m.ly, m.tx, m.ty);
	gdk_draw_line(this.pixMap, this.drawingArea->style->black_gc,
				m.tx, m.ty, m.rx, m.ry);
	gdk_draw_line(this.pixMap, this.drawingArea->style->black_gc,
				m.tx, m.ty, m.bx, m.by);
	/*
	gtk_widget_queue_draw_area(this.drawingArea,
			m.asx, m.asy, m.aex, m.aey);
	*/
	gtk_widget_queue_draw_area(this.drawingArea,
			0, 0, 900, 900);
}

/* Put the footprint of mouse into buffer
 * time : Second unti
 */
void put_mouse_running(int x, int y, int angle, double time)
{
	unsigned int *buffer;
	struct footprint *item;

	if (mouse_footprint == NULL) {
		buffer = malloc(sizeof(unsigned int *) * MAX_MOUSE_FOOTPRINT);
		mouse_footprint = malloc(sizeof(struct circular_buffer));
		if (buffer == NULL || mouse_footprint == NULL)
			print_exit("%s:malloc failure\n", __func__);
		circular_buffer_init(mouse_footprint,
				buffer, MAX_MOUSE_FOOTPRINT);
	}

	item = malloc(sizeof(struct footprint));
	if (item == NULL)
		print_exit("%s:malloc failure\n", __func__);
	item->x = x;
	item->y = y;
	item->angle = angle;
	item->ms = time;
	circular_buffer_write(mouse_footprint, (unsigned int)item);

	print_dbg(DEBUG_SEARCH, "%s X:%d, Y:%d, ang:%d, time:%f\n",
			__func__, x, y, angle, time);
}

/* Draw footprint in the buffer if the saved time in the item
 * is smaller than total timer value.
 */
int draw_mouse_running(double total)
{
	int cnt = 0;
	struct footprint *item;

	if (!mouse_footprint || circular_buffer_empty(mouse_footprint))
		return 0;

	do {
		circular_buffer_read(mouse_footprint, (unsigned int *)&item);
		cnt++;

		/* draw footprint and mouse */
		/* FIXME: draw footprint */
		draw_mouse(item->x, item->y, item->angle);

		if (item->ms >= total) {
			free(item);
			break;
		}

		free(item);

	} while (!circular_buffer_empty(mouse_footprint));

	return cnt;
}
