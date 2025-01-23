#ifndef INPUT_H
#define INPUT_H

#include "glad.h"
#include <GLFW/glfw3.h>

#include "odc.h"

struct engine;

extern double lastX;
extern double lastY;
extern int firstMouse;
extern int mouseCaptured;

struct mouse_position {
	double x;
	double y;
};

ODC_API struct mouse_position odc_input_get_mouse_position(GLFWwindow *window);
ODC_API void odc_input_init(GLFWwindow *window);
ODC_API void odc_input_update(struct engine *e);
ODC_API void odc_handle_mouse_click(GLFWwindow *window, int button, int action,
				    int mods);
ODC_API void odc_handle_mouse_movement(GLFWwindow *window, double xpos,
				       double ypos);
ODC_API void odc_handle_close(GLFWwindow *window);
ODC_API void odc_update_key_states(GLFWwindow *window);
ODC_API int odc_is_button_just_pressed(GLFWwindow *window, int key);
ODC_API void odc_handle_mouse_movement_callback(GLFWwindow *window, double xpos,
						double ypos);
ODC_API int odc_is_mouse_button_just_pressed(GLFWwindow *window, int button);

#endif // INPUT_H
