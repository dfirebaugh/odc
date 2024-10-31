#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "audio.h"
#include "engine.h"
#include "input.h"
#include "note_parser.h"
#include "oscillator.h"
#include "renderer.h"

OscillatorThreadData thread_data;

void draw_waveform(struct renderer *renderer, oscillator **oscillators,
                   int num_channels, int screen_width, int screen_height) {
  float colors[4][4] = {
      {1.0f, 0.0f, 0.0f, 1.0f}, // Red
      {0.0f, 1.0f, 0.0f, 1.0f}, // Green
      {0.0f, 0.0f, 1.0f, 1.0f}, // Blue
      {1.0f, 1.0f, 0.0f, 1.0f}  // Yellow
  };

  int waveform_height = screen_height / num_channels;
  int waveform_width = screen_width - 40;
  int waveform_offset_x = 20;

  for (int ch = 0; ch < num_channels; ch++) {
    oscillator *osc = oscillators[ch];
    float *color = colors[ch % 4];

    int waveform_offset_y = waveform_height * ch + waveform_height / 2;

    pthread_mutex_lock(&osc->mutex);

    int buffer_size = SAMPLE_BUFFER_SIZE;
    int start_index = osc->sample_buffer_index;

    float scale = (float)waveform_height / 2;
    if (ch == 3) {
      scale = 1000;
    }

    int samples_to_draw = waveform_width;
    if (samples_to_draw > buffer_size) {
      samples_to_draw = buffer_size - 1;
    }

    for (int i = 0; i < samples_to_draw - 1; i++) {
      int index1 = (start_index + i) % buffer_size;
      int index2 = (start_index + i + 1) % buffer_size;

      float x1 = waveform_offset_x + i;
      float x2 = waveform_offset_x + i + 1;

      float y1 = waveform_offset_y + osc->sample_buffer[index1] * scale;
      float y2 = waveform_offset_y + osc->sample_buffer[index2] * scale;

      renderer_add_triangle(renderer, x1, y1, x2, y2, x2, y2 - 1, screen_width,
                            screen_height, color);
      renderer_add_triangle(renderer, x1, y1, x1, y1 - 1, x2, y2, screen_width,
                            screen_height, color);
    }

    pthread_mutex_unlock(&osc->mutex);
  }
}

void example_render(struct engine *e, struct renderer *renderer) {
  int windowWidth, windowHeight;
  glfwGetFramebufferSize(engine_get_window(e), &windowWidth, &windowHeight);
  renderer_clear_vertices(renderer);
  renderer_clear(renderer, 41.0f / 255.0f, 44.0f / 255.0f, 60.0f / 255.0f,
                 1.0f);

  oscillator **audio_data = (oscillator **)engine_get_audio_data(e);

  draw_waveform(renderer, audio_data, NUM_CHANNELS, windowWidth, windowHeight);

  const char *text = "press space to play";
  float text_color[4] = {231.0f / 255.0f, 130.0f / 255.0f, 132.0f / 255.0f,
                         1.0f};
  renderer_add_text(renderer, text, 25, 30, 0.4f, windowWidth, windowHeight,
                    text_color);

  renderer_draw(renderer);
  renderer_reset_shape_count(renderer);
}

