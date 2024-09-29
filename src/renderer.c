#include "glad.h"
#include <GLFW/glfw3.h>

#include <stdio.h>
#include <stdlib.h>

#include "renderer.h"
#include "shader.h"

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

    "uniform vec2 u_resolution;\n"

    "out vec2 local_pos;\n"
    "out float op_code;\n"
    "out float radius;\n"
    "out vec4 color;\n"
    "out float width;\n"
    "out float height;\n"

    "void main() {\n"
    "    vec2 scaledShapePos = (in_shape_pos / u_resolution) * 2.0 - 1.0;\n"
    "    scaledShapePos.y = -scaledShapePos.y;\n"
    "    gl_Position = vec4(scaledShapePos + (in_local_pos / u_resolution) * "
    "2.0, 0.0, 1.0);\n"
    "    local_pos = in_local_pos;\n"
    "    op_code = in_op_code;\n"
    "    radius = in_radius;\n"
    "    color = in_color;\n"
    "    width = in_width;\n"
    "    height = in_height;\n"
    "}\n";

const char *fragmentShaderSource =
    "#version 330 core\n"
    "in vec2 local_pos;\n"
    "in float op_code;\n"
    "in float radius;\n"
    "in vec4 color;\n"
    "in float width;\n"
    "in float height;\n"

    "out vec4 fragColor;\n"

    "const float OP_CODE_CIRCLE = 1.0;\n"
    "const float OP_CODE_ROUNDED_RECT = 2.0;\n"
    "const float OP_CODE_TRIANGLE = 3.0;\n"

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
    "        float sdf = sdEquilateralTriangle(p / max(width, height)); \n"
    "        if (sdf < 0.0) {\n"
    "            fragColor = color;\n"
    "        }\n"
    "    }\n"
    "}\n";

void shape_renderer_init(shape_renderer *renderer) {
  char error[256] = {0};
  GLuint shader_program =
      new_program(vertexShaderSource, fragmentShaderSource, error);
  if (!shader_program) {
    fprintf(stderr, "Shader compilation or linking error: %s\n", error);
    return;
  }

  fprintf(stdout, "Shader program successfully created with ID: %d\n",
          shader_program);

  renderer->shape_count = 0;
  renderer->shader_program = shader_program;

  glGenVertexArrays(1, &(renderer->VAO));
  glGenBuffers(1, &(renderer->VBO));

  glBindVertexArray(renderer->VAO);
  glBindBuffer(GL_ARRAY_BUFFER, renderer->VBO);

  glBufferData(GL_ARRAY_BUFFER, sizeof(vertex) * MAX_SHAPES * 6, NULL,
               GL_DYNAMIC_DRAW);

  glVertexAttribPointer(ATTRIB_POS_LOCATION, 2, GL_FLOAT, GL_FALSE,
                        sizeof(vertex), (void *)offsetof(vertex, fs_quad_pos));
  glEnableVertexAttribArray(ATTRIB_POS_LOCATION);

  glVertexAttribPointer(ATTRIB_SHAPE_POS_LOCATION, 2, GL_FLOAT, GL_FALSE,
                        sizeof(vertex), (void *)offsetof(vertex, shape_pos));
  glEnableVertexAttribArray(ATTRIB_SHAPE_POS_LOCATION);

  glVertexAttribPointer(ATTRIB_LOCAL_POS_LOCATION, 2, GL_FLOAT, GL_FALSE,
                        sizeof(vertex), (void *)offsetof(vertex, local_pos));
  glEnableVertexAttribArray(ATTRIB_LOCAL_POS_LOCATION);

  glVertexAttribPointer(ATTRIB_OPCODE_LOCATION, 1, GL_FLOAT, GL_FALSE,
                        sizeof(vertex), (void *)offsetof(vertex, op_code));
  glEnableVertexAttribArray(ATTRIB_OPCODE_LOCATION);

  glVertexAttribPointer(ATTRIB_RADIUS_LOCATION, 1, GL_FLOAT, GL_FALSE,
                        sizeof(vertex), (void *)offsetof(vertex, radius));
  glEnableVertexAttribArray(ATTRIB_RADIUS_LOCATION);

  glVertexAttribPointer(ATTRIB_COLOR_LOCATION, 4, GL_FLOAT, GL_FALSE,
                        sizeof(vertex), (void *)offsetof(vertex, color));
  glEnableVertexAttribArray(ATTRIB_COLOR_LOCATION);

  glVertexAttribPointer(ATTRIB_WIDTH_LOCATION, 1, GL_FLOAT, GL_FALSE,
                        sizeof(vertex), (void *)offsetof(vertex, width));
  glEnableVertexAttribArray(ATTRIB_WIDTH_LOCATION);

  glVertexAttribPointer(ATTRIB_HEIGHT_LOCATION, 1, GL_FLOAT, GL_FALSE,
                        sizeof(vertex), (void *)offsetof(vertex, height));
  glEnableVertexAttribArray(ATTRIB_HEIGHT_LOCATION);

  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glBindVertexArray(0);
}

