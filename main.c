#include <gtk/gtk.h>

typedef struct
{
	GtkWidget *window;
	GtkWidget *da;
} Gui;

void on_destroy (G_GNUC_UNUSED GtkWidget *widget, G_GNUC_UNUSED gpointer user_data)
{
	gtk_main_quit ();
}

int main (int argc, char **argv)
{
	Gui *gui = g_new0 (Gui, 1);
	gtk_init (&argc, &argv);

	gui->window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
	gui->da = gtk_drawing_area_new ();
	gtk_container_add (GTK_CONTAINER (gui->window), gui->da);
	gtk_widget_show_all (gui->window);
	g_signal_connect (gui->window, "destroy", G_CALLBACK (on_destroy), NULL);
	gtk_main ();
	return 0;
}

