#include <gtk/gtk.h>
#include <math.h>

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
	char text[5];
	cairo_select_font_face (cr,
			"Sans",
			CAIRO_FONT_SLANT_NORMAL,
			CAIRO_FONT_WEIGHT_BOLD);

	g_snprintf (text, sizeof (text), "%d", gui->counter + 1);

	cairo_set_font_size (cr, gtk_widget_get_allocated_height (widget));
	cairo_text_extents_t extents;
	cairo_text_extents (cr, text, &extents);

	double x = gtk_widget_get_allocated_width (widget)/2 - (extents.width/2 + extents.x_bearing);
	double y = gtk_widget_get_allocated_height (widget)/2 - (extents.height/2 + extents.y_bearing);

	// draw a bounding box
	cairo_move_to (cr, x, y);
	cairo_set_source_rgba (cr, 1, 0.2, 0.2, 0.6);
	cairo_set_line_width (cr, 6.0);
	cairo_rectangle (cr, x, y - extents.height, extents.width + extents.x_bearing, extents.height);
	cairo_stroke (cr);

	// draw text
	cairo_move_to (cr, x, y);
	cairo_set_source_rgb (cr, 0.0, 0.0, 0.0);
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

