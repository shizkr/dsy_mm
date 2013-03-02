#include <gtk/gtk.h>
#include "algo.h"
#include "maze.h"
#include "drawmaze.h"

struct components this;

static void next_handler(GtkWidget *widget,
                   gpointer data)
{
    g_print ("next %s \n", (gchar *)data);
}

static void previous_handler(GtkWidget *widget,
                   gpointer data)
{
    g_print ("previous %s \n", (gchar *)data);
}

static gboolean delete_event( GtkWidget *widget,
                              GdkEvent  *event,
                              gpointer   data )
{
    return FALSE;
}

/* Another callback */
static void destroy( GtkWidget *widget,
                     gpointer   data )
{
    gtk_main_quit ();
}


static void createWindow() {
    this.window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
    gtk_container_set_border_width (GTK_CONTAINER (this.window), 10);
    gtk_window_set_title (GTK_WINDOW (this.window), "MICROMOUSE SIMULATOR");

    g_signal_connect (G_OBJECT (this.window), "delete_event",
              G_CALLBACK (delete_event), NULL);

    g_signal_connect (G_OBJECT (this.window), "destroy",
                G_CALLBACK (destroy), NULL);

}


static void createButtons() {
    this.next = gtk_button_new_with_label ("Next");
    this.previous = gtk_button_new_with_label ("Previous");

    g_signal_connect (G_OBJECT (this.next), "clicked", G_CALLBACK (next_handler), NULL);
    g_signal_connect (G_OBJECT (this.previous), "clicked", G_CALLBACK (previous_handler), NULL);

}

static void layoutWidgets() {
    /* Create the graph navigation panel and add it to the window. */
    this.graphNavigationPanel = gtk_hbox_new (FALSE, 0);
    this.mainPanel = gtk_vbox_new(FALSE, 0);

    gtk_container_add  (GTK_CONTAINER (this.window), this.mainPanel);


    /* Add the buttons to the graph navigation panel. */
    gtk_box_pack_start (GTK_BOX(this.graphNavigationPanel), this.previous, TRUE, TRUE, 0);
    gtk_box_pack_start (GTK_BOX(this.graphNavigationPanel), this.next,     TRUE, TRUE, 0);


    /*Add the graph navigation panel to the main panel. */
    gtk_box_pack_start (GTK_BOX(this.mainPanel), this.graphNavigationPanel, TRUE, TRUE, 0);
    /* Add the draw-able area to the main panel. */
    gtk_box_pack_start (GTK_BOX(this.mainPanel), this.drawingArea, TRUE, TRUE, 0);
}

static void show()
{
	gtk_widget_show(this.drawingArea);
	gtk_widget_show(this.mainPanel);
	gtk_widget_show(this.next);
	gtk_widget_show(this.previous);
	gtk_widget_show(this.graphNavigationPanel);
	gtk_widget_show(this.window);
}

/* Redraw the screen from the backing pixmap */
static gboolean
expose_event( GtkWidget *widget, GdkEventExpose *event )
{
  gdk_draw_drawable(widget->window,
            widget->style->fg_gc[GTK_WIDGET_STATE (widget)],
            this.pixMap,
            event->area.x, event->area.y,
            event->area.x, event->area.y,
            event->area.width, event->area.height);

  return FALSE;
}
/* Create a new backing pixmap of the appropriate size */
static gboolean
configure_event( GtkWidget *widget, GdkEventConfigure *event )
{
  if (this.pixMap)
    g_object_unref(this.pixMap);

  this.pixMap = gdk_pixmap_new(widget->window,
              widget->allocation.width,
              widget->allocation.height,
              -1);

  gdk_draw_rectangle (this.pixMap,
              widget->style->white_gc,
              TRUE,
              0, 0,
              widget->allocation.width,
              widget->allocation.height);

  return TRUE;
}

static gboolean button_press_event(GtkWidget *widget, GdkEventButton *event)
{
	if (event->button == 1 && this.pixMap != NULL)
		;

	return TRUE;
}

static void createDrawingArea() {
    this.drawingArea = gtk_drawing_area_new();
	GdkGC *gc = this.drawingArea->style->
		fg_gc[GTK_WIDGET_STATE(this.drawingArea)];
	GdkVisual *visual = gtk_widget_get_visual(this.drawingArea);
	GdkColormap *colour_map = gdk_colormap_new(visual, TRUE);
	GdkColor background_colour, light_edge, dark_edge, text_colour;

    gtk_signal_connect (GTK_OBJECT (this.drawingArea), "expose_event",
                  (GtkSignalFunc) expose_event, NULL);
    gtk_signal_connect (GTK_OBJECT(this.drawingArea),"configure_event",
                  (GtkSignalFunc) configure_event, NULL);
    gtk_signal_connect (GTK_OBJECT (this.drawingArea), "button_press_event",
                  (GtkSignalFunc) button_press_event, NULL);

    gtk_widget_set_events (this.drawingArea, GDK_EXPOSURE_MASK
                 | GDK_LEAVE_NOTIFY_MASK
                 | GDK_BUTTON_PRESS_MASK
                 | GDK_POINTER_MOTION_MASK
                 | GDK_POINTER_MOTION_HINT_MASK);

    gtk_drawing_area_size(GTK_DRAWING_AREA(this.drawingArea), WIN_MAX_X, WIN_MAX_Y);

	/* color setting */
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
}

/* set foreground color */
GdkGC *gui_set_color(char *color)
{
	GdkGC *gc = this.drawingArea->style->
		fg_gc[GTK_WIDGET_STATE(this.drawingArea)];
	GdkVisual *visual = gtk_widget_get_visual(this.drawingArea);
	GdkColormap *colour_map = gdk_colormap_new(visual, TRUE);
	GdkColor light_edge;

	/* color setting */
	gdk_color_parse(color, &light_edge);
	gdk_colormap_alloc_color(colour_map, &light_edge, TRUE, TRUE);

	gdk_gc_set_fill(gc, GDK_SOLID);
	gdk_gc_set_foreground(gc, &light_edge);

	return gc;
}

void gui_init(int argc, char *argv[])
{
	gtk_init(&argc, &argv);

	createWindow();
	createButtons();
	createDrawingArea();
	layoutWidgets();

	show();
}

void init_draw_maze(char *maze)
{
	draw_full_maze(&this, maze);
}


