#ifndef FONT_H
#define FONT_H

#include "glad.h"
#include <GLFW/glfw3.h>

#include "stb_truetype.h"

typedef struct {
  stbtt_bakedchar *cdata;
  GLuint texture_id;
  float scale;
} Font;

int font_load(const char *font_path, Font *font);

#endif // FONT_H
