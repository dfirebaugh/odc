#include "input.h"
#include <GLFW/glfw3.h>
#include <stdio.h>
#include <stdlib.h>

#include "engine.h"
#include "renderer.h"

#define MAX_FRAME_TIMES 100

static float frameTimes[MAX_FRAME_TIMES];
static int frameTimeIndex = 0;

float calculate_average_frame_time() {
  float sum = 0.0f;
  for (int i = 0; i < MAX_FRAME_TIMES; ++i) {
    sum += frameTimes[i];
  }
  return sum / MAX_FRAME_TIMES;
}

void update_frame_times(float frameTime) {
  frameTimes[frameTimeIndex] = frameTime;
  frameTimeIndex = (frameTimeIndex + 1) % MAX_FRAME_TIMES;
}

void add_rectangle_with_triangles(struct renderer *renderer, float x, float y,
                                  float width, float height, int screen_width,
                                  int screen_height, float *color) {
  float x1 = x;
  float y1 = y;
  float x2 = x + width;
  float y2 = y;
  float x3 = x + width;
  float y3 = y + height;
  float x4 = x;
  float y4 = y + height;

  renderer_add_triangle(renderer, x1, y1, x2, y2, x3, y3, screen_width,
                        screen_height, color);
  renderer_add_triangle(renderer, x1, y1, x3, y3, x4, y4, screen_width,
                        screen_height, color);
}
void render_frame_time_graph(struct renderer *renderer, int screen_width,
                             int screen_height) {
  float barWidth = (float)screen_width / MAX_FRAME_TIMES;
  float maxFrameTime = 50.0f;

  for (int i = 0; i < MAX_FRAME_TIMES; ++i) {
    int index = (frameTimeIndex + i) % MAX_FRAME_TIMES;
    float frameTime = frameTimes[index];
    float barHeight = (frameTime / maxFrameTime) * screen_height;

    float x = i * barWidth;
    float y = screen_height - barHeight;

    float color[4] = {1.0f, 0.0f, 0.0f, 1.0f};
    add_rectangle_with_triangles(renderer, x, y, barWidth, barHeight,
                                 screen_width, screen_height, color);
  }
}

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

  update_frame_times((float)(delta_time * 1000.0));
  render_frame_time_graph(renderer, windowWidth, windowHeight);

  float avg_frame_time = calculate_average_frame_time();
  char avg_frame_time_text[256];
  sprintf(avg_frame_time_text, "Avg Frame Time: %.2f ms", avg_frame_time);
  renderer_add_text(renderer, avg_frame_time_text, 25, 65, 0.4f, windowWidth,
                    windowHeight, text_color);
}

#define MAX_BUDDIES 100000
#define GRAVITY 0.1f
#define DAMPING 0.9f

struct Buddy {
  float x, y;
  float vx, vy;
  struct TextureRenderOptions options;
  int frame_index;
  double frame_time;
};

static struct Buddy bunnies[MAX_BUDDIES];
static int bunny_count = 0;

void add_buddy(int windowWidth, int windowHeight) {
  if (bunny_count >= MAX_BUDDIES)
    return;

  struct Buddy *bunny = &bunnies[bunny_count++];
  bunny->x = rand() % windowWidth;
  bunny->y = rand() % windowHeight;
  bunny->vx = (float)(rand() % 200 - 100) / 60.0f;
  bunny->vy = (float)(rand() % 200 - 100) / 60.0f;
  bunny->frame_index = 0;
  bunny->frame_time = 0.0;

  bunny->options = (struct TextureRenderOptions){.x = bunny->x,
                                                 .y = bunny->y,
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
}

void buddymark_example_update(struct engine *e, struct renderer *renderer,
                              double delta_time) {
  struct GLFWwindow *window = engine_get_window(e);
  renderer_reset_shape_count(renderer);

  int windowWidth, windowHeight;
  windowWidth = 600;
  windowHeight = 600;

  if (is_mouse_button_just_pressed(window, GLFW_MOUSE_BUTTON_LEFT)) {
    for (int i = 0; i < 500; ++i) {
      add_buddy(windowWidth, windowHeight);
    }
  }
  if (is_mouse_button_just_pressed(window, GLFW_MOUSE_BUTTON_RIGHT)) {
    bunny_count = 0;
    renderer_clear_vertices(renderer);
    return;
  }

  for (int i = 0; i < bunny_count; ++i) {
    struct Buddy *bunny = &bunnies[i];
    bunny->vy += GRAVITY;

    bunny->x += bunny->vx * delta_time * 60.0f;
    bunny->y += bunny->vy * delta_time * 60.0f;

    if (bunny->x < 0) {
      bunny->x = 0;
      bunny->vx *= -DAMPING;
    } else if (bunny->x > windowWidth - 32) {
      bunny->x = windowWidth - 32;
      bunny->vx *= -DAMPING;
    }

    if (bunny->y > windowHeight - 32) {
      bunny->y = windowHeight - 32;
      bunny->vy *= -DAMPING;
    }

    bunny->options.x = bunny->x;
    bunny->options.y = bunny->y;

    bunny->frame_time += delta_time;
    if (bunny->frame_time >= 0.1) {
      bunny->frame_time = 0.0;
      bunny->frame_index = (bunny->frame_index + 1) % 4;
      bunny->options.rect_x = bunny->frame_index * 32;
    }

    renderer_add_texture(renderer, &bunny->options);
  }

  float rounded_rect_width = 240.0f;
  float rounded_rect_height = 120.0f;
  float rounded_rect_radius = 20.0f;
  float rounded_rect_color[4] = {48.0f / 255, 52.0f / 255, 70.0f / 255, 1.0f};
  renderer_add_rounded_rect(renderer, 135, 65, rounded_rect_width,
                            rounded_rect_height, rounded_rect_radius,
                            windowWidth, windowHeight, rounded_rect_color);

  static char fps_text[256] = "";
  static char buddies_text[256] = "";
  static char avg_frame_time_text[256] = "";

  static double text_update_timer = 0.0;
  text_update_timer += delta_time;
  if (text_update_timer >= 0.2) {
    text_update_timer = 0.0;
    sprintf(fps_text, "fps: %d", engine_get_fps(e));
    sprintf(buddies_text, "buddies: %d", bunny_count);
    float avg_frame_time = calculate_average_frame_time();
    sprintf(avg_frame_time_text, "Avg Frame Time: %.1f ms", avg_frame_time);
  }

  float text_color[4] = {231.0f / 255.0f, 130.0f / 255.0f, 132.0f / 255.0f,
                         1.0f};
  renderer_add_text(renderer, fps_text, 25, 35, 0.4f, windowWidth, windowHeight,
                    text_color);
  renderer_add_text(renderer, buddies_text, 25, 75, 0.4f, windowWidth,
                    windowHeight, text_color);
  renderer_add_text(renderer, avg_frame_time_text, 25, 105, 0.4f, windowWidth,
                    windowHeight, text_color);

  update_frame_times((float)(delta_time * 1000.0));
  render_frame_time_graph(renderer, windowWidth, windowHeight);
}
