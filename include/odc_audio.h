#ifndef ODC_AUDIO_H
#define ODC_AUDIO_H

#include <portaudio.h>

#include "odc.h"
#include "odc_oscillator.h"

#define SAMPLE_RATE 44100
#define MAX_CUSTOM_WAVEFORMS 32

typedef struct {
	oscillator **oscillators;
	volatile int *app_running;
} odc_audio_data;

extern ODC_API custom_waveform odc_custom_waveforms[MAX_CUSTOM_WAVEFORMS];
extern ODC_API int odc_num_custom_waveforms;

ODC_API int odc_audio_callback(const void *inputBuffer, void *outputBuffer,
			       unsigned long framesPerBuffer,
			       const PaStreamCallbackTimeInfo *timeInfo,
			       PaStreamCallbackFlags statusFlags,
			       void *userData);
ODC_API int odc_audio_init();
ODC_API int odc_audio_play_waveform(oscillator *osc);
ODC_API int odc_audio_play_note(oscillator *osc);
ODC_API void *odc_audio_play_sequence_thread(void *arg);
ODC_API int odc_audio_is_playback_active();
ODC_API void odc_audio_set_playback_active(int is_active);
ODC_API void odc_audio_terminate();

#endif // ODC_AUDIO_H
