#include <stdio.h>
#include <stdlib.h>

#include "font.h"
#define STB_TRUETYPE_IMPLEMENTATION
#include "stb_truetype.h"

int font_load(const char *font_path, Font *font) {
  FILE *file = fopen(font_path, "rb");
  if (!file) {
    fprintf(stderr, "Failed to open font file: %s\n", font_path);
    return -1;
  }

  fseek(file, 0, SEEK_END);
  size_t size = ftell(file);
  fseek(file, 0, SEEK_SET);

  unsigned char *font_buffer = (unsigned char *)malloc(size);
  if (!font_buffer) {
    fprintf(stderr, "Failed to allocate memory for font buffer\n");
    fclose(file);
    return -1;
  }

  fread(font_buffer, 1, size, file);
  fclose(file);

  font->cdata = (stbtt_bakedchar *)malloc(96 * sizeof(stbtt_bakedchar));
  if (!font->cdata) {
    fprintf(stderr, "Failed to allocate memory for character data\n");
    free(font_buffer);
    return -1;
  }

  unsigned char *bitmap = (unsigned char *)malloc(512 * 512);
  if (!bitmap) {
    fprintf(stderr, "Failed to allocate memory for bitmap\n");
    free(font->cdata);
    free(font_buffer);
    return -1;
  }

  stbtt_BakeFontBitmap(font_buffer, 0, 48.0, bitmap, 512, 512, 32, 96,
                       font->cdata);
  free(font_buffer);

  glGenTextures(1, &font->texture_id);
  glBindTexture(GL_TEXTURE_2D, font->texture_id);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, 512, 512, 0, GL_RED, GL_UNSIGNED_BYTE,
               bitmap);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

  GLenum error = glGetError();
  if (error != GL_NO_ERROR) {
    fprintf(stderr, "OpenGL error after setting texture parameters: %d\n",
            error);
  }

  free(bitmap);

  font->scale = 1.0f / 512.0f;
  return 0;
}
