#include <gtk/gtk.h>
#include <math.h>

#include "metronome.h"

typedef struct
{
	GtkWidget *window;
	GtkWidget *da;
	GtkWidget *grid;
	GtkWidget *bpm_label;
	GtkWidget *bpm_spin;
	GtkWidget *play_stop_button;
} MetronomeGui;

typedef struct metronome_app
{
	MetronomeGui *gui;
	Metronome *metro;
} MetronomeApp;

void on_destroy (G_GNUC_UNUSED GtkWidget *widget, G_GNUC_UNUSED gpointer user_data)
{
	gtk_main_quit ();
}

void
draw_counter_trace (G_GNUC_UNUSED GtkWidget *widget,
		G_GNUC_UNUSED cairo_t *cr,
		int counter)
{
	char text[5];
	g_snprintf (text, sizeof (text), "%d", counter);
	g_print ("%s\n", text);
}

void
draw_counter_numeric (GtkWidget *widget,
		cairo_t *cr,
		int counter)
{
	char text[5];
	g_snprintf (text, sizeof (text), "%d", counter);

	cairo_select_font_face (cr,
			"Sans",
			CAIRO_FONT_SLANT_NORMAL,
			CAIRO_FONT_WEIGHT_BOLD);

	cairo_set_font_size (cr, gtk_widget_get_allocated_height (widget));
	cairo_text_extents_t extents;
	cairo_text_extents (cr, text, &extents);

	double x = gtk_widget_get_allocated_width (widget) / 2;
	double y = gtk_widget_get_allocated_height (widget) / 2;

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
}

void
draw_counter_circle (GtkWidget *widget,
		cairo_t *cr,
		int counter)
{
	char text[5];
	static const int n_times = 4;
	double x[n_times];
	int width = gtk_widget_get_allocated_width (widget);
	int height = gtk_widget_get_allocated_height (widget);
	int i;

	// Compute coordinates of the circle centers
	for (i = 0; i < n_times; i++)
	{
		x[i] = (width * (i + 1)) / (n_times + 1);
	}

	double y = height / 2;
	double scaled_diameter = (width * 0.9) / (n_times + 1);
	double scaled_radius = scaled_diameter / 2;

	// Prepare drawing the numbers
	cairo_set_font_size (cr, scaled_diameter);
	cairo_text_extents_t extents;

	gboolean is_tick = FALSE;

	// Draw the circles and numbers inside them
	for (i = 0; i < n_times; i++)
	{
		is_tick = (counter == (i + 1));

		if (is_tick)
			cairo_set_source_rgb (cr, 0.0, 0.0, 0.0);
		else
			cairo_set_source_rgb (cr, 1.0, 1.0, 1.0);

		cairo_arc (cr,
				x[i],
				y,
				scaled_radius,
				0.0,
				2 * M_PI);
		cairo_fill (cr);

		if (is_tick)
		{
			g_snprintf (text, sizeof (text), "%d", i + 1);
			cairo_text_extents (cr, text, &extents);
			cairo_set_source_rgb (cr, 1.0, 1.0, 1.0);
#if 0
			cairo_rectangle (cr,
					x[i] - extents.width / 2,
					y - extents.height / 2,
					extents.width,
					extents.height);

			cairo_fill (cr);
#endif
			cairo_move_to (cr,
					x[i] - extents.x_bearing - extents.width / 2,
					y + extents.height / 2);
			// cairo_set_source_rgb (cr, 1.0, 0.0, 0.0);
			cairo_show_text (cr, text);
		}
	}
}

gboolean on_draw (G_GNUC_UNUSED GtkWidget *widget,
		cairo_t *cr,
		gpointer user_data)
{
	Metronome *metro = user_data;

	int counter = metronome_get_counter(metro);

	// clear screen when we're not counting
	if (counter == 0)
		return TRUE;

	draw_counter_circle (widget, cr, counter);

	return TRUE;
}

void
on_click (gpointer user_data)
{
	MetronomeApp *app = user_data;
	MetronomeGui *gui = app->gui;

	// Display this click
	g_print ("%d\n", metronome_get_counter (app->metro));
	gtk_widget_queue_draw (gui->da);
}

void on_play_stop_button_clicked (G_GNUC_UNUSED GtkButton *button, gpointer user_data)
{
	MetronomeApp *app = user_data;

	if (! metronome_is_running (app->metro))
	{
		metronome_start (app->metro, on_click, app);
	}
	else
	{
		metronome_stop (app->metro);
	}
}

void on_bpm_spin_value_changed (GtkSpinButton *spin, gpointer user_data)
{
	MetronomeApp *app = user_data;
	metronome_set_bpm (app->metro, gtk_spin_button_get_value_as_int (spin));
}

MetronomeGui *
create_gui (Metronome *metro)
{
	MetronomeGui *gui = g_new0 (MetronomeGui, 1);

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
	gtk_spin_button_set_value (GTK_SPIN_BUTTON (gui->bpm_spin), metronome_get_bpm (metro));
	
	// Show UI and connect signals
	gtk_window_set_default_size (GTK_WINDOW (gui->window), 600, 400);
	gtk_widget_show_all (gui->window);

	return gui;
}

int main (int argc, char **argv)
{
	gtk_init (&argc, &argv);

	MetronomeApp *app = g_new0 (MetronomeApp, 1);
	app->metro = metronome_new ();

	MetronomeGui *gui = create_gui (app->metro);
	app->gui = gui;

	if (gui != NULL)
	{
		g_signal_connect (gui->window, "destroy", G_CALLBACK (on_destroy), NULL);
		g_signal_connect (gui->da, "draw", G_CALLBACK (on_draw), app->metro);
		g_signal_connect (gui->play_stop_button, "clicked", G_CALLBACK (on_play_stop_button_clicked), app);
		g_signal_connect (gui->bpm_spin, "value-changed", G_CALLBACK (on_bpm_spin_value_changed), app);
	}

	gtk_main ();
	return 0;
}

