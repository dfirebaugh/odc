#ifndef DEBUG_H
#define DEBUG_H

struct renderer;

float debug_calculate_average_frame_time();
void debug_update_frame_times(float frameTime);
void debug_render_frame_time_graph(struct renderer *renderer, int screen_width,
                                   int screen_height);

#endif // DEBUG_H
