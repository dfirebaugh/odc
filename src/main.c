#include "glad.h"
#include <GLFW/glfw3.h>

#include <stdio.h>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include "engine.h"
#include "examples.h"
#include "input.h"
#include "renderer.h"

void update(struct engine *e, struct renderer *renderer, double delta_time) {
  GLFWwindow *window = engine_get_window(e);
  if (is_mouse_button_just_pressed(window, GLFW_MOUSE_BUTTON_LEFT)) {
    struct MousePosition m = input_get_mouse_position(window);
    printf("x: %d, y: %d\n", (int)m.x, (int)m.y);
  }

  /*example_update(e, renderer, delta_time);*/
  buddymark_example_update(e, renderer, delta_time);
}
void flip_image_vertically(unsigned char *data, int width, int height,
                           int channels) {
  int stride = width * channels;
  unsigned char *row = (unsigned char *)malloc(stride);
  if (!row) {
    fprintf(stderr, "Failed to allocate memory for row buffer\n");
    return;
  }

  for (int y = 0; y < height / 2; ++y) {
    unsigned char *top = data + y * stride;
    unsigned char *bottom = data + (height - y - 1) * stride;

    memcpy(row, top, stride);
    memcpy(top, bottom, stride);
    memcpy(bottom, row, stride);
  }

  free(row);
}

int main() {
  struct engine *e = engine_new(600, 600);
  if (!e) {
    return -1;
  }
  char *file_path = "./assets/images/buddy_dance.png";
  int width, height, channels;
  unsigned char *data = stbi_load(file_path, &width, &height, &channels, 4);
  if (!data) {
    fprintf(stderr, "Failed to load texture: %s\n", file_path);
    return 1;
  }
  flip_image_vertically(data, width, height, channels);
  renderer_upload_texture_atlas(engine_get_renderer(e), data, width, height);

  engine_set_update_callback(e, update);
  engine_run(e);
  engine_destroy(e);

  stbi_image_free(data);

  return 0;
}
