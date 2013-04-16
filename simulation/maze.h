#ifndef MAZE_H
#define MAZE_H

#include <gtk/gtk.h>

#define MOUSE_INIT    0
#define MOUSE_RUNNING 1
#define MOUSE_STOP    2

struct components {
    GtkWidget *window;
    GtkWidget *start;
    GtkWidget *stop;
    GtkWidget *graphNavigationPanel;
    GtkWidget *mainPanel;
    GtkWidget *drawingArea;
    GdkPixmap *pixMap;
};

extern struct components this;

void gui_init(int argc, char *argv[]);
void init_draw_maze(unsigned char *maze);
GdkGC *gui_set_color(char *color);

int get_mouse_run_gui_state(void);
void set_mouse_run_gui_state(int state);

#endif
