#ifndef SHADER_H
#define SHADER_H

#include "glad.h"

GLuint new_program(const char *vertexShaderSource,
                   const char *fragmentShaderSource, char *error);
GLuint compile_shader(const char *source, GLenum shaderType, char *error);

#endif // SHADER_H
