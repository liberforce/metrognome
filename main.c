#include <gtk/gtk.h>
#include <math.h>

#include "metronome.h"

typedef struct
{
	GtkBuilder *builder;
	GtkWidget *window;
	GtkWidget *da;
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

void on_play_stop_button_clicked (GtkButton *button, gpointer user_data)
{
	MetronomeApp *app = user_data;

	if (! metronome_is_running (app->metro))
	{
		metronome_start (app->metro, on_click, app);
		gtk_button_set_label (button, "gtk-media-stop");
	}
	else
	{
		metronome_stop (app->metro);
		gtk_button_set_label (button, "gtk-media-play");
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
	GtkBuilder *builder = gtk_builder_new_from_file ("metrognome.ui");
	gui->builder = builder;
	g_assert (builder != NULL);

	// Create widgets
	gui->window = GTK_WIDGET(gtk_builder_get_object (builder, "window"));
	gui->da = GTK_WIDGET(gtk_builder_get_object (builder, "da"));
	gui->bpm_spin = GTK_WIDGET(gtk_builder_get_object (builder, "bpm_spin"));
	gui->play_stop_button = GTK_WIDGET(gtk_builder_get_object (builder, "play_stop_button"));

	// Initialize widget content
	gtk_spin_button_set_value (GTK_SPIN_BUTTON (gui->bpm_spin), metronome_get_bpm (metro));
	
	// Show UI
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

	// Connect signals
	g_signal_connect (gui->window, "destroy", G_CALLBACK (on_destroy), NULL);
	g_signal_connect (gui->da, "draw", G_CALLBACK (on_draw), app->metro);
	g_signal_connect (gui->play_stop_button, "clicked", G_CALLBACK (on_play_stop_button_clicked), app);
	g_signal_connect (gui->bpm_spin, "value-changed", G_CALLBACK (on_bpm_spin_value_changed), app);

	gtk_main ();
	return 0;
}

