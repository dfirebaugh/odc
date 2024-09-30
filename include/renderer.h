#ifndef RENDERER_H
#define RENDERER_H

#include "glad.h"

#define ATTRIB_POS_LOCATION 0
#define ATTRIB_SHAPE_POS_LOCATION 1
#define ATTRIB_LOCAL_POS_LOCATION 2
#define ATTRIB_OPCODE_LOCATION 3
#define ATTRIB_RADIUS_LOCATION 4
#define ATTRIB_COLOR_LOCATION 5
#define ATTRIB_WIDTH_LOCATION 6
#define ATTRIB_HEIGHT_LOCATION 7
#define ATTRIB_TEX_COORD_LOCATION 8
#define ATTRIB_RESOLUTION_LOCATION 9

#define OP_CODE_CIRCLE 1.0f
#define OP_CODE_ROUNDED_RECT 2.0f
#define OP_CODE_TRIANGLE 3.0f
#define OP_CODE_TEXT 4.0f

#define MAX_SHAPES 20000
struct renderer;

struct renderer *renderer_new();
void renderer_init(struct renderer *renderer, const char *font_path);
void renderer_destroy(struct renderer *renderer);
GLuint renderer_get_shader(struct renderer *renderer);
void renderer_reset_shape_count(struct renderer *renderer);
void renderer_add_circle(struct renderer *renderer, float x, float y,
                         float radius, int screen_width, int screen_height,
                         float *color);
void renderer_add_rounded_rect(struct renderer *renderer, float x, float y,
                               float width, float height, float radius,
                               int screen_width, int screen_height,
                               float *color);
void renderer_add_triangle(struct renderer *renderer, float x, float y,
                           float size, int screen_width, int screen_height,
                           float *color);
void renderer_draw(struct renderer *renderer);
void renderer_add_text(struct renderer *renderer, const char *text, float x,
                       float y, float scale, int screen_width,
                       int screen_height, float *color);

void check_gl_errors();

#endif // RENDERER_H
