#include "engine.h"
#include "input.h"
#include "renderer.h"
#include <GLFW/glfw3.h>
#include <stdlib.h>

void example_update(struct engine *e, struct renderer *renderer,
                    double delta_time) {
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
  renderer_add_rounded_rect(renderer, xpos, ypos, rounded_rect_width,
                            rounded_rect_height, rounded_rect_radius,
                            windowWidth, windowHeight, rounded_rect_color);

  float circle_radius = 15.0f;
  float circle_color[4] = {0.5f, 0.1f, 0.8f, 1.0f};
  renderer_add_circle(renderer, xpos - 30, ypos, circle_radius, windowWidth,
                      windowHeight, circle_color);

  float triangle_size = 40.0f;
  float triangle_color[4] = {1.0f, 0.0f, 0.0f, 1.0f};
  renderer_add_triangle(renderer, xpos + 30, ypos, triangle_size, windowWidth,
                        windowHeight, triangle_color);

  float text_color[4] = {231.0f / 255.0f, 130.0f / 255.0f, 132.0f / 255.0f,
                         1.0f};
  const char *debug_text = "DEBUG TEXT: ABCDEFGHIJKLMNOPQRSTUVWXYZ!@#$%^&*";
  renderer_add_text(renderer, debug_text, 25, 25, 0.4f, windowWidth,
                    windowHeight, text_color);
  const char *debug_text_lower_case =
      "debug text: abcdefghijklmnopqrstuvwxyz!@#$%^&*";
  renderer_add_text(renderer, debug_text_lower_case, 25, 45, 0.4f, windowWidth,
                    windowHeight, text_color);
}

void stress_test_example_update(struct engine *e, struct renderer *renderer,
                                double delta_time) {
  struct GLFWwindow *window = engine_get_window(e);
  renderer_reset_shape_count(renderer);

  int windowWidth, windowHeight;
  glfwGetFramebufferSize(window, &windowWidth, &windowHeight);

  int num_shapes = 1000;
  for (int i = 0; i < num_shapes; ++i) {
    float x = (float)(rand() % windowWidth);
    float y = (float)(rand() % windowHeight);
    float size = (float)(rand() % 50 + 10);
    float radius = (float)(rand() % 20 + 5);
    float color[4] = {(float)rand() / RAND_MAX, (float)rand() / RAND_MAX,
                      (float)rand() / RAND_MAX, 1.0f};

    int shape_type = rand() % 3;
    switch (shape_type) {
    case 0:
      renderer_add_circle(renderer, x, y, radius, windowWidth, windowHeight,
                          color);
      break;
    case 1:
      renderer_add_rounded_rect(renderer, x, y, size, size, radius, windowWidth,
                                windowHeight, color);
      break;
    case 2:
      renderer_add_triangle(renderer, x, y, size, windowWidth, windowHeight,
                            color);
      break;
    }
  }
}

void text_fill_example_update(struct engine *e, struct renderer *renderer,
                              double delta_time) {
  struct GLFWwindow *window = engine_get_window(e);
  renderer_reset_shape_count(renderer);

  int windowWidth, windowHeight;
  glfwGetFramebufferSize(window, &windowWidth, &windowHeight);

  float text_color[4] = {1.0f, 1.0f, 1.0f, 1.0f};
  const char *text = "Hello, World!";
  float scale = 0.5f;
  float y_offset = 20.0f;

  for (float y = 0; y < windowHeight; y += y_offset) {
    for (float x = 0; x < windowWidth; x += 100.0f) {
      renderer_add_text(renderer, text, x, y, scale, windowWidth, windowHeight,
                        text_color);
    }
  }
}
