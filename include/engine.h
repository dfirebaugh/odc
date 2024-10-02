#ifndef ENGINE_H
#define ENGINE_H

struct engine;
struct renderer;

typedef void (*update_callback_t)(struct engine *e, struct renderer *renderer,
                                  double delta_time);
typedef void (*render_callback_t)(struct engine *e, struct renderer *renderer);

struct engine *engine_new(int width, int height);
void engine_run(struct engine *e);

void engine_update(struct engine *e, double delta_time);
void engine_render(struct engine *e);

void engine_set_update_callback(struct engine *e, update_callback_t callback);
void engine_set_render_callback(struct engine *e, render_callback_t callback);
void engine_destroy(struct engine *e);
struct GLFWwindow *engine_get_window(struct engine *e);
struct renderer *engine_get_renderer(struct engine *e);
int engine_get_fps(struct engine *e);

#endif // ENGINE_H
