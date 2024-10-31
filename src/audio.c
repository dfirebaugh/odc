#include <math.h>
#include <portaudio.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include "audio.h"
#include "oscillator.h"

int is_playback_active = 0;

CustomWaveform custom_waveforms[MAX_CUSTOM_WAVEFORMS];
int num_custom_waveforms = 0;

static float calculate_adsr_envelope(oscillator *osc);
static float generate_waveform_sample(oscillator *osc, float envelope);
static float generate_noise_sample(oscillator *osc, float envelope);
static void update_sample_buffer(oscillator *osc, float sample);
static int initialize_portaudio();
static int start_portaudio_stream(PaStream **stream, oscillator *osc);
static void stop_and_close_stream(PaStream *stream);
static void handle_vibrato(oscillator *osc, float *current_freq);
static void handle_sweep_up(oscillator *osc, float *current_freq);
static void handle_sweep_down(oscillator *osc, float *current_freq);
static void apply_max_volume(oscillator *osc, float *sample);

int audio_callback(const void *inputBuffer, void *outputBuffer,
                   unsigned long framesPerBuffer,
                   const PaStreamCallbackTimeInfo *timeInfo,
                   PaStreamCallbackFlags statusFlags, void *userData) {
  AudioData *audio_data = (AudioData *)userData;
  if (audio_data == NULL || audio_data->app_running == NULL) {
    fprintf(stderr,
            "Audio callback error: audio_data or app_running is NULL.\n");
    return paAbort;
  }

  if (!*(audio_data->app_running)) {
    memset(outputBuffer, 0, framesPerBuffer * sizeof(float));
    return paComplete;
  }

  oscillator **oscillators = audio_data->oscillators;
  float *out = (float *)outputBuffer;
  (void)inputBuffer;

  for (unsigned long i = 0; i < framesPerBuffer; i++) {
    float sample = 0.0f;

    for (int j = 0; j < NUM_CHANNELS; j++) {
      oscillator *osc = oscillators[j];
      pthread_mutex_lock(&osc->mutex);
      float osc_sample = 0.0f;

      if (osc->playing) {
        osc->note_time += 1.0f / SAMPLE_RATE;

        float envelope = calculate_adsr_envelope(osc);

        if (envelope < 0.0f)
          envelope = 0.0f;
        if (envelope > 1.0f)
          envelope = 1.0f;

        if (osc->is_noise) {
          osc_sample = generate_noise_sample(osc, envelope);
        } else {
          osc_sample = generate_waveform_sample(osc, envelope);
        }

        apply_max_volume(osc, &osc_sample);
      } else {
        osc_sample = 0.0f;
      }

      update_sample_buffer(osc, osc_sample);

      pthread_mutex_unlock(&osc->mutex);

      sample += osc_sample;
    }

    sample /= NUM_CHANNELS;
    if (sample > 1.0f)
      sample = 1.0f;
    if (sample < -1.0f)
      sample = -1.0f;

    *out++ = sample;
  }

  return paContinue;
}

static float calculate_adsr_envelope(oscillator *osc) {
  float envelope = 0.0f;
  float t = osc->note_time;

  if (!osc->release_started) {
    // Attack phase
    if (t < osc->attack_time) {
      envelope = (t / osc->attack_time);
    }
    // Decay phase
    else if (t < (osc->attack_time + osc->decay_time)) {
      float decay_elapsed = t - osc->attack_time;
      envelope = 1.0f - ((1.0f - osc->sustain_level) *
                         (decay_elapsed / osc->decay_time));
    }
    // Sustain phase
    else if (t < (osc->note_duration - osc->release_time)) {
      envelope = osc->sustain_level;
    }
    // Start release phase
    else {
      osc->release_started = 1;
      osc->release_start_time = t;
    }
  }

  if (osc->release_started) {
    float release_elapsed = t - osc->release_start_time;
    if (release_elapsed < osc->release_time) {
      envelope =
          osc->sustain_level * (1.0f - (release_elapsed / osc->release_time));
    } else {
      envelope = 0.0f;
      osc->playing = 0;
    }
  }

  return envelope;
}

