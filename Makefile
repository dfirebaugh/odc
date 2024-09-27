
CC = gcc
CFLAGS = -Iinclude -Iexternal/cglm/include -Iexternal/glfw/include -Iexternal/glad/include -g
LDFLAGS = -Lexternal/glfw/src -lglfw -lGL -ldl -lpthread -lX11 -lXrandr -lXi -lm

SRC = src/glad.c src/main.c src/engine.c src/renderer.c src/shader.c src/input.c src/examples.c
BUILD_DIR = build
OBJ_DIR = $(BUILD_DIR)/obj
OBJ = $(SRC:src/%.c=$(OBJ_DIR)/%.o)
EXEC = $(BUILD_DIR)/demo

all: $(BUILD_DIR) $(OBJ_DIR) $(EXEC)

$(BUILD_DIR) $(OBJ_DIR):
	mkdir -p $@

$(EXEC): $(OBJ)
	$(CC) -o $@ $^ $(LDFLAGS)

$(OBJ_DIR)/%.o: src/%.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -rf $(BUILD_DIR)

.PHONY: all clean
