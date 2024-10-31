#ifndef NOTE_PARSER_H
#define NOTE_PARSER_H

#include "oscillator.h"

float get_frequency(const char *note_name);
int get_waveform(const char *waveform_name);
EffectType get_effect(const char *effect_name);
void parse_notes_from_file(const char *filename, Note ***sequences,
                           int **num_notes);

#endif // NOTE_PARSER_H
