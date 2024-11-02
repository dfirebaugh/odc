#include <ctype.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "odc_audio.h"
#include "odc_oscillator.h"

#define DEFAULT_TEMPO 120

typedef struct {
  const char *name;
  float frequency;
} note_frequency;

static note_frequency note_frequencies[] = {
    {"C0", 16.35},    {"C#0", 17.32},   {"D0", 18.35},    {"D#0", 19.45},
    {"E0", 20.60},    {"F0", 21.83},    {"F#0", 23.12},   {"G0", 24.50},
    {"G#0", 25.96},   {"A0", 27.50},    {"A#0", 29.14},   {"B0", 30.87},

    {"C1", 32.70},    {"C#1", 34.65},   {"D1", 36.71},    {"D#1", 38.89},
    {"E1", 41.20},    {"F1", 43.65},    {"F#1", 46.25},   {"G1", 49.00},
    {"G#1", 51.91},   {"A1", 55.00},    {"A#1", 58.27},   {"B1", 61.74},

    {"C2", 65.41},    {"C#2", 69.30},   {"D2", 73.42},    {"D#2", 77.78},
    {"E2", 82.41},    {"F2", 87.31},    {"F#2", 92.50},   {"G2", 98.00},
    {"G#2", 103.83},  {"A2", 110.00},   {"A#2", 116.54},  {"B2", 123.47},

    {"C3", 130.81},   {"C#3", 138.59},  {"D3", 146.83},   {"D#3", 155.56},
    {"E3", 164.81},   {"F3", 174.61},   {"F#3", 185.00},  {"G3", 196.00},
    {"G#3", 207.65},  {"A3", 220.00},   {"A#3", 233.08},  {"B3", 246.94},

    {"C4", 261.63},   {"C#4", 277.18},  {"D4", 293.66},   {"D#4", 311.13},
    {"E4", 329.63},   {"F4", 349.23},   {"F#4", 369.99},  {"G4", 392.00},
    {"G#4", 415.30},  {"A4", 440.00},   {"A#4", 466.16},  {"B4", 493.88},

    {"C5", 523.25},   {"C#5", 554.37},  {"D5", 587.33},   {"D#5", 622.25},
    {"E5", 659.25},   {"F5", 698.46},   {"F#5", 739.99},  {"G5", 783.99},
    {"G#5", 830.61},  {"A5", 880.00},   {"A#5", 932.33},  {"B5", 987.77},

    {"C6", 1046.50},  {"C#6", 1108.73}, {"D6", 1174.66},  {"D#6", 1244.51},
    {"E6", 1318.51},  {"F6", 1396.91},  {"F#6", 1479.98}, {"G6", 1567.98},
    {"G#6", 1661.22}, {"A6", 1760.00},  {"A#6", 1864.66}, {"B6", 1975.53},

    {"C7", 2093.00},  {"C#7", 2217.46}, {"D7", 2349.32},  {"D#7", 2489.02},
    {"E7", 2637.02},  {"F7", 2793.83},  {"F#7", 2959.96}, {"G7", 3135.96},
    {"G#7", 3322.44}, {"A7", 3520.00},  {"A#7", 3729.31}, {"B7", 3951.07},

    {"C8", 4186.01},  {"C#8", 4434.92}, {"D8", 4698.63},  {"D#8", 4978.03},
    {"E8", 5274.04},  {"F8", 5587.65},  {"F#8", 5919.91}, {"G8", 6271.93},
    {"G#8", 6644.88}, {"A8", 7040.00},  {"A#8", 7458.62}, {"B8", 7902.13},

    {"REST", -1.0},   {NULL, 0.0}};

static float get_frequency_static(const char *note_name);
static waveform_type get_waveform_static(const char *waveform_name);
static effect_type get_effect_static(const char *effect_name);
static void parse_tempo_directive(const char *line, float *tempo);
static void parse_define_directive(const char *line);
static void parse_note_line(const char *line, float tempo,
                            oscillator_note ***sequences, int *num_notes);
static void initialize_sequences(oscillator_note ***sequences, int **num_notes);

