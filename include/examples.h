#ifndef EXAMPLES_H
#define EXAMPLES_H

#include "engine.h"
#include "renderer.h"

void init_triangles();
void init_rounded_rects();
void triangle_example_update(struct engine *e, shape_renderer *renderer,
                             double delta_time);
void circle_example_update(struct engine *e, shape_renderer *renderer,
                           double delta_time);
void rounded_rect_example_update(struct engine *e, shape_renderer *renderer,
                                 double delta_time);

void example_update(struct engine *e, shape_renderer *renderer,
                    double delta_time);

#endif // EXAMPLES_H