void example_update(struct engine *e, struct renderer *renderer,
                    double delta_time) {
  struct GLFWwindow *window = engine_get_window(e);
  int windowWidth, windowHeight;
  glfwGetFramebufferSize(window, &windowWidth, &windowHeight);
  oscillator **oscillators = (oscillator **)engine_get_audio_data(e);
  int is_playback_active = audio_is_playback_active();

  if (is_button_just_pressed(window, GLFW_KEY_Z) && !is_playback_active) {
    Note single_note = {.freq = 440.0f,
                        .duration = 2.0f,
                        .waveform_name = "sine",
                        .attack = 0.1f,
                        .decay = 0.2f,
                        .sustain = 0.7f,
                        .release = 0.5f,
                        .effect = EFFECT_NONE};

    oscillator_play_note_struct(oscillators[0], &single_note);
  }

  if (is_button_just_pressed(window, GLFW_KEY_SPACE) && !is_playback_active) {
    for (int i = 0; i < NUM_CHANNELS; i++) {
      for (int j = 0; j < thread_data.num_notes[i]; j++) {
        free(thread_data.sequences[i][j].waveform_name);
      }
      free(thread_data.sequences[i]);
    }
    free(thread_data.sequences);
    free(thread_data.num_notes);

    parse_notes_from_file("notes.txt", &thread_data.sequences,
                          &thread_data.num_notes);

    pthread_t audio_thread;
    pthread_create(&audio_thread, NULL, audio_play_sequence_thread,
                   &thread_data);
    pthread_detach(audio_thread);
    audio_set_playback_active(1);
  }
}

int main() {
  if (audio_init() != 0) {
    return -1;
  }

  oscillator *oscillators[NUM_CHANNELS];

  for (int i = 0; i < NUM_CHANNELS; i++) {
    oscillators[i] = malloc(sizeof(oscillator));
    *oscillators[i] = oscillator_init(440, 8192, 0.15f);
    oscillator_set_waveform(oscillators[i], "sine");
  }

  struct engine *e = engine_new(240 * 3, 160 * 3);
  if (!e) {
    audio_terminate();
    return -1;
  }

  int app_running = 1;

  AudioData audio_data;
  audio_data.oscillators = oscillators;
  audio_data.app_running = &app_running;

  PaStream *stream;
  PaError err = Pa_OpenDefaultStream(&stream, 0, 1, paFloat32, SAMPLE_RATE, 256,
                                     audio_callback, &audio_data);
  if (err != paNoError) {
    fprintf(stderr, "PortAudio error: %s\n", Pa_GetErrorText(err));
    return -1;
  }
  if (err != paNoError) {
    fprintf(stderr, "PortAudio error: %s\n", Pa_GetErrorText(err));
    return -1;
  }

  err = Pa_StartStream(stream);
  if (err != paNoError) {
    fprintf(stderr, "PortAudio error: %s\n", Pa_GetErrorText(err));
    return -1;
  }

  Note **sequences;
  int *num_notes;

  parse_notes_from_file("notes.txt", &sequences, &num_notes);

  thread_data.oscillators = oscillators;
  thread_data.sequences = sequences;
  thread_data.num_notes = num_notes;
  thread_data.num_channels = NUM_CHANNELS;
  thread_data.app_running = &app_running;

  for (int i = 0; i < NUM_CHANNELS; i++) {
    oscillators[i]->max_volume = 0.4f;
  }

  engine_set_update_callback(e, example_update);
  engine_set_render_callback(e, example_render);
  engine_set_audio_data(e, oscillators);
  engine_run(e);

  app_running = 0;

  err = Pa_StopStream(stream);
  if (err != paNoError) {
    fprintf(stderr, "PortAudio error: %s\n", Pa_GetErrorText(err));
    return -1;
  }

  err = Pa_CloseStream(stream);
  if (err != paNoError) {
    fprintf(stderr, "PortAudio error: %s\n", Pa_GetErrorText(err));
    return -1;
  }

  engine_destroy(e);

  for (int i = 0; i < NUM_CHANNELS; i++) {
    if (oscillators[i]->current_waveform) {
      free(oscillators[i]->current_waveform);
      oscillators[i]->current_waveform = NULL;
    }
    free(oscillators[i]);
    for (int j = 0; j < num_notes[i]; j++) {
      free(sequences[i][j].waveform_name);
    }
    free(sequences[i]);
  }
  free(sequences);
  free(num_notes);

  for (int i = 0; i < num_custom_waveforms; i++) {
    free(custom_waveforms[i].samples);
  }

  audio_terminate();

  return 0;
}
