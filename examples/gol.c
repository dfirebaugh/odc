#include <stdlib.h>
#include <time.h>

#include "odc_engine.h"
#include "odc_renderer.h"

#define GRID_WIDTH 100
#define GRID_HEIGHT 100
#define CELL_SIZE 6

#define ALIVE_COLOR                                                            \
  { 0.0f, 1.0f, 0.0f, 1.0f }
#define DEAD_COLOR                                                             \
  { 0.1f, 0.1f, 0.1f, 1.0f }

typedef struct {
  int width;
  int height;
  int *cells;
} GameOfLife;

GameOfLife *game_of_life_new(int width, int height) {
  GameOfLife *game = malloc(sizeof(GameOfLife));
  game->width = width;
  game->height = height;
  game->cells = malloc(width * height * sizeof(int));

  srand(time(NULL));
  for (int i = 0; i < width * height; i++) {
    game->cells[i] = rand() % 2;
  }
  return game;
}

void game_of_life_destroy(GameOfLife *game) {
  free(game->cells);
  free(game);
}

int game_of_life_get_cell(GameOfLife *game, int x, int y) {
  if (x < 0 || x >= game->width || y < 0 || y >= game->height)
    return 0;
  return game->cells[y * game->width + x];
}

void game_of_life_update(GameOfLife *game) {
  int *new_cells = malloc(game->width * game->height * sizeof(int));

  for (int y = 0; y < game->height; y++) {
    for (int x = 0; x < game->width; x++) {
      int alive_neighbors = 0;
      for (int dy = -1; dy <= 1; dy++) {
        for (int dx = -1; dx <= 1; dx++) {
          if (dx == 0 && dy == 0)
            continue;
          alive_neighbors += game_of_life_get_cell(game, x + dx, y + dy);
        }
      }

      int cell_index = y * game->width + x;
      if (game->cells[cell_index] == 1) {
        new_cells[cell_index] = (alive_neighbors == 2 || alive_neighbors == 3);
      } else {
        new_cells[cell_index] = (alive_neighbors == 3);
      }
    }
  }

  free(game->cells);
  game->cells = new_cells;
}

void game_of_life_render(GameOfLife *game, struct renderer *renderer,
                         int screen_width, int screen_height) {
  float alive_color[] = ALIVE_COLOR;
  float dead_color[] = DEAD_COLOR;

  odc_renderer_clear(renderer, 0.0f, 0.0f, 0.0f, 1.0f);
  odc_renderer_reset_shape_count(renderer);

  for (int y = 0; y < game->height; y++) {
    for (int x = 0; x < game->width; x++) {
      float screen_x = x * CELL_SIZE;
      float screen_y = y * CELL_SIZE;
      float *color =
          game->cells[y * game->width + x] ? alive_color : dead_color;
      odc_renderer_add_rounded_rect(renderer, screen_x, screen_y, CELL_SIZE,
                                    CELL_SIZE, 1.0f, screen_width,
                                    screen_height, color);
    }
  }

  odc_renderer_draw(renderer);
}

void update_callback(struct engine *e, struct renderer *renderer,
                     double delta_time) {
  GameOfLife *game = (GameOfLife *)odc_engine_get_audio_data(e);
  game_of_life_update(game);
}

void render_callback(struct engine *e, struct renderer *renderer) {
  int window_width, window_height;
  glfwGetFramebufferSize(odc_engine_get_window(e), &window_width,
                         &window_height);

  GameOfLife *game = (GameOfLife *)odc_engine_get_audio_data(e);
  game_of_life_render(game, renderer, window_width, window_height);
}

int main() {
  struct engine *e =
      odc_engine_new(GRID_WIDTH * CELL_SIZE, GRID_HEIGHT * CELL_SIZE);
  if (!e) {
    return -1;
  }

  GameOfLife *game = game_of_life_new(GRID_WIDTH, GRID_HEIGHT);

  odc_engine_set_update_callback(e, update_callback);
  odc_engine_set_render_callback(e, render_callback);
  odc_engine_set_audio_data(e, game);

  odc_engine_run(e);

  game_of_life_destroy(game);
  odc_engine_destroy(e);
  return 0;
}
