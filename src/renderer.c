#include "glad.h"
#include <GLFW/glfw3.h>

#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#include "font.h"
#include "renderer.h"
#include "shader.h"
#include "string.h"

struct vertex {
  float fs_quad_pos[2];
  float shape_pos[2];
  float local_pos[2];
  float op_code;
  float radius;
  float width;
  float height;
  float color[4];
  float resolution[2];
  float tex_coord[2];
};

struct renderer {
  struct vertex vertices[MAX_SHAPES * 6];
  unsigned int VAO, VBO;
  int shape_count;
  GLuint shader_program;
  Font font;
  GLuint atlas_id;
};

void check_gl_errors() {
  GLenum err;
  while ((err = glGetError()) != GL_NO_ERROR) {
    fprintf(stderr, "OpenGL error: %d\n", err);
  }
}

const float quadVertices[] = {
    -1.0f, 1.0f,  // Top-left
    -1.0f, -1.0f, // Bottom-left
    1.0f,  -1.0f, // Bottom-right
    -1.0f, 1.0f,  // Top-left
    1.0f,  -1.0f, // Bottom-right
    1.0f,  1.0f   // Top-right
};

const char *vertexShaderSource =
    "#version 330 core\n"
    "layout(location = 0) in vec2 in_pos;\n"
    "layout(location = 1) in vec2 in_shape_pos;\n"
    "layout(location = 2) in vec2 in_local_pos;\n"
    "layout(location = 3) in float in_op_code;\n"
    "layout(location = 4) in float in_radius;\n"
    "layout(location = 5) in vec4 in_color;\n"
    "layout(location = 6) in float in_width;\n"
    "layout(location = 7) in float in_height;\n"
    "layout(location = 8) in vec2 in_tex_coord;\n"
    "layout(location = 9) in vec2 in_resolution;\n"

    "out vec2 local_pos;\n"
    "out float op_code;\n"
    "out float radius;\n"
    "out vec4 color;\n"
    "out float width;\n"
    "out float height;\n"
    "out vec2 tex_coord;\n"

    "void main() {\n"
    "    vec2 scaledShapePos = in_shape_pos;\n"
    "    if (in_op_code == 4.0 || in_op_code == 5.0) {\n"
    "        gl_Position = vec4(in_pos, 0.0, 1.0);\n"
    "    } else {\n"
    "        gl_Position = vec4(scaledShapePos + in_local_pos / in_resolution "
    "* 2.0, 0.0, 1.0);\n"
    "    }\n"
    "    local_pos = in_local_pos;\n"
    "    op_code = in_op_code;\n"
    "    radius = in_radius;\n"
    "    color = in_color;\n"
    "    width = in_width;\n"
    "    height = in_height;\n"
    "    tex_coord = in_tex_coord;\n"
    "}\n";

