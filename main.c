#include <gtk/gtk.h>
#include <math.h>

typedef struct
{
	GtkWidget *window;
	GtkWidget *da;
	GtkWidget *grid;
	GtkWidget *bpm_label;
	GtkWidget *bpm_spin;
	GtkWidget *play_stop_button;
	guint timeout_source;
} MetronomeGui;

typedef struct metronome
{
	int counter;
	guint bpm;
	gdouble next_beat_at; // in seconds
	GTimer *timer;
} Metronome;

typedef struct metronome_app
{
	MetronomeGui *gui;
	Metronome *metro;
} MetronomeApp;

Metronome *
metronome_new (void)
{
	Metronome * metro = g_new0(Metronome, 1);
	metro->counter = -1;
	metro->bpm = 60;
	metro->timer = g_timer_new();
	return metro;
}

void
metronome_set_bpm (Metronome *metro, guint bpm)
{
	g_assert (metro != NULL);
	metro->bpm = CLAMP(bpm, 40, 300);
}

guint
metronome_get_next_beat (Metronome *metro)
{
	g_assert (metro != NULL);

	// Now and next_beat_at are absolute timings in seconds since timer
	// was started
	gdouble now = g_timer_elapsed (metro->timer, NULL);
	guint period_in_msec = 60000/metro->bpm;
	metro->next_beat_at += ((gdouble)period_in_msec) / 1000;
	return (metro->next_beat_at - now) * 1000;
}

void
metronome_start (Metronome *metro)
{
	g_assert (metro != NULL);
	g_timer_start (metro->timer);
}

void
metronome_stop (Metronome *metro)
{
	g_assert (metro != NULL);
	g_timer_stop (metro->timer);
}

void
metronome_free (Metronome *metro)
{
	if (metro != NULL)
	{
		g_timer_destroy (metro->timer);
		g_free (metro);
	}
}

void on_destroy (G_GNUC_UNUSED GtkWidget *widget, G_GNUC_UNUSED gpointer user_data)
{
	gtk_main_quit ();
}

gboolean on_draw (G_GNUC_UNUSED GtkWidget *widget,
		cairo_t *cr,
		gpointer user_data)
{
	MetronomeApp *app = user_data;
	char text[5];

	// clear screen when we're not counting
	if (app->metro->counter == -1)
		return TRUE;

	g_snprintf (text, sizeof (text), "%d", app->metro->counter + 1);

	cairo_select_font_face (cr,
			"Sans",
			CAIRO_FONT_SLANT_NORMAL,
			CAIRO_FONT_WEIGHT_BOLD);

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
	MetronomeApp *app = user_data;
	MetronomeGui *gui = app->gui;

	// Display this click
	gtk_widget_queue_draw (gui->da);
	app->metro->counter++;
	app->metro->counter %= 4;

	// g_timeout_add uses time in milliseconds
	gui->timeout_source = g_timeout_add (
			metronome_get_next_beat (app->metro),
			on_timeout,
			app);

	// Prevent this timeout source from running again
	// (we run a new one at each beat)
	return FALSE;
}

void on_play_stop_button_clicked (G_GNUC_UNUSED GtkButton *button, gpointer user_data)
{
	MetronomeApp *app = user_data;
	MetronomeGui *gui = app->gui;

	g_assert (app->metro->bpm > 0);

	app->metro->counter = -1;

	if (gui->timeout_source == 0)
	{
		guint initial_delay_in_msec = 500;
		gui->timeout_source = g_timeout_add (initial_delay_in_msec, on_timeout, app);
		app->metro->next_beat_at = ((gdouble)initial_delay_in_msec) / 1000;
		metronome_start (app->metro);
	}
	else
	{
		g_source_remove (gui->timeout_source);
		gui->timeout_source = 0;
		gtk_widget_queue_draw (gui->da);
		metronome_stop (app->metro);
	}
}

void on_bpm_spin_value_changed (GtkSpinButton *spin, gpointer user_data)
{
	MetronomeApp *app = user_data;
	MetronomeGui *gui = app->gui;
	app->metro->bpm = gtk_spin_button_get_value_as_int (spin);

	if (gui->timeout_source != 0)
	{
		// reset the source if it's already running, using the new bpm
		g_source_remove (gui->timeout_source);
		guint period_in_msec = 60000/app->metro->bpm;
		gui->timeout_source = g_timeout_add (period_in_msec, on_timeout, app);
		app->metro->next_beat_at = ((gdouble)period_in_msec) / 1000;
		metronome_start (app->metro);
	}
}

int main (int argc, char **argv)
{
	MetronomeApp *app = g_new0 (MetronomeApp, 1);
	MetronomeGui *gui = g_new0 (MetronomeGui, 1);
	app->metro = metronome_new ();
	app->gui = gui;

	gtk_init (&argc, &argv);

	// Create widgets
	gui->window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
	gui->grid = gtk_grid_new ();
	gui->da = gtk_drawing_area_new ();
	gui->bpm_label = gtk_label_new ("BPM:");
	gui->bpm_spin = gtk_spin_button_new_with_range (1.0, 300.0, 1.0);
	gui->play_stop_button = gtk_button_new_from_icon_name ("media-playback-start", GTK_ICON_SIZE_BUTTON);

	// Build widget tree
	gtk_container_add (GTK_CONTAINER (gui->window), gui->grid);
	gtk_grid_attach (GTK_GRID (gui->grid), gui->bpm_label, 0, 0, 1, 1);
	gtk_widget_set_valign (gui->bpm_label, GTK_ALIGN_START);
	gtk_widget_set_halign (gui->bpm_label, GTK_ALIGN_END);
	gtk_grid_attach (GTK_GRID (gui->grid), gui->bpm_spin, 1, 0, 1, 1);
	gtk_widget_set_valign (gui->bpm_spin, GTK_ALIGN_START);
	gtk_widget_set_halign (gui->bpm_spin, GTK_ALIGN_START);
	gtk_grid_attach (GTK_GRID (gui->grid), gui->play_stop_button, 0, 1, 1, 1);
	gtk_widget_set_valign (gui->play_stop_button, GTK_ALIGN_START);
	gtk_widget_set_halign (gui->play_stop_button, GTK_ALIGN_FILL);
	gtk_button_set_always_show_image (GTK_BUTTON (gui->play_stop_button), TRUE);
	gtk_grid_attach (GTK_GRID (gui->grid), gui->da, 2, 0, 1, 2);
	gtk_widget_set_hexpand (gui->da, TRUE);
	gtk_widget_set_vexpand (gui->da, TRUE);

	// Initialize widget content
	gtk_spin_button_set_value (GTK_SPIN_BUTTON (gui->bpm_spin), app->metro->bpm);
	
	// Show UI and connect signals
	gtk_window_set_default_size (GTK_WINDOW (gui->window), 600, 400);
	gtk_widget_show_all (gui->window);
	g_signal_connect (gui->window, "destroy", G_CALLBACK (on_destroy), NULL);
	g_signal_connect (gui->da, "draw", G_CALLBACK (on_draw), app);
	g_signal_connect (gui->play_stop_button, "clicked", G_CALLBACK (on_play_stop_button_clicked), app);
	g_signal_connect (gui->bpm_spin, "value-changed", G_CALLBACK (on_bpm_spin_value_changed), app);

	gtk_main ();
	return 0;
}