static float generate_waveform_sample(oscillator *osc, float envelope) {
  float waveform_sample = 0.0f;

  if (osc->t < 0.0f) {
    fprintf(stderr, "Warning: osc->t is negative (%f). Resetting to 0.\n",
            osc->t);
    osc->t = 0.0f;
  }

  if (!osc->is_noise && osc->current_waveform != NULL) {
    int sample_index = (int)osc->t % osc->table_size;

    if (sample_index < 0 || sample_index >= osc->table_size) {
      fprintf(
          stderr,
          "Error: sample_index %d out of bounds (0 - %d) for oscillator %p.\n",
          sample_index, osc->table_size - 1, (void *)osc);
      sample_index = 0;
    }

    waveform_sample =
        ((float)osc->current_waveform[sample_index] / 32767.5f - 1.0f);
  } else {
    if (osc->is_noise) {
      waveform_sample = generate_noise_sample(osc, envelope);
    } else {
      // Treat as silence
    }
  }

  float osc_sample = waveform_sample * envelope * osc->waveform_scale;

  float current_freq = osc->base_freq;

  if (osc->effect != EFFECT_NONE && osc->current_waveform != NULL &&
      strcasecmp(osc->current_waveform_name, "rest") != 0) {
    switch (osc->effect) {
    case EFFECT_VIBRATO:
      handle_vibrato(osc, &current_freq);
      break;
    case EFFECT_SWEEP_UP:
      handle_sweep_up(osc, &current_freq);
      break;
    case EFFECT_SWEEP_DOWN:
      handle_sweep_down(osc, &current_freq);
      break;
    default:
      break;
    }
  }

  if (current_freq < 0.0f) {
    current_freq = 0.0f;
  }

  double freq_increment =
      (double)current_freq * osc->table_size / osc->sample_rate;
  osc->t += freq_increment;
  if (osc->t >= osc->table_size) {
    osc->t -= osc->table_size;
  }

  return osc_sample;
}

static float generate_noise_sample(oscillator *osc, float envelope) {
  int samples_per_shift = (int)(osc->sample_rate / osc->base_freq);
  if (samples_per_shift < 1)
    samples_per_shift = 1;

  if (osc->noise_counter <= 0) {
    unsigned short feedback;
    if (osc->noise_mode == 0) {
      feedback = ((osc->lfsr & 1) ^ ((osc->lfsr >> 1) & 1)) << 14;
    } else {
      feedback = ((osc->lfsr & 1) ^ ((osc->lfsr >> 6) & 1)) << 14;
    }
    osc->lfsr = (osc->lfsr >> 1) | feedback;
    osc->noise_counter = samples_per_shift;
  } else {
    osc->noise_counter--;
  }

  int16_t output = (osc->lfsr & 1) ? 32767 : -32768;
  float osc_sample = (output / 32768.0f) * envelope * osc->waveform_scale;

  return osc_sample;
}

static void update_sample_buffer(oscillator *osc, float sample) {
  osc->sample_buffer[osc->sample_buffer_index] = sample;
  osc->sample_buffer_index =
      (osc->sample_buffer_index + 1) % SAMPLE_BUFFER_SIZE;
}

static void handle_vibrato(oscillator *osc, float *current_freq) {
  if (osc->effect == EFFECT_VIBRATO) {
    float vibrato_amount = 5.0f;
    float vibrato_freq = 5.0f;
    *current_freq +=
        vibrato_amount * sinf(2.0f * M_PI * vibrato_freq * osc->note_time);
  }
}

static void handle_sweep_up(oscillator *osc, float *current_freq) {
  float progress = osc->note_time / osc->note_duration;
  if (progress > 1.0f)
    progress = 1.0f;
  *current_freq =
      osc->freq_start + (osc->freq_end - osc->freq_start) * progress;
}

static void handle_sweep_down(oscillator *osc, float *current_freq) {
  float progress = osc->note_time / osc->note_duration;
  if (progress > 1.0f)
    progress = 1.0f;
  *current_freq =
      osc->freq_start - (osc->freq_start - osc->freq_end) * progress;
}

static void apply_max_volume(oscillator *osc, float *sample) {
  *sample *= osc->max_volume;
}

static int initialize_portaudio() {
  PaError err = Pa_Initialize();
  if (err != paNoError) {
    fprintf(stderr, "PortAudio error: %s\n", Pa_GetErrorText(err));
    return -1;
  }

  PaHostApiIndex hostApiIndex = Pa_HostApiTypeIdToHostApiIndex(paALSA);
  if (hostApiIndex == paHostApiNotFound) {
    fprintf(stderr, "PortAudio error: ALSA host API not found\n");
    return -1;
  }

  const PaHostApiInfo *hostApiInfo = Pa_GetHostApiInfo(Pa_GetDefaultHostApi());
  if (hostApiInfo) {
    printf("Using default host API: %s\n", hostApiInfo->name);
  } else {
    printf("Using default host API.\n");
  }

  return 0;
}