const char *fragmentShaderSource =
    "#version 330 core\n"
    "in vec2 local_pos;\n"
    "in float op_code;\n"
    "in float radius;\n"
    "in vec4 color;\n"
    "in float width;\n"
    "in float height;\n"
    "in vec2 tex_coord;\n"

    "out vec4 fragColor;\n"

    "uniform sampler2D text;\n"
    "uniform sampler2D texture_sampler;\n"

    "const float OP_CODE_CIRCLE = 1.0;\n"
    "const float OP_CODE_ROUNDED_RECT = 2.0;\n"
    "const float OP_CODE_TRIANGLE = 3.0;\n"
    "const float OP_CODE_REGULAR_TRIANGLE = 4.0;\n"
    "const float OP_CODE_TEXT = 5.0;\n"
    "const float OP_CODE_TEXTURE = 6.0;\n"

    "float sdCircle(vec2 p, float r) {\n"
    "    return length(p) - r;\n"
    "}\n"

    "float sdRoundedRect(vec2 p, vec2 bounds, float r) {\n"
    "    vec2 b = bounds - vec2(r);\n"
    "    vec2 q = abs(p) - b;\n"
    "    return length(max(q, 0.0)) - r;\n"
    "}\n"

    "float sdEquilateralTriangle(vec2 p) {\n"
    "    float k = sqrt(3.0);\n"
    "    p.x = abs(p.x) - 0.5;\n"
    "    p.y = p.y + 0.5 / k;\n"
    "    if (p.x + k * p.y > 0.0) p = vec2(p.x - k * p.y, -k * p.x - p.y) / "
    "2.0;\n"
    "    p.x -= clamp(p.x, -1.0, 0.0);\n"
    "    return -length(p) * sign(p.y);\n"
    "}\n"

    "void main() {\n"
    "    vec2 p = local_pos;\n"
    "    fragColor = vec4(color.rgb, 0.0);\n"

    "    if (op_code == OP_CODE_CIRCLE) {\n"
    "        float sdf = sdCircle(p, radius);\n"
    "        if (sdf < 0.0) {\n"
    "            fragColor = color;\n"
    "        }\n"
    "    } else if (op_code == OP_CODE_ROUNDED_RECT) {\n"
    "        float sdf = sdRoundedRect(p, vec2(width, height) * 0.5, radius);\n"
    "        if (sdf < 0.0) {\n"
    "            fragColor = color;\n"
    "        }\n"
    "    } else if (op_code == OP_CODE_TRIANGLE) {\n"
    "        float sdf = sdEquilateralTriangle(p / max(width, height));\n"
    "        if (sdf < 0.0) {\n"
    "            fragColor = color;\n"
    "        }\n"
    "    } else if (op_code == OP_CODE_REGULAR_TRIANGLE) {\n"
    "        fragColor = color;\n"
    "    } else if (op_code == OP_CODE_TEXT) {\n"
    "        float sampled = texture(text, tex_coord).r;\n"
    "        fragColor = vec4(color.rgb, sampled);\n"
    "    } else if (op_code == OP_CODE_TEXTURE) {\n"
    "        fragColor = texture(texture_sampler, tex_coord);\n"
    "    }\n"
    "}\n";

struct renderer *renderer_new() {
  return (struct renderer *)malloc(sizeof(struct renderer));
}

