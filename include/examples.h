#ifndef EXAMPLES_H
#define EXAMPLES_H

struct engine;
struct renderer;

void example_update(struct engine *e, struct renderer *renderer,
                    double delta_time);
void stress_test_example_update(struct engine *e, struct renderer *renderer,
                                double delta_time);
void text_fill_example_update(struct engine *e, struct renderer *renderer,
                              double delta_time);

void buddymark_example_update(struct engine *e, struct renderer *renderer,
                              double delta_time);

#endif // EXAMPLES_H
