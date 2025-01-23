#include "glad.h"
#include <GLFW/glfw3.h>

#include <stdio.h>
#include <stdlib.h>

#include "odc_engine.h"
#include "odc_input.h"
#include "odc_renderer.h"

struct engine {
	GLFWwindow *window;
	int window_width;
	int window_height;
	update_callback_t update_callback;
	render_callback_t render_callback;
	void *audio_data;
	struct renderer *renderer;
	int fps;
	GLFWmonitor *monitor;
	const GLFWvidmode *original_vidmode;
};

static void framebuffer_size_callback(GLFWwindow *window, int width, int height)
{
	glViewport(0, 0, width, height);
}

static void set_default_window_size(struct engine *e, int width, int height)
{
	if (e->window_width == 0) {
		e->window_width = width;
	}
	if (e->window_height == 0) {
		e->window_height = height;
	}
}

struct engine *odc_engine_new(int width, int height, int fullscreen)
{
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

	if (fullscreen) {
		e->monitor = glfwGetPrimaryMonitor();
		if (e->monitor) {
			e->original_vidmode = glfwGetVideoMode(e->monitor);
			glfwWindowHint(GLFW_RED_BITS,
				       e->original_vidmode->redBits);
			glfwWindowHint(GLFW_GREEN_BITS,
				       e->original_vidmode->greenBits);
			glfwWindowHint(GLFW_BLUE_BITS,
				       e->original_vidmode->blueBits);
			glfwWindowHint(GLFW_REFRESH_RATE,
				       e->original_vidmode->refreshRate);
			e->window =
				glfwCreateWindow(e->original_vidmode->width,
						 e->original_vidmode->height,
						 "odc", e->monitor, NULL);
		}
	} else {
		e->window = glfwCreateWindow(e->window_width, e->window_height,
					     "odc", NULL, NULL);
	}

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

	e->renderer = odc_renderer_new();
	if (!e->renderer) {
		fprintf(stderr, "Failed to allocate memory for renderer\n");
		glfwDestroyWindow(e->window);
		glfwTerminate();
		free(e);
		return NULL;
	}
	odc_renderer_init(e->renderer);

	e->fps = 0;

	return e;
}

void odc_engine_destroy(struct engine *e)
{
	if (e) {
		if (e->renderer) {
			odc_renderer_destroy(e->renderer);
		}
		if (e->window) {
			glfwDestroyWindow(e->window);
		}
		if (e->monitor && e->original_vidmode) {
			glfwSetWindowMonitor(e->window, NULL, 0, 0,
					     e->original_vidmode->width,
					     e->original_vidmode->height,
					     e->original_vidmode->refreshRate);
		}
		free(e);
	}
	glfwTerminate();
}

struct GLFWwindow *odc_engine_get_window(struct engine *e)
{
	return e->window;
}

struct renderer *odc_engine_get_renderer(struct engine *e)
{
	return e->renderer;
}
void process_input(struct engine *e)
{
	odc_input_update(e);
}

void odc_engine_run(struct engine *e)
{
	odc_input_init(e->window);

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
			e->update_callback(e);
		}

		if (e->render_callback) {
			e->render_callback(e);
		}

		/*odc_renderer_clear(e->renderer, 0.1f);*/
		odc_renderer_draw(e->renderer);

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

void odc_engine_set_update_callback(struct engine *e,
				    update_callback_t callback)
{
	e->update_callback = callback;
}

void odc_engine_set_render_callback(struct engine *e,
				    render_callback_t callback)
{
	e->render_callback = callback;
}

void odc_engine_set_audio_data(struct engine *e, void *audio_data)
{
	e->audio_data = audio_data;
}

void *odc_engine_get_audio_data(struct engine *e)
{
	return e->audio_data;
}

int odc_engine_get_fps(struct engine *e)
{
	return e->fps;
}

void odc_engine_set_window_title(struct engine *e, const char *title)
{
	if (e && e->window) {
		glfwSetWindowTitle(e->window, title);
	}
}