void renderer_init(struct renderer *renderer, const char *font_path) {
  if (!renderer) {
    fprintf(stderr, "Renderer pointer is null\n");
    return;
  }

  char error[256] = {0};
  GLuint shader_program =
      new_program(vertexShaderSource, fragmentShaderSource, error);
  if (!shader_program) {
    fprintf(stderr, "Shader compilation or linking error: %s\n", error);
    return;
  }

  renderer->shape_count = 0;
  renderer->shader_program = shader_program;

  glGenVertexArrays(1, &(renderer->VAO));
  glGenBuffers(1, &(renderer->VBO));

  glBindVertexArray(renderer->VAO);
  glBindBuffer(GL_ARRAY_BUFFER, renderer->VBO);

  glBufferData(GL_ARRAY_BUFFER, sizeof(struct vertex) * MAX_SHAPES * 6, NULL,
               GL_DYNAMIC_DRAW);

  glVertexAttribPointer(ATTRIB_POS_LOCATION, 2, GL_FLOAT, GL_FALSE,
                        sizeof(struct vertex),
                        (void *)offsetof(struct vertex, fs_quad_pos));
  glEnableVertexAttribArray(ATTRIB_POS_LOCATION);

  glVertexAttribPointer(ATTRIB_SHAPE_POS_LOCATION, 2, GL_FLOAT, GL_FALSE,
                        sizeof(struct vertex),
                        (void *)offsetof(struct vertex, shape_pos));
  glEnableVertexAttribArray(ATTRIB_SHAPE_POS_LOCATION);

  glVertexAttribPointer(ATTRIB_LOCAL_POS_LOCATION, 2, GL_FLOAT, GL_FALSE,
                        sizeof(struct vertex),
                        (void *)offsetof(struct vertex, local_pos));
  glEnableVertexAttribArray(ATTRIB_LOCAL_POS_LOCATION);

  glVertexAttribPointer(ATTRIB_OPCODE_LOCATION, 1, GL_FLOAT, GL_FALSE,
                        sizeof(struct vertex),
                        (void *)offsetof(struct vertex, op_code));
  glEnableVertexAttribArray(ATTRIB_OPCODE_LOCATION);

  glVertexAttribPointer(ATTRIB_RADIUS_LOCATION, 1, GL_FLOAT, GL_FALSE,
                        sizeof(struct vertex),
                        (void *)offsetof(struct vertex, radius));
  glEnableVertexAttribArray(ATTRIB_RADIUS_LOCATION);

  glVertexAttribPointer(ATTRIB_COLOR_LOCATION, 4, GL_FLOAT, GL_FALSE,
                        sizeof(struct vertex),
                        (void *)offsetof(struct vertex, color));
  glEnableVertexAttribArray(ATTRIB_COLOR_LOCATION);

  glVertexAttribPointer(ATTRIB_WIDTH_LOCATION, 1, GL_FLOAT, GL_FALSE,
                        sizeof(struct vertex),
                        (void *)offsetof(struct vertex, width));
  glEnableVertexAttribArray(ATTRIB_WIDTH_LOCATION);

  glVertexAttribPointer(ATTRIB_HEIGHT_LOCATION, 1, GL_FLOAT, GL_FALSE,
                        sizeof(struct vertex),
                        (void *)offsetof(struct vertex, height));
  glEnableVertexAttribArray(ATTRIB_HEIGHT_LOCATION);

  glVertexAttribPointer(ATTRIB_TEX_COORD_LOCATION, 2, GL_FLOAT, GL_FALSE,
                        sizeof(struct vertex),
                        (void *)offsetof(struct vertex, tex_coord));
  glEnableVertexAttribArray(ATTRIB_TEX_COORD_LOCATION);

  glVertexAttribPointer(ATTRIB_RESOLUTION_LOCATION, 2, GL_FLOAT, GL_FALSE,
                        sizeof(struct vertex),
                        (void *)offsetof(struct vertex, resolution));
  glEnableVertexAttribArray(ATTRIB_RESOLUTION_LOCATION);

  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glBindVertexArray(0);

  if (font_load(font_path, &renderer->font) != 0) {
    fprintf(stderr, "Failed to load font\n");
    return;
  }
}

GLuint renderer_get_shader(struct renderer *renderer) {
  return renderer->shader_program;
}

void renderer_reset_shape_count(struct renderer *renderer) {
  renderer->shape_count = 0;
}

void renderer_draw(struct renderer *renderer) {
  check_gl_errors();

  glUseProgram(renderer->shader_program);

  int screen_width, screen_height;
  glfwGetFramebufferSize(glfwGetCurrentContext(), &screen_width,
                         &screen_height);
  glUniform2f(glGetUniformLocation(renderer->shader_program, "u_resolution"),
              (float)screen_width, (float)screen_height);

  glBindVertexArray(renderer->VAO);
  check_gl_errors();

  glBindBuffer(GL_ARRAY_BUFFER, renderer->VBO);
  glBufferSubData(GL_ARRAY_BUFFER, 0,
                  sizeof(struct vertex) * renderer->shape_count * 6,
                  renderer->vertices);
  check_gl_errors();

  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, renderer->font.texture_id);
  glUniform1i(glGetUniformLocation(renderer->shader_program, "text"), 0);

  glActiveTexture(GL_TEXTURE1);
  glBindTexture(GL_TEXTURE_2D, renderer->atlas_id);
  glUniform1i(glGetUniformLocation(renderer->shader_program, "texture_sampler"),
              1);

  glDrawArrays(GL_TRIANGLES, 0, renderer->shape_count * 6);
  check_gl_errors();

  glBindVertexArray(0);
}

