#include <gtk/gtk.h>
#include <math.h>
#include "drawmaze.h"
#include "drawmouse.h"
#include "maze.h"

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

void draw_mouse(int x, int y, int angle)
{
	int zero_x = START_WIN_X + BLOCK_LEN/2;
	int zero_y = START_WIN_Y + 15*BLOCK_LEN + BLOCK_LEN/2;
	int rx, ry;
	double cal_x, cal_y;
	static int init;
	static struct mouse_shape m;

	printf("%s: %d, %d\n", __func__, x, y);
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
