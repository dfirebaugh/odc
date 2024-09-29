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

#define OP_CODE_CIRCLE 1.0f
#define OP_CODE_ROUNDED_RECT 2.0f
#define OP_CODE_TRIANGLE 3.0f

#define MAX_SHAPES 20000

typedef struct {
  float fs_quad_pos[2];
  float shape_pos[2];
  float local_pos[2];
  float op_code;
  float radius;
  float width;
  float height;
  float color[4];
} vertex;

typedef struct {
  vertex vertices[MAX_SHAPES * 6];
  unsigned int VAO, VBO;
  int shape_count;
  GLuint shader_program;
} shape_renderer;

void shape_renderer_init(shape_renderer *renderer);
void shape_renderer_add_circle(shape_renderer *renderer, float x, float y,
                               float radius, int screen_width,
                               int screen_height, float *color);
void shape_renderer_add_rounded_rect(shape_renderer *renderer, float x, float y,
                                     float width, float height, float radius,
                                     int screen_width, int screen_height,
                                     float *color);
void shape_renderer_add_triangle(shape_renderer *renderer, float x, float y,
                                 float size, int screen_width,
                                 int screen_height, float *color);
void shape_renderer_draw(shape_renderer *renderer);

void check_gl_errors();

#endif // RENDERER_H
