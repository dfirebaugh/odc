
#ifndef ENGINE_H
#define ENGINE_H

#include "renderer.h"
#include <GLFW/glfw3.h>

typedef struct engine engine;

typedef void (*update_callback_t)(engine *e, shape_renderer *renderer,
                                  double delta_time);

struct engine {
  GLFWwindow *window;
  int window_width;
  int window_height;
  update_callback_t update_callback;
  shape_renderer renderer;
};

struct engine *engine_new(int width, int height);
void engine_run(engine *e);
void engine_set_update_callback(engine *e, update_callback_t callback);
void engine_destroy(engine *e);

#endif // ENGINE_H
