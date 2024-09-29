#include "glad.h"
#include <GLFW/glfw3.h>

#include <stdio.h>
#include <stdlib.h>

#include "engine.h"
#include "input.h"
#include "renderer.h"

struct engine {
  GLFWwindow *window;
  int window_width;
  int window_height;
  update_callback_t update_callback;
  shape_renderer renderer;
};

void framebuffer_size_callback(GLFWwindow *window, int width, int height) {
  glViewport(0, 0, width, height);
}

static void set_default_window_size(struct engine *e, int width, int height) {
  if (e->window_width == 0) {
    e->window_width = width;
  }
  if (e->window_height == 0) {
    e->window_height = height;
  }
}

struct engine *engine_new(int width, int height) {
  struct engine *e = (struct engine *)calloc(1, sizeof(struct engine));
  if (!e) {
    fprintf(stderr, "Failed to allocate memory for engine\n");
    return NULL;
  }

  if (!glfwInit()) {
    fprintf(stderr, "Failed to initialize GLFW\n");
    free(e);
    return NULL;
  }

  set_default_window_size(e, width, height);
  printf("Using window size: %dx%d\n", e->window_width, e->window_height);

  glfwWindowHint(GLFW_RESIZABLE, 1);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

  e->window = glfwCreateWindow(e->window_width, e->window_height,
                               "Hello OpenGL", NULL, NULL);
  if (!e->window) {
    fprintf(stderr, "Failed to create GLFW window\n");
    glfwTerminate();
    free(e);
    return NULL;
  }

  glfwMakeContextCurrent(e->window);

  if (!gladLoaderLoadGL()) {
    fprintf(stderr, "Failed to initialize GLAD\n");
    glfwDestroyWindow(e->window);
    glfwTerminate();
    free(e);
    return NULL;
  }

  glViewport(0, 0, e->window_width, e->window_height);
  glfwSetFramebufferSizeCallback(e->window, framebuffer_size_callback);

  return e;
}

void engine_destroy(struct engine *e) {
  if (e) {
    if (e->window) {
      glfwDestroyWindow(e->window);
    }
    free(e);
  }
  glfwTerminate();
}

struct GLFWwindow *engine_get_window(struct engine *e) {
  return e->window;
}

void process_input(struct engine *e) { input_update(e); }

void engine_run(struct engine *e) {
  shape_renderer_init(&e->renderer);
  input_init(e->window);

  double lastTime = glfwGetTime();
  int frameCount = 0;

  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  while (!glfwWindowShouldClose(e->window)) {
    double currentTime = glfwGetTime();
    double deltaTime = currentTime - lastTime;

    if (e->update_callback) {
      e->update_callback(e, &e->renderer, deltaTime);
    }
    frameCount++;

    if (currentTime - lastTime >= 1.0) {
      char title[256];
      snprintf(title, sizeof(title), "Hello OpenGL - FPS: %d", frameCount);
      glfwSetWindowTitle(e->window, title);

      frameCount = 0;
      lastTime = currentTime;
    }

    process_input(e);
    int windowWidth, windowHeight;
    glfwGetFramebufferSize(e->window, &windowWidth, &windowHeight);
    float aspect_ratio = (float)windowWidth / (float)windowHeight;

    glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glUseProgram(e->renderer.shader_program);
    check_gl_errors();
    glUniform1f(
        glGetUniformLocation(e->renderer.shader_program, "aspect_ratio"),
        aspect_ratio);
    check_gl_errors();

    shape_renderer_draw(&e->renderer);
    check_gl_errors();

    glfwSwapBuffers(e->window);
    glfwPollEvents();
  }
}

void engine_set_update_callback(struct engine *e, update_callback_t callback) {
  e->update_callback = callback;
}
