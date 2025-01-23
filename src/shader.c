#include "glad.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "odc_shader.h"

GLuint odc_shader_compile_shader(const char *source, GLenum shaderType,
				 char *error)
{
	GLuint shader = glCreateShader(shaderType);

	glShaderSource(shader, 1, &source, NULL);
	glCompileShader(shader);

	GLint status;
	glGetShaderiv(shader, GL_COMPILE_STATUS, &status);
	if (status == GL_FALSE) {
		GLint logLength;
		glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &logLength);

		char *log = (char *)malloc(logLength + 1);
		glGetShaderInfoLog(shader, logLength, NULL, log);

		snprintf(error, logLength + 1, "failed to compile shader: %s",
			 log);
		free(log);

		glDeleteShader(shader);
		return 0;
	}

	return shader;
}

GLuint odc_shader_new_program(const char *vertexShaderSource,
			      const char *fragmentShaderSource, char *error)
{
	GLuint vertexShader = odc_shader_compile_shader(
		vertexShaderSource, GL_VERTEX_SHADER, error);
	if (vertexShader == 0) {
		return 0;
	}

	GLuint fragmentShader = odc_shader_compile_shader(
		fragmentShaderSource, GL_FRAGMENT_SHADER, error);
	if (fragmentShader == 0) {
		return 0;
	}

	GLuint program = glCreateProgram();
	glAttachShader(program, vertexShader);
	glAttachShader(program, fragmentShader);
	glLinkProgram(program);

	GLint status;
	glGetProgramiv(program, GL_LINK_STATUS, &status);
	if (status == GL_FALSE) {
		GLint logLength;
		glGetProgramiv(program, GL_INFO_LOG_LENGTH, &logLength);

		char *log = (char *)malloc(logLength + 1);
		glGetProgramInfoLog(program, logLength, NULL, log);

		snprintf(error, logLength + 1, "failed to link program: %s",
			 log);
		free(log);

		glDeleteShader(vertexShader);
		glDeleteShader(fragmentShader);
		glDeleteProgram(program);
		return 0;
	}

	glDeleteShader(vertexShader);
	glDeleteShader(fragmentShader);

	return program;
}
