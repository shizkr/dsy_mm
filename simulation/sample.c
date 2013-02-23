#include <gtk/gtk.h>

int main(int   argc, char *argv[])
{
	GtkWidget *window, *button;

	gtk_init(&argc, &argv);

	window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_widget_set_size_request(window, 768, 768);
	gtk_widget_show(window);

	gtk_main();

	return 0;
}
