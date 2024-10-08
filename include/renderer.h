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
#define OP_CODE_EQUILATERAL_TRIANGLE 3.0f
#define OP_CODE_TRIANGLE 4.0f
#define OP_CODE_TEXT 5.0f
#define OP_CODE_TEXTURE 6.0f

#define MAX_SHAPES 400000
struct renderer;

struct TextureRenderOptions {
  float x;
  float y;
  float width;
  float height;
  float rect_x;
  float rect_y;
  float rect_width;
  float rect_height;
  int screen_width;
  int screen_height;
  int flip_x;
  int flip_y;
  float scale;
  float rotation;
};

struct renderer *renderer_new();
void renderer_init(struct renderer *renderer, const char *font_path);
void renderer_destroy(struct renderer *renderer);
void renderer_draw(struct renderer *renderer);
void renderer_clear(struct renderer *renderer, float r, float g, float b,
                    float a);
void renderer_clear_vertices(struct renderer *renderer);
void renderer_reset_shape_count(struct renderer *renderer);
GLuint renderer_get_shader(struct renderer *renderer);

void renderer_add_circle(struct renderer *renderer, float x, float y,
                         float radius, int screen_width, int screen_height,
                         float *color);
void renderer_add_rounded_rect(struct renderer *renderer, float x, float y,
                               float width, float height, float radius,
                               int screen_width, int screen_height,
                               float *color);
void renderer_add_equilateral_triangle(struct renderer *renderer, float x,
                                       float y, float size, int screen_width,
                                       int screen_height, float *color);
void renderer_add_triangle(struct renderer *renderer, float x1, float y1,
                           float x2, float y2, float x3, float y3,
                           int screen_width, int screen_height, float *color);
void renderer_add_text(struct renderer *renderer, const char *text, float x,
                       float y, float scale, int screen_width,
                       int screen_height, float *color);

void renderer_add_texture(struct renderer *renderer,
                          struct TextureRenderOptions *options);

void renderer_upload_texture_atlas(struct renderer *renderer,
                                   const unsigned char *data, int width,
                                   int height);
void check_gl_errors();

#endif // RENDERER_H
