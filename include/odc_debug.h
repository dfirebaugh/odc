#ifndef DEBUG_H
#define DEBUG_H

#include "odc.h"

struct renderer;

ODC_API float odc_debug_calculate_average_frame_time();
ODC_API void odc_debug_update_frame_times(float frameTime);
ODC_API void odc_debug_render_frame_time_graph(struct renderer *renderer,
                                               int screen_width,
                                               int screen_height);
#endif // DEBUG_H
