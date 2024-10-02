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
  render_callback_t render_callback;
  struct renderer *renderer;
  int fps;
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

  glfwWindowHint(GLFW_RESIZABLE, 1);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

  e->window =
      glfwCreateWindow(e->window_width, e->window_height, "odc", NULL, NULL);
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

  e->renderer = renderer_new();
  if (!e->renderer) {
    fprintf(stderr, "Failed to allocate memory for renderer\n");
    glfwDestroyWindow(e->window);
    glfwTerminate();
    free(e);
    return NULL;
  }
  renderer_init(
      e->renderer,
      "./assets/fonts/ComicShannsMono/ComicShannsMonoNerdFont-Bold.otf");

  e->fps = 0;

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

struct GLFWwindow *engine_get_window(struct engine *e) { return e->window; }

struct renderer *engine_get_renderer(struct engine *e) { return e->renderer; }

void process_input(struct engine *e) { input_update(e); }

void engine_run(struct engine *e) {
  if (!e->renderer) {
    fprintf(stderr, "Renderer is not initialized\n");
    return;
  }

  input_init(e->window);

  double lastTime = glfwGetTime();
  double lastTitleUpdateTime = lastTime;
  double lastLogTime = lastTime;
  int frameCount = 0;
  double totalFrameTime = 0.0;

  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  while (!glfwWindowShouldClose(e->window)) {
    double currentTime = glfwGetTime();
    double deltaTime = currentTime - lastTime;
    lastTime = currentTime;

    if (e->update_callback) {
      e->update_callback(e, e->renderer, deltaTime);
    }

    if (e->render_callback) {
      e->render_callback(e, e->renderer);
    }

    frameCount++;

    if (currentTime - lastTitleUpdateTime >= 1.0) {
      e->fps = frameCount;
      frameCount = 0;
      lastTitleUpdateTime = currentTime;
    }

    process_input(e);
    glfwSwapBuffers(e->window);
    glfwPollEvents();
  }
}

void engine_set_update_callback(struct engine *e, update_callback_t callback) {
  e->update_callback = callback;
}

void engine_set_render_callback(struct engine *e, render_callback_t callback) {
  e->render_callback = callback;
}

int engine_get_fps(struct engine *e) { return e->fps; }
