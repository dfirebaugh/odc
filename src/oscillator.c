#include <math.h>
#include <portaudio.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <time.h>
#include <unistd.h>

#include "audio.h"
#include "oscillator.h"
unsigned short *oscillator_generate_sine_wave(int table_size);
unsigned short *oscillator_generate_square_wave(int table_size);
unsigned short *oscillator_generate_triangle_wave(int table_size);

oscillator oscillator_init(int base_freq, int table_size,
                           float initial_volume) {
  oscillator osc;
  osc.base_freq = base_freq;
  osc.table_size = table_size;
  osc.sample_rate = SAMPLE_RATE;
  osc.current_waveform = NULL;

  osc.current_waveform = oscillator_generate_sine_wave(table_size);
  if (!osc.current_waveform) {
    fprintf(stderr, "Failed to allocate square waveform.\n");
  }
  osc.t = 0.0;
  osc.volume = initial_volume;
  osc.initial_volume = initial_volume;
  osc.default_volume = initial_volume;
  pthread_mutex_init(&osc.mutex, NULL);
  osc.sample_buffer_index = 0;
  memset(osc.sample_buffer, 0, sizeof(osc.sample_buffer));
  osc.is_noise = 0;
  osc.freq_start = 0.0f;
  osc.freq_end = 0.0f;
  osc.phase = 0.0f;
  osc.note_duration = 0.0f;
  osc.note_time = 0.0;
  osc.playing = 0;
  osc.decay_rate = 0.0f;
  osc.lfsr = 0x4000;
  osc.noise_mode = 0;
  osc.noise_counter = 0;
  osc.effect = EFFECT_NONE;
  return osc;
}

void oscillator_set_volume(oscillator *osc, float volume) {
  if (volume < 0.0f) {
    volume = 0.0f;
  } else if (volume > 1.0f) {
    volume = 1.0f;
  }
  osc->volume = volume;
}

void oscillator_set_max_volume(oscillator *osc, float volume) {
  if (volume < 0.0f) {
    volume = 0.0f;
  } else if (volume > 1.0f) {
    volume = 1.0f;
  }
  pthread_mutex_lock(&osc->mutex);
  osc->max_volume = volume;
  pthread_mutex_unlock(&osc->mutex);
}

void oscillator_set_waveform(oscillator *osc, const char *waveform_type) {
  if (osc->current_waveform != NULL) {
    free(osc->current_waveform);
    osc->current_waveform = NULL;
  }

  strncpy(osc->current_waveform_name, waveform_type,
          sizeof(osc->current_waveform_name) - 1);
  osc->current_waveform_name[sizeof(osc->current_waveform_name) - 1] = '\0';

  printf("Setting waveform for oscillator %p to %s.\n", (void *)osc,
         waveform_type);

  if (strcasecmp(waveform_type, "sine") == 0) {
    osc->current_waveform = oscillator_generate_sine_wave(osc->table_size);
    osc->is_noise = 0;
  } else if (strcasecmp(waveform_type, "square") == 0) {
    osc->current_waveform = oscillator_generate_square_wave(osc->table_size);
    osc->is_noise = 0;
  } else if (strcasecmp(waveform_type, "triangle") == 0) {
    osc->current_waveform = oscillator_generate_triangle_wave(osc->table_size);
    osc->is_noise = 0;
  } else if (strcasecmp(waveform_type, "rest") == 0) {
    osc->current_waveform = malloc(osc->table_size * sizeof(unsigned short));
    if (osc->current_waveform) {
      memset(osc->current_waveform, 0,
             osc->table_size * sizeof(unsigned short));
      osc->is_noise = 0;
      printf("Oscillator %p set to REST waveform.\n", (void *)osc);
    } else {
      fprintf(stderr, "Failed to allocate memory for REST waveform.\n");
      osc->is_noise = 0;
      osc->current_waveform = oscillator_generate_sine_wave(osc->table_size);
      if (osc->current_waveform) {
        printf("Oscillator %p fallback to SINE waveform.\n", (void *)osc);
      } else {
        fprintf(stderr, "Failed to allocate default waveform.\n");
      }
    }
  } else {
    printf("Unknown waveform type: %s. Defaulting to SINE.\n", waveform_type);
    osc->current_waveform = oscillator_generate_sine_wave(osc->table_size);
    osc->is_noise = 0;
  }

  osc->waveform_scale = 1.0f;
}

