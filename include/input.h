#ifndef INPUT_H
#define INPUT_H

#include "glad.h"
#include <GLFW/glfw3.h>

#include "engine.h"

extern double lastX;
extern double lastY;
extern int firstMouse;
extern int mouseCaptured;

struct MousePosition {
  double x;
  double y;
};

struct MousePosition input_get_mouse_position(GLFWwindow *window);
void input_init(GLFWwindow *window);
void input_update(struct engine *e);
void handle_mouse_click(GLFWwindow *window, int button, int action, int mods);
void handle_mouse_movement(GLFWwindow *window, double xpos, double ypos);
void handle_close(GLFWwindow *window);
void update_key_states(GLFWwindow *window);
int is_button_just_pressed(GLFWwindow *window, int key);
void handle_mouse_movement_callback(GLFWwindow *window, double xpos,
                                    double ypos);
int is_mouse_button_just_pressed(GLFWwindow *window, int button);

#endif // INPUT_H
