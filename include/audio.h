#ifndef AUDIO_H
#define AUDIO_H

#include <portaudio.h>

#include "oscillator.h"

#define SAMPLE_RATE 44100
#define MAX_CUSTOM_WAVEFORMS 32

extern CustomWaveform custom_waveforms[MAX_CUSTOM_WAVEFORMS];
extern int num_custom_waveforms;

int audio_callback(const void *inputBuffer, void *outputBuffer,
                   unsigned long framesPerBuffer,
                   const PaStreamCallbackTimeInfo *timeInfo,
                   PaStreamCallbackFlags statusFlags, void *userData);
int audio_init();
int audio_play_waveform(oscillator *osc);
void *audio_play_sequence_thread(void *arg);
int audio_is_playback_active();
void audio_set_playback_active(int is_active);
void audio_terminate();

#endif // AUDIO_H
