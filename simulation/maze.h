#ifndef MAZE_H
#define MAZE_H

struct components {
    GtkWidget *window;
    GtkWidget *previous;
    GtkWidget *next;
    GtkWidget *graphNavigationPanel;
    GtkWidget *mainPanel;
    GtkWidget *drawingArea;
    GdkPixmap *pixMap;
};

extern struct components this;

void gui_init(int argc, char *argv[]);
void init_draw_maze(char *maze);
#endif
