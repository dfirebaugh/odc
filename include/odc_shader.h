#ifndef SHADER_H
#define SHADER_H

#include "glad.h"
#include "odc.h"

ODC_API GLuint odc_shader_new_program(const char *vertexShaderSource,
				      const char *fragmentShaderSource,
				      char *error);
ODC_API GLuint odc_shader_compile_shader(const char *source, GLenum shaderType,
					 char *error);

#endif // SHADER_H