void shape_renderer_draw(shape_renderer *renderer) {
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
                  sizeof(vertex) * renderer->shape_count * 6,
                  renderer->vertices);
  check_gl_errors();

  glDrawArrays(GL_TRIANGLES, 0, renderer->shape_count * 6);
  check_gl_errors();

  glBindVertexArray(0);
}

void normalize_coordinates(float x, float y, int screen_width,
                           int screen_height, float *norm_x, float *norm_y) {
  *norm_x = x;
  *norm_y = y;
}

void shape_renderer_add_triangle(shape_renderer *renderer, float x, float y,
                                 float size, int screen_width,
                                 int screen_height, float *color) {
  if (renderer->shape_count >= MAX_SHAPES)
    return;

  float norm_x, norm_y;
  normalize_coordinates(x, y, screen_width, screen_height, &norm_x, &norm_y);

  float half_size = size / 2.0f;
  float height = size;

  float vertices[12] = {-half_size,     -height / 2.0f, half_size,
                        -height / 2.0f, 0.0f,           height / 2.0f,

                        -half_size,     -height / 2.0f, half_size,
                        -height / 2.0f, 0.0f,           height / 2.0f};

  for (int i = 0; i < 6; ++i) {
    vertex *v = &renderer->vertices[renderer->shape_count * 6 + i];

    v->fs_quad_pos[0] = vertices[i * 2];
    v->fs_quad_pos[1] = vertices[i * 2 + 1];

    v->shape_pos[0] = norm_x;
    v->shape_pos[1] = norm_y;

    v->local_pos[0] = vertices[i * 2];
    v->local_pos[1] = vertices[i * 2 + 1];

    v->op_code = OP_CODE_TRIANGLE;
    v->radius = 0.0f;
    v->width = size;
    v->height = size;

    for (int j = 0; j < 4; ++j)
      v->color[j] = color[j];
  }

  renderer->shape_count++;
}

void shape_renderer_add_circle(shape_renderer *renderer, float x, float y,
                               float radius, int screen_width,
                               int screen_height, float *color) {
  if (renderer->shape_count >= MAX_SHAPES)
    return;

  float norm_x, norm_y;
  normalize_coordinates(x, y, screen_width, screen_height, &norm_x, &norm_y);

  float vertices[12] = {-radius, radius, -radius, -radius, radius, -radius,

                        -radius, radius, radius,  -radius, radius, radius};

  for (int i = 0; i < 6; ++i) {
    vertex *v = &renderer->vertices[renderer->shape_count * 6 + i];

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
  }
  renderer->shape_count++;
}

void shape_renderer_add_rounded_rect(shape_renderer *renderer, float x, float y,
                                     float width, float height, float radius,
                                     int screen_width, int screen_height,
                                     float *color) {
  if (renderer->shape_count >= MAX_SHAPES)
    return;

  float norm_x, norm_y;
  normalize_coordinates(x, y, screen_width, screen_height, &norm_x, &norm_y);

  float half_width = width * 0.5f;
  float half_height = height * 0.5f;

  float vertices[12] = {-half_width,  half_height, -half_width,
                        -half_height, half_width,  -half_height,

                        -half_width,  half_height, half_width,
                        -half_height, half_width,  half_height};

  for (int i = 0; i < 6; ++i) {
    vertex *v = &renderer->vertices[renderer->shape_count * 6 + i];

    v->fs_quad_pos[0] = vertices[i * 2];
    v->fs_quad_pos[1] = vertices[i * 2 + 1];

    v->shape_pos[0] = norm_x;
    v->shape_pos[1] = norm_y;

    v->local_pos[0] = vertices[i * 2];
    v->local_pos[1] = vertices[i * 2 + 1];

    v->op_code = OP_CODE_ROUNDED_RECT;
    v->radius = radius;
    v->width = width;
    v->height = height;

    for (int j = 0; j < 4; ++j)
      v->color[j] = color[j];
  }
  renderer->shape_count++;
}
