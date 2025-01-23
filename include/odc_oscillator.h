#ifndef ODC_OSCILLATOR_H
#define ODC_OSCILLATOR_H

#include <pthread.h>

#include "odc.h"

#define SAMPLE_BUFFER_SIZE 2048
#define NUM_CHANNELS 4

typedef enum {
	WAVEFORM_SINE,
	WAVEFORM_SQUARE,
	WAVEFORM_SAWTOOTH,
	WAVEFORM_TRIANGLE,
	WAVEFORM_NOISE
} waveform_type;

typedef struct {
	char name[16];
	float *samples;
	int num_samples;
} custom_waveform;

typedef enum {
	EFFECT_NONE,
	EFFECT_SWEEP_UP,
	EFFECT_SWEEP_DOWN,
	EFFECT_VIBRATO,
} effect_type;

typedef struct {
	int base_freq;
	int table_size;
	int sample_rate;
	unsigned short *current_waveform;
	char current_waveform_name[16];
	double t;
	float volume;
	float initial_volume;
	float max_volume;
	float waveform_scale;
	pthread_mutex_t mutex;
	float sample_buffer[SAMPLE_BUFFER_SIZE];
	int sample_buffer_index;
	int is_noise;
	float default_volume;
	float freq_start;
	float freq_end;
	float phase;
	float note_duration;
	double note_time;
	float decay_rate;
	effect_type effect;
	unsigned short lfsr;
	int noise_mode;
	int noise_counter;
	int playing;
	float prev_noise_sample;

	float attack_time;
	float decay_time;
	float sustain_level;
	float release_time;
	float envelope_level;
	int release_started;
	float release_start_time;
} oscillator;

typedef struct {
	float freq;
	float duration;
	char *waveform_name;
	float attack;
	float decay;
	float sustain;
	float release;
	effect_type effect;
	float sweep_end;
} oscillator_note;

typedef struct {
	oscillator **oscillators;
	oscillator_note **sequences;
	int *num_notes;
	int num_channels;
	volatile int *app_running;
} odc_oscillator_thread_data;

ODC_API oscillator odc_oscillator_init(int base_freq, int table_size,
				       float initial_volume);
ODC_API void odc_oscillator_set_volume(oscillator *dco, float volume);
ODC_API void odc_oscillator_set_max_volume(oscillator *dco, float volume);
ODC_API void odc_oscillator_set_custom_waveform(oscillator *dco,
						custom_waveform *waveform);
ODC_API void odc_oscillator_set_waveform(oscillator *osc,
					 const char *waveform_type);
ODC_API void odc_oscillator_set_noise_mode(oscillator *dco, int mode);
ODC_API void odc_oscillator_set_freq(oscillator *dco, int freq);
ODC_API void odc_oscillator_stop_dco(oscillator *dco);
ODC_API void odc_oscillator_play_note(oscillator *dco, int freq, float duration,
				      const char *waveform);
ODC_API void odc_oscillator_play_note_struct(oscillator *dco,
					     const oscillator_note *note);
ODC_API void odc_oscillator_play_sequence(oscillator *dco,
					  oscillator_note notes[],
					  int num_notes,
					  volatile int *app_running);
#endif // ODC_OSCILLATOR_H
