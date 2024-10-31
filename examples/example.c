#include "glad.h"
#include "input.h"
#include <GLFW/glfw3.h>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include <stdio.h>
#include <stdlib.h>

#include "debug.h"
#include "engine.h"
#include "renderer.h"

void example_update(struct engine *e, struct renderer *renderer,
                    double delta_time) {
  struct GLFWwindow *window = engine_get_window(e);

  double xpos, ypos;
  struct MousePosition mpos = input_get_mouse_position(window);
  xpos = mpos.x;
  ypos = mpos.y;

  int windowWidth, windowHeight;
  glfwGetFramebufferSize(window, &windowWidth, &windowHeight);

  debug_update_frame_times((float)(delta_time * 1000.0));
}

void example_render(struct engine *e, struct renderer *renderer) {
  renderer_clear(renderer, 41.0f / 255.0f, 44.0f / 255.0f, 60.0f / 255.0f,
                 1.0f);

  struct GLFWwindow *window = engine_get_window(e);
  renderer_reset_shape_count(renderer);

  double xpos, ypos;
  struct MousePosition mpos = input_get_mouse_position(window);
  xpos = mpos.x;
  ypos = mpos.y;

  int windowWidth, windowHeight;
  glfwGetFramebufferSize(window, &windowWidth, &windowHeight);

  float rounded_rect_width = 200.0f;
  float rounded_rect_height = 150.0f;
  float rounded_rect_radius = 30.0f;
  float rounded_rect_color[4] = {0.0f, 0.7f, 0.2f, 1.0f};
  renderer_add_rounded_rect(renderer, xpos - rounded_rect_width / 2,
                            ypos - rounded_rect_height / 2, rounded_rect_width,
                            rounded_rect_height, rounded_rect_radius,
                            windowWidth, windowHeight, rounded_rect_color);

  float circle_radius = 15.0f;
  float circle_color[4] = {0.5f, 0.1f, 0.8f, 1.0f};
  renderer_add_circle(renderer, xpos - 30, ypos, circle_radius, windowWidth,
                      windowHeight, circle_color);

  float triangle_size = 40.0f;
  float triangle_color[4] = {1.0f, 0.0f, 0.0f, 1.0f};
  renderer_add_equilateral_triangle(renderer, xpos + 30, ypos, triangle_size,
                                    windowWidth, windowHeight, triangle_color);
  float regular_triangle_color[4] = {0.0f, 0.0f, 1.0f, 1.0f};
  renderer_add_triangle(renderer, xpos - 50, ypos - 50, xpos + 50, ypos - 50,
                        xpos, ypos, windowWidth, windowHeight,
                        regular_triangle_color);

  float text_color[4] = {231.0f / 255.0f, 130.0f / 255.0f, 132.0f / 255.0f,
                         1.0f};
  const char *debug_text = "DEBUG TEXT: ABCDEFGHIJKLMNOPQRSTUVWXYZ!@#$%^&*";
  renderer_add_text(renderer, debug_text, 25, 25, 0.4f, windowWidth,
                    windowHeight, text_color);
  const char *debug_text_lower_case =
      "debug text: abcdefghijklmnopqrstuvwxyz!@#$%^&*";
  renderer_add_text(renderer, debug_text_lower_case, 25, 45, 0.4f, windowWidth,
                    windowHeight, text_color);

  renderer_add_rounded_rect(renderer, xpos, ypos, 32 * 5, 32 * 5, 1,
                            windowWidth, windowHeight,
                            (float[4]){0.0f, 0.0f, 0.0f, 1.0f});
  struct TextureRenderOptions options = {.x = 100.0f,
                                         .y = 100.0f,
                                         .width = 128,
                                         .height = 32,
                                         .rect_x = 0,
                                         .rect_y = 0,
                                         .rect_width = 32,
                                         .rect_height = 32,
                                         .screen_width = windowWidth,
                                         .screen_height = windowHeight,
                                         .flip_x = 0,
                                         .flip_y = 0,
                                         .scale = 1.0f,
                                         .rotation = 0.0f};

  options.x = xpos;
  options.y = ypos;
  options.scale = 5.0f;
  renderer_add_texture(renderer, &options);
  options.x = 100.0f;
  options.y = 100.0f;
  options.scale = 1.0f;
  renderer_add_texture(renderer, &options);

  options.x = 140.0f;
  options.flip_x = 1;
  renderer_add_texture(renderer, &options);
  options.x = 100.0f;
  options.y = 130.0f;
  options.rect_x = 32;
  options.flip_x = 0;
  renderer_add_texture(renderer, &options);

  options.y = 160.0f;
  options.rect_x = 64;
  options.flip_x = 0;
  options.flip_y = 1;
  renderer_add_texture(renderer, &options);

  options.y = 190.0f;
  options.rect_x = 96;
  options.flip_y = 0;
  options.rotation = 45.0f * (3.14159265f / 180.0f);
  renderer_add_texture(renderer, &options);

  options.y = 325.0f;
  options.x = 160.0f;
  options.rect_x = 0;
  options.scale = 8.0f;
  options.rotation = 0;
  renderer_add_texture(renderer, &options);

  debug_render_frame_time_graph(renderer, windowWidth, windowHeight);

  float avg_frame_time = debug_calculate_average_frame_time();
  char avg_frame_time_text[256];
  sprintf(avg_frame_time_text, "Avg Frame Time: %.2f ms", avg_frame_time);
  renderer_add_text(renderer, avg_frame_time_text, 25, 65, 0.4f, windowWidth,
                    windowHeight, text_color);

  renderer_draw(renderer);
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
        struct engine *e = engine_new(240 * 3, 160 * 3);
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

        engine_set_update_callback(e, example_update);
        engine_set_render_callback(e, example_render);
        engine_run(e);
        engine_destroy(e);

        stbi_image_free(data);

        return 0;
}
