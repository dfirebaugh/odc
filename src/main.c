#include "glad/glad.h"
#include <GLFW/glfw3.h>

#include <stdio.h>

#include "engine.h"
#include "examples.h"
#include "input.h"

void update(struct engine *e, shape_renderer *renderer, double delta_time) {
  if (is_mouse_button_just_pressed(e->window, GLFW_MOUSE_BUTTON_LEFT)) {
    printf("mouse left clicked\n");
  }

  example_update(e, renderer, delta_time);
}

int main() {
  struct engine *e = engine_new(600, 600);
  if (!e) {
    return -1;
  }

  engine_set_update_callback(e, update);
  engine_run(e);
  engine_destroy(e);

  return 0;
}
