
#ifndef ENGINE_H
#define ENGINE_H

#include "renderer.h"

struct engine;

typedef void (*update_callback_t)(struct engine *e, shape_renderer *renderer,
                                  double delta_time);

struct engine *engine_new(int width, int height);
void engine_run(struct engine *e);
void engine_set_update_callback(struct engine *e, update_callback_t callback);
void engine_destroy(struct engine *e);
struct GLFWwindow *engine_get_window(struct engine *e);

#endif // ENGINE_H
