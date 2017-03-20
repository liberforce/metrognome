#ifndef METRONOME_H
#define METRONOME_H

#include <glib.h>

typedef struct metronome Metronome;
typedef void (*ClickCallback) (gpointer user_data);

Metronome *
metronome_new (void);

void
metronome_set_bpm (Metronome *metro, guint bpm);

guint
metronome_get_bpm (Metronome *metro);

gboolean
metronome_is_running (Metronome *metro);

guint
metronome_get_next_beat (Metronome *metro);

gint
metronome_get_counter (Metronome *metro);

void
metronome_reset_counter (Metronome *metro);

void
metronome_incr_counter (Metronome *metro);

void
metronome_start (Metronome *metro, ClickCallback callback, gpointer user_data);

void
metronome_stop (Metronome *metro);

void
metronome_free (Metronome *metro);

#endif
