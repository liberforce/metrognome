#include <gtk/gtk.h>
#include <math.h>

typedef struct
{
	GtkWidget *window;
	GtkWidget *da;
	GtkWidget *grid;
	GtkWidget *bpm_label;
	GtkWidget *bpm_spin;
	GtkWidget *play_stop_button;;
	int counter;
	int bpm;
	GTimer *timer;
	gdouble next_beat; // in seconds
	guint timeout_source;
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

	// clear screen when we're not counting
	if (gui->counter == -1)
		return TRUE;

	cairo_select_font_face (cr,
			"Sans",
			CAIRO_FONT_SLANT_NORMAL,
			CAIRO_FONT_WEIGHT_BOLD);

	g_snprintf (text, sizeof (text), "%d", gui->counter + 1);

	cairo_set_font_size (cr, gtk_widget_get_allocated_height (widget));
	cairo_text_extents_t extents;
	cairo_text_extents (cr, text, &extents);

	double x = gtk_widget_get_allocated_width (widget)/2 - (extents.width/2 + extents.x_bearing);
	double y = gtk_widget_get_allocated_height (widget)/2 + extents.height/2;

#if 0
	// draw a bounding box
	cairo_move_to (cr, x, y);
	cairo_set_source_rgba (cr, 1, 0.2, 0.2, 0.6);
	cairo_set_line_width (cr, 6.0);
	cairo_rectangle (cr, x + extents.x_bearing, y + extents.y_bearing, extents.width, extents.height);
	cairo_stroke (cr);
#endif

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

	// Prepare next metronome click
	gdouble now = g_timer_elapsed (gui->timer, NULL);
	gdouble delta = now - gui->next_beat;

	// g_timeout_add uses time in milliseconds
	// g_timer_elapsed and next_beat are in seconds
	gui->timeout_source = g_timeout_add (60000/gui->bpm - (delta * 1000), on_timeout, gui);
	gui->next_beat += 60.0/gui->bpm;

	// Prevent this timeout source from running again
	// (we run a new one at each beat)
	return FALSE;
}

void on_play_stop_button_clicked (G_GNUC_UNUSED GtkButton *button, gpointer user_data)
{
	Gui *gui = user_data;

	// FIXME: make button insensitive instead of checking bpm
	if (gui->timeout_source == 0 && gui->bpm > 0)
	{
		gui->counter = 0;
		gtk_widget_queue_draw (gui->da);
		gui->timeout_source = g_timeout_add (60000/gui->bpm, on_timeout, gui);
		g_timer_start (gui->timer);
		gui->next_beat += 60.0/gui->bpm;
	}
	else if (gui->timeout_source != 0)
	{
		g_source_remove (gui->timeout_source);
		gui->timeout_source = 0;
		gui->counter = -1;
		gtk_widget_queue_draw (gui->da);
		g_timer_stop (gui->timer);
	}
}

void on_bpm_spin_value_changed (GtkSpinButton *spin, gpointer user_data)
{
	Gui *gui = user_data;
	gui->bpm = gtk_spin_button_get_value_as_int (spin);

	if (gui->timeout_source != 0)
	{
		// reset the source if it's already runnin, using the new bpm
		g_source_remove (gui->timeout_source);
		gui->timeout_source = g_timeout_add (60000/gui->bpm, on_timeout, gui);
	}
}

int main (int argc, char **argv)
{
	Gui *gui = g_new0 (Gui, 1);
	gui->bpm = 60;
	gui->counter = -1;

	gtk_init (&argc, &argv);

	// Create widgets
	gui->window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
	gui->grid = gtk_grid_new ();
	gui->da = gtk_drawing_area_new ();
	gui->bpm_label = gtk_label_new ("BPM:");
	gui->bpm_spin = gtk_spin_button_new_with_range (1.0, 300.0, 1.0);
	gui->play_stop_button = gtk_button_new_from_stock (GTK_STOCK_MEDIA_PLAY);

	// Build widget tree
	gtk_container_add (GTK_CONTAINER (gui->window), gui->grid);
	gtk_grid_attach (GTK_GRID (gui->grid), gui->bpm_label, 0, 0, 1, 1);
	gtk_grid_attach (GTK_GRID (gui->grid), gui->bpm_spin, 1, 0, 1, 1);
	gtk_grid_attach (GTK_GRID (gui->grid), gui->play_stop_button, 0, 1, 1, 1);
	gtk_button_set_always_show_image (GTK_BUTTON (gui->play_stop_button), TRUE);
	gtk_grid_attach (GTK_GRID (gui->grid), gui->da, 2, 0, 1, 2);
	gtk_widget_set_hexpand (gui->da, TRUE);
	gtk_widget_set_vexpand (gui->da, TRUE);

	// Initialize widget content
	gtk_spin_button_set_value (GTK_SPIN_BUTTON (gui->bpm_spin), gui->bpm);
	
	// Show UI and connect signals
	gtk_window_set_default_size (GTK_WINDOW (gui->window), 600, 400);
	gtk_widget_show_all (gui->window);
	g_signal_connect (gui->window, "destroy", G_CALLBACK (on_destroy), NULL);
	g_signal_connect (gui->da, "draw", G_CALLBACK (on_draw), gui);
	g_signal_connect (gui->play_stop_button, "clicked", G_CALLBACK (on_play_stop_button_clicked), gui);
	g_signal_connect (gui->bpm_spin, "value-changed", G_CALLBACK (on_bpm_spin_value_changed), gui);
	gui->timer = g_timer_new ();

	gtk_main ();
	return 0;
}

