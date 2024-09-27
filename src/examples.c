#include "engine.h"
#include "input.h"
#include "renderer.h"

void example_update(struct engine *e, shape_renderer *renderer,
                    double delta_time) {
  renderer->shape_count = 0;

  double xpos, ypos;
  struct MousePosition mpos = input_get_mouse_position(e->window);
  xpos = mpos.x;
  ypos = mpos.y;

  int windowWidth, windowHeight;
  glfwGetFramebufferSize(e->window, &windowWidth, &windowHeight);

  float rounded_rect_width = 200.0f;
  float rounded_rect_height = 150.0f;
  float rounded_rect_radius = 30.0f;
  float rounded_rect_color[4] = {0.0f, 0.7f, 0.2f, 1.0f};
  shape_renderer_add_rounded_rect(
      renderer, xpos, ypos, rounded_rect_width, rounded_rect_height,
      rounded_rect_radius, windowWidth, windowHeight, rounded_rect_color);

  float circle_radius = 15.0f;
  float circle_color[4] = {0.5f, 0.1f, 0.8f, 1.0f};
  shape_renderer_add_circle(renderer, xpos - 30, ypos, circle_radius,
                            windowWidth, windowHeight, circle_color);

  float triangle_size = 40.0f;
  float triangle_color[4] = {1.0f, 0.0f, 0.0f, 1.0f};
  shape_renderer_add_triangle(renderer, xpos + 30, ypos, triangle_size,
                              windowWidth, windowHeight, triangle_color);
}
