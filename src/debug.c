#include "odc_renderer.h"

#define MAX_FRAME_TIMES 100

static float frameTimes[MAX_FRAME_TIMES];
static int frameTimeIndex = 0;
static double last_delta_time = 0.0;

float odc_debug_calculate_average_frame_time() {
  float sum = 0.0f;
  for (int i = 0; i < MAX_FRAME_TIMES; ++i) {
    sum += frameTimes[i];
  }
  return sum / MAX_FRAME_TIMES;
}

void odc_debug_update_frame_times(float frameTime) {
  frameTimes[frameTimeIndex] = frameTime;
  frameTimeIndex = (frameTimeIndex + 1) % MAX_FRAME_TIMES;
}

static void add_rectangle_with_triangles(struct renderer *renderer, float x,
                                         float y, float width, float height,
                                         int screen_width, int screen_height,
                                         float *color) {
  float x1 = x;
  float y1 = y;
  float x2 = x + width;
  float y2 = y;
  float x3 = x + width;
  float y3 = y + height;
  float x4 = x;
  float y4 = y + height;

  odc_renderer_add_triangle(renderer, x1, y1, x2, y2, x3, y3, screen_width,
                            screen_height, color);
  odc_renderer_add_triangle(renderer, x1, y1, x3, y3, x4, y4, screen_width,
                            screen_height, color);
}

void odc_debug_render_frame_time_graph(struct renderer *renderer,
                                       int screen_width, int screen_height) {
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
