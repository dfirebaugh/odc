#ifndef ODC_ENGINE_H
#define ODC_ENGINE_H

#include "odc.h"

struct engine;
struct renderer;

typedef void (*update_callback_t)(struct engine *e);
typedef void (*render_callback_t)(struct engine *e);

ODC_API struct engine *odc_engine_new(int width, int height);
ODC_API void odc_engine_run(struct engine *e);

ODC_API void odc_engine_update(struct engine *e, double delta_time);
ODC_API void odc_engine_render(struct engine *e);

ODC_API void odc_engine_set_update_callback(struct engine *e,
                                            update_callback_t callback);
ODC_API void odc_engine_set_render_callback(struct engine *e,
                                            render_callback_t callback);
ODC_API void odc_engine_set_audio_data(struct engine *e, void *audio_data);
ODC_API void *odc_engine_get_audio_data(struct engine *e);

ODC_API void odc_engine_destroy(struct engine *e);
ODC_API struct GLFWwindow *odc_engine_get_window(struct engine *e);
ODC_API struct renderer *odc_engine_get_renderer(struct engine *e);
ODC_API int odc_engine_get_fps(struct engine *e);
ODC_API void odc_engine_set_window_title(struct engine *e, const char *title);

#endif // ODC_ENGINE_H
