#include <gtk/gtk.h>

typedef struct
{
	GtkWidget *window;
	GtkWidget *da;
	int counter;
} Gui;

void on_destroy (G_GNUC_UNUSED GtkWidget *widget, G_GNUC_UNUSED gpointer user_data)
{
	gtk_main_quit ();
}

gboolean on_draw (G_GNUC_UNUSED GtkWidget *widget,
		cairo_t *cr,
		gpointer user_data)
{
	Gui *gui = user_data;
	char text[2];
	cairo_select_font_face (cr,
			"Sans",
			CAIRO_FONT_SLANT_NORMAL,
			CAIRO_FONT_WEIGHT_BOLD);
	cairo_set_font_size (cr, 120.0);
	cairo_move_to (cr, 10.0, 135.0);
	g_snprintf (text, sizeof (text), "%d", gui->counter + 1);
	cairo_show_text (cr, text);
	return TRUE;
}

gboolean
on_timeout (gpointer user_data)
{
	Gui *gui = user_data;
	GtkWidget *widget = gui->da;
	gui->counter++;
	gui->counter %= 4;
	gtk_widget_queue_draw (widget);
	g_print ("%d\n", gui->counter);
	return TRUE;
}

int main (int argc, char **argv)
{
	Gui *gui = g_new0 (Gui, 1);
	guchar bpm = 60;
	gtk_init (&argc, &argv);

	gui->window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
	gui->da = gtk_drawing_area_new ();
	gtk_container_add (GTK_CONTAINER (gui->window), gui->da);
	gtk_widget_show_all (gui->window);
	g_signal_connect (gui->window, "destroy", G_CALLBACK (on_destroy), NULL);
	g_signal_connect (gui->da, "draw", G_CALLBACK (on_draw), gui);

	g_timeout_add (60000/bpm, on_timeout, gui);
	gtk_main ();
	return 0;
}