void renderer_clear_vertices(struct renderer *renderer) {
  memset(renderer->vertices, 0, sizeof(renderer->vertices));
}

void renderer_clear(struct renderer *renderer, float r, float g, float b,
                    float a) {
  glClearColor(r, g, b, a);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void normalize_coordinates(float x, float y, int screen_width,
                           int screen_height, float *norm_x, float *norm_y) {
  *norm_x = (x / (float)screen_width) * 2.0f - 1.0f;
  *norm_y = 1.0f - (y / (float)screen_height) * 2.0f;
}

void renderer_add_equilateral_triangle(struct renderer *renderer, float x,
                                       float y, float size, int screen_width,
                                       int screen_height, float *color) {
  if (renderer->shape_count >= MAX_SHAPES)
    return;

  float norm_x, norm_y;
  normalize_coordinates(x, y, screen_width, screen_height, &norm_x, &norm_y);

  float half_size = size / 2.0f;
  float height = size;

  float vertices[12] = {-half_size, -height / 2.0f, half_size,  -height / 2.0f,
                        0.0f,       height / 2.0f,  -half_size, -height / 2.0f,
                        half_size,  -height / 2.0f, 0.0f,       height / 2.0f};

  for (int i = 0; i < 6; ++i) {
    struct vertex *v = &renderer->vertices[renderer->shape_count * 6 + i];

    v->fs_quad_pos[0] = vertices[i * 2];
    v->fs_quad_pos[1] = vertices[i * 2 + 1];

    v->shape_pos[0] = norm_x;
    v->shape_pos[1] = norm_y;

    v->local_pos[0] = vertices[i * 2];
    v->local_pos[1] = vertices[i * 2 + 1];

    v->op_code = OP_CODE_EQUILATERAL_TRIANGLE;
    v->radius = 0.0f;
    v->width = size;
    v->height = size;

    for (int j = 0; j < 4; ++j)
      v->color[j] = color[j];

    v->resolution[0] = (float)screen_width;
    v->resolution[1] = (float)screen_height;
  }

  renderer->shape_count++;
}

void renderer_add_triangle(struct renderer *renderer, float x1, float y1,
                           float x2, float y2, float x3, float y3,
                           int screen_width, int screen_height, float *color) {
  if (renderer->shape_count >= MAX_SHAPES)
    return;

  float norm_x1, norm_y1, norm_x2, norm_y2, norm_x3, norm_y3;
  normalize_coordinates(x1, y1, screen_width, screen_height, &norm_x1,
                        &norm_y1);
  normalize_coordinates(x2, y2, screen_width, screen_height, &norm_x2,
                        &norm_y2);
  normalize_coordinates(x3, y3, screen_width, screen_height, &norm_x3,
                        &norm_y3);

  float vertices[6] = {norm_x1, norm_y1, norm_x2, norm_y2, norm_x3, norm_y3};

  for (int i = 0; i < 3; ++i) {
    struct vertex *v = &renderer->vertices[renderer->shape_count * 6 + i];

    v->fs_quad_pos[0] = vertices[i * 2];
    v->fs_quad_pos[1] = vertices[i * 2 + 1];

    v->shape_pos[0] = 0.0f;
    v->shape_pos[1] = 0.0f;

    v->local_pos[0] = vertices[i * 2];
    v->local_pos[1] = vertices[i * 2 + 1];

    v->op_code = OP_CODE_TRIANGLE;
    v->radius = 0.0f;
    v->width = 0.0f;
    v->height = 0.0f;

    for (int j = 0; j < 4; ++j)
      v->color[j] = color[j];

    v->resolution[0] = (float)screen_width;
    v->resolution[1] = (float)screen_height;
  }

  renderer->shape_count++;
}

void renderer_add_circle(struct renderer *renderer, float x, float y,
                         float radius, int screen_width, int screen_height,
                         float *color) {
  if (renderer->shape_count >= MAX_SHAPES)
    return;

  float norm_x, norm_y;
  normalize_coordinates(x, y, screen_width, screen_height, &norm_x, &norm_y);

  float vertices[12] = {-radius, radius, -radius, -radius, radius, -radius,
                        -radius, radius, radius,  -radius, radius, radius};

  for (int i = 0; i < 6; ++i) {
    struct vertex *v = &renderer->vertices[renderer->shape_count * 6 + i];

    v->fs_quad_pos[0] = vertices[i * 2];
    v->fs_quad_pos[1] = vertices[i * 2 + 1];

    v->shape_pos[0] = norm_x;
    v->shape_pos[1] = norm_y;

    v->local_pos[0] = vertices[i * 2];
    v->local_pos[1] = vertices[i * 2 + 1];

    v->op_code = OP_CODE_CIRCLE;
    v->radius = radius;
    v->width = radius * 2.0f;
    v->height = radius * 2.0f;

    for (int j = 0; j < 4; ++j)
      v->color[j] = color[j];

    v->resolution[0] = (float)screen_width;
    v->resolution[1] = (float)screen_height;
  }
  renderer->shape_count++;
}

void renderer_add_rounded_rect(struct renderer *renderer, float x, float y,
                               float width, float height, float radius,
                               int screen_width, int screen_height,
                               float *color) {
  if (renderer->shape_count >= MAX_SHAPES)
    return;

  float norm_x, norm_y;
  normalize_coordinates(x, y, screen_width, screen_height, &norm_x, &norm_y);

  float half_width = width * 0.5f;
  float half_height = height * 0.5f;

  float vertices[12] = {-half_width, half_height,  -half_width, -half_height,
                        half_width,  -half_height, -half_width, half_height,
                        half_width,  -half_height, half_width,  half_height};

  for (int i = 0; i < 6; ++i) {
    struct vertex *v = &renderer->vertices[renderer->shape_count * 6 + i];

    v->fs_quad_pos[0] = vertices[i * 2];
    v->fs_quad_pos[1] = vertices[i * 2 + 1];

    v->shape_pos[0] = norm_x + half_width / screen_width * 2.0f;
    v->shape_pos[1] = norm_y - half_height / screen_height * 2.0f;

    v->local_pos[0] = vertices[i * 2];
    v->local_pos[1] = vertices[i * 2 + 1];

    v->op_code = OP_CODE_ROUNDED_RECT;
    v->radius = radius;
    v->width = width;
    v->height = height;

    for (int j = 0; j < 4; ++j)
      v->color[j] = color[j];

    v->resolution[0] = (float)screen_width;
    v->resolution[1] = (float)screen_height;
  }
  renderer->shape_count++;
}

void renderer_add_text(struct renderer *renderer, const char *text, float x,
                       float y, float scale, int screen_width,
                       int screen_height, float *color) {
  if (renderer->shape_count >= MAX_SHAPES)
    return;

  for (const char *c = text; *c; c++) {
    stbtt_bakedchar *b = &renderer->font.cdata[*c - 32];

    float xpos = x + b->xoff * scale;
    float ypos = y + b->yoff * scale;

    float w = (b->x1 - b->x0) * scale;
    float h = (b->y1 - b->y0) * scale;

    float tex_x0 = b->x0 / 512.0f;
    float tex_y0 = b->y1 / 512.0f;
    float tex_x1 = b->x1 / 512.0f;
    float tex_y1 = b->y0 / 512.0f;

    float vertices[24] = {
        xpos,     ypos,     tex_x0, tex_y1, xpos,     ypos + h, tex_x0, tex_y0,
        xpos + w, ypos + h, tex_x1, tex_y0, xpos,     ypos,     tex_x0, tex_y1,
        xpos + w, ypos + h, tex_x1, tex_y0, xpos + w, ypos,     tex_x1, tex_y1};

    for (int i = 0; i < 6; ++i) {
      struct vertex *v = &renderer->vertices[renderer->shape_count * 6 + i];

      normalize_coordinates(vertices[i * 4], vertices[i * 4 + 1], screen_width,
                            screen_height, &v->fs_quad_pos[0],
                            &v->fs_quad_pos[1]);

      v->shape_pos[0] = 0.0f;
      v->shape_pos[1] = 0.0f;

      v->local_pos[0] = vertices[i * 4];
      v->local_pos[1] = vertices[i * 4 + 1];

      v->op_code = OP_CODE_TEXT;
      v->radius = 0.0f;
      v->width = w;
      v->height = h;

      for (int j = 0; j < 4; ++j)
        v->color[j] = color[j];

      v->resolution[0] = (float)screen_width;
      v->resolution[1] = (float)screen_height;

      v->tex_coord[0] = vertices[i * 4 + 2];
      v->tex_coord[1] = vertices[i * 4 + 3];
    }

    renderer->shape_count++;
    x += (b->xadvance) * scale;
  }
}

void renderer_add_texture(struct renderer *renderer,
                          struct TextureRenderOptions *options) {
  if (renderer->shape_count >= MAX_SHAPES)
    return;

  float norm_x, norm_y;
  normalize_coordinates(options->x, options->y, options->screen_width,
                        options->screen_height, &norm_x, &norm_y);

  float width = options->rect_width * options->scale;
  float height = options->rect_height * options->scale;

  float u0 = options->rect_x / options->width;
  float v0 = options->rect_y / options->height;
  float u1 = (options->rect_x + options->rect_width) / options->width;
  float v1 = (options->rect_y + options->rect_height) / options->height;

  if (options->flip_x) {
    float temp = u0;
    u0 = u1;
    u1 = temp;
  }

  if (options->flip_y) {
    float temp = v0;
    v0 = v1;
    v1 = temp;
  }

  float vertices[24] = {0.0f,  0.0f,    u0, v1, 0.0f,  -height, u0, v0,
                        width, -height, u1, v0, 0.0f,  0.0f,    u0, v1,
                        width, -height, u1, v0, width, 0.0f,    u1, v1};

  float cos_theta = cosf(options->rotation);
  float sin_theta = sinf(options->rotation);

  for (int i = 0; i < 6; ++i) {
    struct vertex *v = &renderer->vertices[renderer->shape_count * 6 + i];

    float local_x = vertices[i * 4];
    float local_y = vertices[i * 4 + 1];

    float rotated_x = local_x * cos_theta - local_y * sin_theta;
    float rotated_y = local_x * sin_theta + local_y * cos_theta;

    v->fs_quad_pos[0] = rotated_x;
    v->fs_quad_pos[1] = rotated_y;

    v->shape_pos[0] = norm_x;
    v->shape_pos[1] = norm_y;

    v->local_pos[0] = rotated_x;
    v->local_pos[1] = rotated_y;

    v->op_code = OP_CODE_TEXTURE;
    v->radius = 0.0f;
    v->width = options->width;
    v->height = options->height;

    for (int j = 0; j < 4; ++j)
      v->color[j] = 1.0f;

    v->resolution[0] = (float)options->screen_width;
    v->resolution[1] = (float)options->screen_height;

    v->tex_coord[0] = vertices[i * 4 + 2];
    v->tex_coord[1] = vertices[i * 4 + 3];
  }

  renderer->shape_count++;
}

void renderer_upload_texture_atlas(struct renderer *renderer,
                                   const unsigned char *data, int width,
                                   int height) {
  glGenTextures(1, &renderer->atlas_id);
  glBindTexture(GL_TEXTURE_2D, renderer->atlas_id);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA,
               GL_UNSIGNED_BYTE, data);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glBindTexture(GL_TEXTURE_2D, 0);
}
