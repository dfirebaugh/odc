#ifndef NOTE_PARSER_H
#define NOTE_PARSER_H

#include "odc.h"
#include "odc_oscillator.h"

/*ODC_API float odc_get_frequency(const char *note_name);*/
/*ODC_API waveform_type odc_get_waveform(const char *waveform_name);*/
/*ODC_API effect_type odc_get_effect(const char *effect_name);*/
ODC_API void odc_parse_notes_from_file(const char *filename,
				       oscillator_note ***sequences,
				       int **num_notes);

#endif // NOTE_PARSER_H
