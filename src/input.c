#include "glad.h"
#include <GLFW/glfw3.h>

#include "odc_engine.h"
#include "odc_input.h"

double lastX = 0.0;
double lastY = 0.0;
int firstMouse = 1;
int mouseCaptured = 0;
int keyStates[GLFW_KEY_LAST] = {0};
int mouseStates[GLFW_MOUSE_BUTTON_LAST] = {0};

struct mouse_position odc_input_get_mouse_position(GLFWwindow *window) {
  struct mouse_position pos;
  glfwGetCursorPos(window, &pos.x, &pos.y);
  return pos;
}

void odc_input_init(GLFWwindow *window) {
  glfwSetMouseButtonCallback(window, odc_handle_mouse_click);
  glfwSetCursorPosCallback(window, odc_handle_mouse_movement_callback);
}

void odc_input_update(struct engine *e) {
  struct GLFWwindow *window = odc_engine_get_window(e);
  if (mouseCaptured) {
    double xpos, ypos;
    glfwGetCursorPos(window, &xpos, &ypos);
    odc_handle_mouse_movement(window, xpos, ypos);
  }
  odc_handle_close(window);
  odc_update_key_states(window);
}

void odc_handle_mouse_click(GLFWwindow *window, int button, int action,
                            int mods) {
  double xpos, ypos;
  glfwGetCursorPos(window, &xpos, &ypos);

  if (action == GLFW_PRESS) {
    if (button >= 0 && button < GLFW_MOUSE_BUTTON_LAST) {
      mouseStates[button] = 1;
    }
    /*if (button == GLFW_MOUSE_BUTTON_LEFT) {*/
    /*  printf("Mouse clicked at position: (%f, %f)\n", xpos, ypos);*/
    /*}*/
  } else if (action == GLFW_RELEASE) {
    if (button >= 0 && button < GLFW_MOUSE_BUTTON_LAST) {
      mouseStates[button] = 0;
    }
  }
}

void odc_handle_mouse_movement_callback(GLFWwindow *window, double xpos,
                                        double ypos) {
  struct camera *cam = glfwGetWindowUserPointer(window);
  if (mouseCaptured) {
    odc_handle_mouse_movement(window, xpos, ypos);
  }
}

void odc_handle_mouse_movement(GLFWwindow *window, double xpos, double ypos) {
  if (firstMouse) {
    lastX = xpos;
    lastY = ypos;
    firstMouse = 0;
  }

  float xOffset = (float)(xpos - lastX);
  float yOffset = (float)(lastY - ypos);
  lastX = xpos;
  lastY = ypos;
}

void odc_handle_close(GLFWwindow *window) {
  if (odc_is_button_just_pressed(window, GLFW_KEY_ESCAPE)) {
    glfwSetWindowShouldClose(window, GLFW_TRUE);
  }
}

void odc_update_key_states(GLFWwindow *window) {
  for (int key = GLFW_KEY_SPACE; key <= GLFW_KEY_LAST; key++) {
    keyStates[key] = glfwGetKey(window, key);
  }
}

int odc_is_button_just_pressed(GLFWwindow *window, int key) {
  int currentState = glfwGetKey(window, key);
  if (currentState == GLFW_PRESS && keyStates[key] == GLFW_RELEASE) {
    return 1;
  }
  return 0;
}

int odc_is_mouse_button_just_pressed(GLFWwindow *window, int button) {
  if (button >= 0 && button < GLFW_MOUSE_BUTTON_LAST) {
    return mouseStates[button] &&
           glfwGetMouseButton(window, button) == GLFW_PRESS;
  }
  return 0;
}