void odc_parse_notes_from_file(const char *filename,
                               oscillator_note ***sequences, int **num_notes) {
  FILE *file = fopen(filename, "r");
  if (!file) {
    fprintf(stderr, "Failed to open notes file: %s\n", filename);
    exit(EXIT_FAILURE);
  }

  initialize_sequences(sequences, num_notes);

  float tempo = DEFAULT_TEMPO;

  char line[256];
  while (fgets(line, sizeof(line), file)) {
    char *trimmed_line = line;
    while (isspace(*trimmed_line))
      trimmed_line++;

    if (*trimmed_line == '#' || *trimmed_line == '\0' || *trimmed_line == '\n')
      continue;

    if (strncasecmp(trimmed_line, "TEMPO", 5) == 0) {
      parse_tempo_directive(trimmed_line, &tempo);
    } else if (strncasecmp(trimmed_line, "DEFINE", 6) == 0) {
      parse_define_directive(trimmed_line);
    } else {
      parse_note_line(trimmed_line, tempo, sequences, *num_notes);
    }
  }

  fclose(file);
}

static void initialize_sequences(oscillator_note ***sequences,
                                 int **num_notes) {
  *sequences = malloc(NUM_CHANNELS * sizeof(oscillator_note *));
  if (!*sequences) {
    fprintf(stderr, "Memory allocation failed for sequences.\n");
    exit(EXIT_FAILURE);
  }

  *num_notes = malloc(NUM_CHANNELS * sizeof(int));
  if (!*num_notes) {
    fprintf(stderr, "Memory allocation failed for num_notes.\n");
    exit(EXIT_FAILURE);
  }

  for (int i = 0; i < NUM_CHANNELS; i++) {
    (*sequences)[i] = NULL;
    (*num_notes)[i] = 0;
  }
}

static float get_frequency_static(const char *note_name) {
  for (int i = 0; note_frequencies[i].name != NULL; i++) {
    if (strcasecmp(note_name, note_frequencies[i].name) == 0) {
      return note_frequencies[i].frequency;
    }
  }
  fprintf(stderr, "Unknown note: %s. Defaulting to REST.\n", note_name);
  return -1.0f;
}

static waveform_type get_waveform_static(const char *waveform_name) {
  if (strcasecmp(waveform_name, "SINE") == 0)
    return WAVEFORM_SINE;
  else if (strcasecmp(waveform_name, "SQUARE") == 0)
    return WAVEFORM_SQUARE;
  else if (strcasecmp(waveform_name, "SAWTOOTH") == 0)
    return WAVEFORM_SAWTOOTH;
  else if (strcasecmp(waveform_name, "TRIANGLE") == 0)
    return WAVEFORM_TRIANGLE;
  else if (strcasecmp(waveform_name, "NOISE") == 0)
    return WAVEFORM_NOISE;
  else {
    fprintf(stderr, "Unknown waveform: %s. Defaulting to SINE.\n",
            waveform_name);
    return WAVEFORM_SINE;
  }
}

static effect_type get_effect_static(const char *effect_name) {
  if (strcasecmp(effect_name, "NONE") == 0)
    return EFFECT_NONE;
  else if (strcasecmp(effect_name, "SWEEP_UP") == 0)
    return EFFECT_SWEEP_UP;
  else if (strcasecmp(effect_name, "SWEEP_DOWN") == 0)
    return EFFECT_SWEEP_DOWN;
  else if (strcasecmp(effect_name, "VIBRATO") == 0)
    return EFFECT_VIBRATO;
  else {
    fprintf(stderr, "Unknown effect: %s. Defaulting to NONE.\n", effect_name);
    return EFFECT_NONE;
  }
}

static void parse_tempo_directive(const char *line, float *tempo) {
  if (sscanf(line, "TEMPO %f", tempo) == 1) {
    printf("Set tempo to %.2f BPM\n", *tempo);
  } else {
    fprintf(stderr, "Invalid TEMPO directive: %s\n", line);
  }
}