void oscillator_set_custom_waveform(oscillator *osc, CustomWaveform *waveform) {
  if (osc->current_waveform != NULL) {
    free(osc->current_waveform);
    osc->current_waveform = NULL;
  }
  osc->current_waveform =
      malloc(waveform->num_samples * sizeof(unsigned short));
  osc->table_size = waveform->num_samples;
  for (int i = 0; i < waveform->num_samples; i++) {
    osc->current_waveform[i] =
        (unsigned short)((waveform->samples[i] + 1.0f) * 32767.5f);
  }
  osc->waveform_scale = 1.0f;
  osc->is_noise = 0;
}

unsigned short *oscillator_generate_sine_wave(int table_size) {
  unsigned short *waveform = malloc(table_size * sizeof(unsigned short));
  if (!waveform)
    return NULL;

  for (int i = 0; i < table_size; i++) {
    float angle = 2.0f * M_PI * i / table_size;
    // Sine wave ranges from 0 to 65535
    waveform[i] = (unsigned short)((sinf(angle) + 1.0f) * 32767.5f);
  }
  return waveform;
}

unsigned short *oscillator_generate_square_wave(int table_size) {
  unsigned short *waveform = malloc(table_size * sizeof(unsigned short));
  if (!waveform)
    return NULL;

  for (int i = 0; i < table_size; i++) {
    if (i < table_size / 2)
      waveform[i] = 65535; // High
    else
      waveform[i] = 0; // Low
  }
  return waveform;
}

unsigned short *oscillator_generate_triangle_wave(int table_size) {
  unsigned short *waveform = malloc(table_size * sizeof(unsigned short));
  if (!waveform)
    return NULL;

  for (int i = 0; i < table_size; i++) {
    float phase = (float)i / table_size;
    float value;
    if (phase < 0.25f)
      value = 4.0f * phase;
    else if (phase < 0.75f)
      value = 2.0f - 4.0f * phase;
    else
      value = -4.0f + 4.0f * phase;
    // Normalize to 0 - 65535
    waveform[i] = (unsigned short)((value + 1.0f) * 32767.5f);
  }
  return waveform;
}

unsigned short *oscillator_generate_sawtooth_wave(int table_size) {
  unsigned short *wave =
      (unsigned short *)malloc(table_size * sizeof(unsigned short));
  for (int i = 0; i < table_size; i++) {
    wave[i] = (unsigned short)(65535 * i / table_size);
  }
  return wave;
}

void oscillator_set_noise_mode(oscillator *osc, int mode) {
  osc->noise_mode = mode;
}

void oscillator_set_freq(oscillator *osc, int freq) { osc->base_freq = freq; }

void oscillator_stop(oscillator *osc) { printf("DCO stopped\n"); }

void oscillator_play_note(oscillator *osc, int freq, float duration,
                          const char *waveform) {
  if (osc == NULL) {
    printf("DCO is NULL. Cannot play note.\n");
    return;
  }
  if (freq <= 20 || freq > 20000) {
    printf("Unknown note: %d. Defaulting to REST.\n", freq);
    return;
  }

  pthread_mutex_lock(&osc->mutex);

  if (waveform == NULL) {
    printf("Waveform is NULL. Defaulting to sine.\n");
    waveform = "sine";
  }

  oscillator_set_waveform(osc, waveform);
  oscillator_set_freq(osc, freq);
  osc->t = 0;
  pthread_mutex_unlock(&osc->mutex);

  PaStream *stream;
  PaError err;

  err = Pa_OpenDefaultStream(&stream, 0, 1, paFloat32, SAMPLE_RATE, 256,
                             audio_callback, osc);
  if (err != paNoError) {
    fprintf(stderr, "PortAudio error: %s\n", Pa_GetErrorText(err));
    return;
  }

  err = Pa_StartStream(stream);
  if (err != paNoError) {
    fprintf(stderr, "PortAudio error: %s\n", Pa_GetErrorText(err));
    return;
  }

  Pa_Sleep((int)(duration * 1000));

  err = Pa_StopStream(stream);
  if (err != paNoError) {
    fprintf(stderr, "PortAudio error: %s\n", Pa_GetErrorText(err));
    return;
  }

  err = Pa_CloseStream(stream);
  if (err != paNoError) {
    fprintf(stderr, "PortAudio error: %s\n", Pa_GetErrorText(err));
    return;
  }
}

