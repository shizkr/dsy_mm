#ifndef DRAWMOUSE_H
#define DRAWMOUSE_H

#include "diagonal.h"

#define degree_to_radian(x) (x*(PI/180.0))

void draw_mouse(int x, int y, int angle);
/* Put the footprint of mouse into buffer */
void put_mouse_running(int x, int y, int angle, double time);
/* Draw footprint in the buffer if the saved time in the item
 * is smaller than total timer value.
 */
int draw_mouse_running(double total);

#endif