static void parse_define_directive(const char *line) {
  if (odc_num_custom_waveforms >= MAX_CUSTOM_WAVEFORMS) {
    fprintf(stderr, "Maximum number of custom waveforms reached.\n");
    return;
  }

  char waveform_name[16];
  char *token = NULL;
  char *copy_line = strdup(line);
  if (!copy_line) {
    fprintf(stderr,
            "Memory allocation failed while parsing DEFINE directive.\n");
    return;
  }

  token = strtok(copy_line, " \t\n");
  token = strtok(NULL, " \t\n");
  if (token) {
    strncpy(waveform_name, token, 15);
    waveform_name[15] = '\0';
  } else {
    fprintf(stderr, "Invalid DEFINE directive: %s\n", line);
    free(copy_line);
    return;
  }

  float *samples = NULL;
  int num_samples = 0;
  int capacity = 0;

  while ((token = strtok(NULL, " \t\n")) != NULL) {
    int value = atoi(token);
    if (value < 0 || value > 255) {
      fprintf(stderr, "Waveform value out of range (0-255): %d\n", value);
      continue;
    }
    if (num_samples >= capacity) {
      capacity += 16;
      float *temp = realloc(samples, capacity * sizeof(float));
      if (!temp) {
        fprintf(stderr,
                "Memory allocation failed while parsing waveform samples.\n");
        free(samples);
        free(copy_line);
        return;
      }
      samples = temp;
    }
    samples[num_samples++] = (value / 127.5f) - 1.0f;
  }

  if (num_samples > 0) {
    custom_waveform *waveform =
        &odc_custom_waveforms[odc_num_custom_waveforms++];
    strncpy(waveform->name, waveform_name, 15);
    waveform->name[15] = '\0';
    waveform->samples = samples;
    waveform->num_samples = num_samples;
    printf("Defined custom waveform: %s with %d samples\n", waveform->name,
           waveform->num_samples);
  } else {
    fprintf(stderr, "No samples provided for waveform: %s\n", waveform_name);
    free(samples);
  }

  free(copy_line);
}

static void parse_note_line(const char *line, float tempo,
                            oscillator_note ***sequences, int *num_notes) {
  int channel;
  char note_name[16];
  char note_length_str[16];
  char waveform_name[16];
  float attack, decay, sustain, release;
  char effect_name[16];
  float sweep_end = 0.0f; // Default value

  int items_read =
      sscanf(line, "%d %15s %15s %15s %f %f %f %f %15s %f", &channel, note_name,
             note_length_str, waveform_name, &attack, &decay, &sustain,
             &release, effect_name, &sweep_end);

  if (items_read < 9) {
    fprintf(stderr, "Invalid line in notes file: %s\n", line);
    return;
  }

  if (channel < 0 || channel >= NUM_CHANNELS) {
    fprintf(stderr, "Invalid channel number: %d\n", channel);
    return;
  }

  float freq = get_frequency_static(note_name);
  int waveform = get_waveform_static(waveform_name);
  effect_type effect = get_effect_static(effect_name);

  float duration = 0.0f;
  int numerator, denominator;
  if (sscanf(note_length_str, "%d/%d", &numerator, &denominator) == 2 &&
      denominator != 0) {
    float note_length = (float)numerator / (float)denominator;
    duration = (60.0f / tempo) * note_length;
  } else if (sscanf(note_length_str, "%f", &duration) == 1) {
    // duration already set
  } else {
    fprintf(stderr, "Invalid note length: %s. Defaulting to quarter note.\n",
            note_length_str);
    duration = (60.0f / tempo) * (1.0f / 4.0f);
  }

  int n = num_notes[channel];
  oscillator_note *temp =
      realloc((*sequences)[channel], (n + 1) * sizeof(oscillator_note));
  if (!temp) {
    fprintf(stderr, "Memory allocation failed for channel %d notes.\n",
            channel);
    return;
  }
  (*sequences)[channel] = temp;

  oscillator_note *note = &((*sequences)[channel][n]);
  note->freq = freq;
  note->duration = duration;
  note->attack = attack;
  note->decay = decay;
  note->sustain = sustain;
  note->release = release;
  note->effect = effect;
  note->sweep_end = sweep_end;
  note->waveform_name = strdup(waveform_name);

  if (!note->waveform_name) {
    fprintf(stderr, "Memory allocation failed for waveform name.\n");
    return;
  }

  if (effect == EFFECT_SWEEP_UP || effect == EFFECT_SWEEP_DOWN) {
    if (items_read >= 10) {
    } else {
      note->sweep_end =
          (effect == EFFECT_SWEEP_UP) ? note->freq * 1.5f : note->freq * 0.5f;
    }
  }

  num_notes[channel]++;
  printf("Parsed note: Channel %d, Note %s, Duration %.2f sec, Waveform %s, "
         "ADSR(%.2f, %.2f, %.2f, %.2f), Effect %d, Sweep End %.2f Hz\n",
         channel, note_name, duration, waveform_name, attack, decay, sustain,
         release, effect, note->sweep_end);
}
