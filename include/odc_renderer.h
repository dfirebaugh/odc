#ifndef ODC_RENDERER_H
#define ODC_RENDERER_H

#include "glad.h"

#include "odc.h"

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

struct texture_render_options {
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

ODC_API struct renderer *odc_renderer_new();
ODC_API void odc_renderer_init(struct renderer *renderer);
ODC_API void odc_renderer_destroy(struct renderer *renderer);
ODC_API void odc_renderer_draw(struct renderer *renderer);
ODC_API void odc_renderer_clear(struct renderer *renderer, float r, float g,
                                float b, float a);
ODC_API void odc_renderer_clear_vertices(struct renderer *renderer);
ODC_API void odc_renderer_reset_shape_count(struct renderer *renderer);
ODC_API GLuint odc_renderer_get_shader(struct renderer *renderer);

ODC_API void odc_renderer_add_circle(struct renderer *renderer, float x,
                                     float y, float radius, int screen_width,
                                     int screen_height, float *color);
ODC_API void odc_renderer_add_rounded_rect(struct renderer *renderer, float x,
                                           float y, float width, float height,
                                           float radius, int screen_width,
                                           int screen_height, float *color);
ODC_API void odc_renderer_add_equilateral_triangle(struct renderer *renderer,
                                                   float x, float y, float size,
                                                   int screen_width,
                                                   int screen_height,
                                                   float *color);
ODC_API void odc_renderer_add_triangle(struct renderer *renderer, float x1,
                                       float y1, float x2, float y2, float x3,
                                       float y3, int screen_width,
                                       int screen_height, float *color);
ODC_API void odc_renderer_add_text(struct renderer *renderer, const char *text,
                                   float x, float y, float scale,
                                   int screen_width, int screen_height,
                                   float *color);

ODC_API void odc_renderer_add_texture(struct renderer *renderer,
                                      GLuint texture_handle,
                                      struct texture_render_options *options);
ODC_API void odc_renderer_add_multiline_text(struct renderer *renderer,
                                             const char *text, float x, float y,
                                             float scale, int screen_width,
                                             int screen_height, float *color);

ODC_API void odc_renderer_load_font(struct renderer *r, const char *font_path);
ODC_API GLuint odc_renderer_upload_texture(struct renderer *renderer,
                                           const unsigned char *data, int width,
                                           int height);
ODC_API void check_gl_errors();

#endif // ODC_RENDERER_H