void oscillator_play_note_struct(oscillator *osc, const Note *note) {
  pthread_mutex_lock(&osc->mutex);

  if (note->freq <= 0.0f) {
    oscillator_set_waveform(osc, "rest");
    osc->note_duration = note->duration;
    osc->note_time = 0.0f;
    osc->playing = 1;
    osc->release_started = 0;
    osc->release_start_time = 0.0f;
    osc->effect = EFFECT_NONE;
    pthread_mutex_unlock(&osc->mutex);
    return;
  }

  oscillator_set_waveform(osc, note->waveform_name);
  oscillator_set_freq(osc, note->freq);

  osc->attack_time = note->attack;
  osc->decay_time = note->decay;
  osc->sustain_level = note->sustain;
  osc->release_time = note->release;

  osc->note_duration = note->duration;
  osc->note_time = 0.0f;
  osc->playing = 1;
  osc->release_started = 0;
  osc->release_start_time = 0.0f;

  switch (note->effect) {
  case EFFECT_SWEEP_UP:
    osc->freq_start = note->freq;
    osc->freq_end = note->freq * 1.5f;
    break;
  case EFFECT_SWEEP_DOWN:
    osc->freq_start = note->freq;
    osc->freq_end = note->freq * 0.5f;
    break;
  case EFFECT_NONE:
  case EFFECT_VIBRATO:
  default:
    osc->freq_start = note->freq;
    osc->freq_end = note->freq;
    break;
  }

  osc->effect = note->effect;

  pthread_mutex_unlock(&osc->mutex);
}

typedef struct {
  oscillator *osc;
  Note *notes;
  int num_notes;
  volatile int *app_running;
} OscillatorSequenceData;

void *oscillator_play_sequence_thread(void *arg) {
  OscillatorSequenceData *data = (OscillatorSequenceData *)arg;
  oscillator *osc = data->osc;
  Note *notes = data->notes;
  int num_notes = data->num_notes;
  volatile int *app_running = data->app_running;

  int note_index = 0;
  double note_start_time = 0.0;
  int playing = 0;

  struct timespec ts_start, ts_now;
  clock_gettime(CLOCK_MONOTONIC, &ts_start);

  while (note_index < num_notes && *app_running) {
    clock_gettime(CLOCK_MONOTONIC, &ts_now);
    double elapsed_time = (ts_now.tv_sec - ts_start.tv_sec) +
                          (ts_now.tv_nsec - ts_start.tv_nsec) / 1e9;

    if (!playing) {
      Note current_note = notes[note_index];

      pthread_mutex_lock(&osc->mutex);

      int is_custom_waveform = 0;
      for (int i = 0; i < num_custom_waveforms; i++) {
        if (strcasecmp(current_note.waveform_name, custom_waveforms[i].name) ==
            0) {
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

      osc->note_duration = current_note.duration;
      osc->note_time = 0.0f;
      osc->playing = 1;
      osc->release_started = 0;
      osc->release_start_time = 0.0f;

      pthread_mutex_unlock(&osc->mutex);

      note_start_time = elapsed_time;
      playing = 1;
    } else {
      double note_elapsed_time = elapsed_time - note_start_time;
      Note current_note = notes[note_index];
      if (note_elapsed_time >= current_note.duration) {
        pthread_mutex_lock(&osc->mutex);
        osc->playing = 0;
        pthread_mutex_unlock(&osc->mutex);

        note_index++;
        playing = 0;
      }
    }

    usleep(10000);
  }

  pthread_mutex_lock(&osc->mutex);
  osc->playing = 0;
  pthread_mutex_unlock(&osc->mutex);

  free(data);
  return NULL;
}

void oscillator_play_sequence(oscillator *osc, Note notes[], int num_notes,
                              volatile int *app_running) {
  OscillatorSequenceData *data = malloc(sizeof(OscillatorSequenceData));
  data->osc = osc;
  data->notes = notes;
  data->num_notes = num_notes;
  data->app_running = app_running;

  pthread_t sequence_thread;
  pthread_create(&sequence_thread, NULL, oscillator_play_sequence_thread, data);
  pthread_detach(sequence_thread);
}