static int start_portaudio_stream(PaStream **stream, oscillator *osc) {
  PaError err = Pa_OpenDefaultStream(stream, 0, 1, paFloat32, SAMPLE_RATE, 256,
                                     audio_callback, osc);
  if (err != paNoError) {
    fprintf(stderr, "PortAudio error: %s\n", Pa_GetErrorText(err));
    return -1;
  }

  err = Pa_StartStream(*stream);
  if (err != paNoError) {
    fprintf(stderr, "PortAudio error: %s\n", Pa_GetErrorText(err));
    Pa_CloseStream(*stream);
    return -1;
  }

  return 0;
}

static void stop_and_close_stream(PaStream *stream) {
  PaError err;

  err = Pa_StopStream(stream);
  if (err != paNoError) {
    fprintf(stderr, "PortAudio error while stopping stream: %s\n",
            Pa_GetErrorText(err));
  }

  err = Pa_CloseStream(stream);
  if (err != paNoError) {
    fprintf(stderr, "PortAudio error while closing stream: %s\n",
            Pa_GetErrorText(err));
  }
}

int audio_init() {
  return initialize_portaudio();
}

int audio_play_waveform(oscillator *osc) {
  PaStream *stream;

  if (start_portaudio_stream(&stream, osc) != 0) {
    return -1;
  }

  float duration = (float)osc->table_size / osc->base_freq;
  Pa_Sleep(duration * 1000);

  stop_and_close_stream(stream);

  return 0;
}

int audio_is_playback_active() { return is_playback_active; }

void audio_set_playback_active(int is_active) {
  is_playback_active = is_active;
}

void *audio_play_sequence_thread(void *arg) {
  OscillatorThreadData *data = (OscillatorThreadData *)arg;
  oscillator **oscillators = data->oscillators;
  Note **sequences = data->sequences;
  int *num_notes = data->num_notes;
  int num_channels = data->num_channels;

  int note_indices[num_channels];
  double note_start_times[num_channels];
  int playing[num_channels];

  for (int ch = 0; ch < num_channels; ch++) {
    note_indices[ch] = 0;
    note_start_times[ch] = 0.0;
    playing[ch] = 0;
  }

  struct timespec ts_start, ts_now;
  clock_gettime(CLOCK_MONOTONIC, &ts_start);

  int active_channels = num_channels;

  while (active_channels > 0 && *(data->app_running)) {
    active_channels = 0;
    clock_gettime(CLOCK_MONOTONIC, &ts_now);
    double elapsed_time = (ts_now.tv_sec - ts_start.tv_sec) +
                          (ts_now.tv_nsec - ts_start.tv_nsec) / 1e9;

    for (int ch = 0; ch < num_channels; ch++) {
      if (note_indices[ch] >= num_notes[ch]) {
        continue;
      }

      oscillator *osc = oscillators[ch];
      Note *notes = sequences[ch];

      if (!playing[ch]) {
        Note current_note = notes[note_indices[ch]];

        pthread_mutex_lock(&osc->mutex);

        int is_custom_waveform = 0;
        for (int i = 0; i < num_custom_waveforms; i++) {
          if (strcasecmp(current_note.waveform_name,
                         custom_waveforms[i].name) == 0) {
            oscillator_set_custom_waveform(osc, &custom_waveforms[i]);
            is_custom_waveform = 1;
            break;
          }
        }
        if (!is_custom_waveform) {
          oscillator_set_waveform(osc, current_note.waveform_name);
        }

        oscillator_set_freq(osc, current_note.freq);

        osc->attack_time = current_note.attack;
        osc->decay_time = current_note.decay;
        osc->sustain_level = current_note.sustain;
        osc->release_time = current_note.release;

        osc->effect = current_note.effect;

        osc->note_duration = current_note.duration;
        osc->note_time = 0.0f;
        osc->playing = 1;
        osc->release_started = 0;
        osc->release_start_time = 0.0f;

        pthread_mutex_unlock(&osc->mutex);

        note_start_times[ch] = elapsed_time;
        playing[ch] = 1;
      } else {
        double note_elapsed_time = elapsed_time - note_start_times[ch];
        Note current_note = notes[note_indices[ch]];
        if (note_elapsed_time >= current_note.duration) {
          pthread_mutex_lock(&osc->mutex);
          osc->playing = 0;
          pthread_mutex_unlock(&osc->mutex);

          note_indices[ch]++;
          playing[ch] = 0;
        }
      }
      active_channels++;
    }
    usleep(10000);
  }

  for (int ch = 0; ch < data->num_channels; ch++) {
    pthread_mutex_lock(&oscillators[ch]->mutex);
    oscillators[ch]->playing = 0;
    pthread_mutex_unlock(&oscillators[ch]->mutex);
  }

  is_playback_active = 0;
  return NULL;
}

void audio_terminate() { Pa_Terminate(); }
