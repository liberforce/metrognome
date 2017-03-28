#include "metronome.h"

typedef struct metronome
{
	int counter;
	guint bpm;
	guint timeout_source;
	gdouble next_beat_at; // in seconds
	GTimer *timer;
	ClickCallback on_click;
	gpointer on_click_user_data;
} Metronome;

Metronome *
metronome_new (void)
{
	Metronome * metro = g_new0(Metronome, 1);
	metro->counter = -1;
	metro->bpm = 60;
	metro->timeout_source = 0;
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
metronome_get_bpm (Metronome *metro)
{
	g_assert (metro != NULL);
	return metro->bpm;
}

gboolean
metronome_is_running (Metronome *metro)
{
	g_assert (metro != NULL);
	return metro->timeout_source != 0;
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

gint
metronome_get_counter (Metronome *metro)
{
	g_assert (metro != NULL);
	return metro->counter + 1;
}

void
metronome_reset_counter (Metronome *metro)
{
	g_assert (metro != NULL);
	metro->counter = -1;
	g_print ("%s\n", "reset");
}
void

metronome_incr_counter (Metronome *metro)
{
	g_assert (metro != NULL);
	metro->counter = (metro->counter + 1) % 4;
}

static gboolean
on_timeout (gpointer user_data)
{
	Metronome *metro = user_data;

	// Handle this metronome tick
	metronome_incr_counter(metro);
	(*metro->on_click)(metro->on_click_user_data);

	// Prepare next metronome tick
	metro->timeout_source = g_timeout_add (
			metronome_get_next_beat (metro),
			on_timeout,
			metro);

	// Prevent this timeout source from running again
	// (we run a new one at each beat)
	return FALSE;
}

void
metronome_start (Metronome *metro, ClickCallback callback, gpointer user_data)
{
	g_assert (metro != NULL);

	g_timer_start (metro->timer);
	g_print ("%s\n", "start");
	guint initial_delay_in_msec = 100;
	metro->next_beat_at = ((gdouble)initial_delay_in_msec) / 1000;
	metro->on_click = callback;
	metro->on_click_user_data = user_data;
	metro->timeout_source = g_timeout_add (initial_delay_in_msec, on_timeout, metro);
}

void
metronome_stop (Metronome *metro)
{
	g_assert (metro != NULL);

	g_timer_stop (metro->timer);
	metronome_reset_counter (metro);
	g_source_remove (metro->timeout_source);
	metro->timeout_source = 0;
	// Add a last redraw to clean up the screen
	(*metro->on_click)(metro->on_click_user_data);
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

