CC = gcc
CFLAGS = -Iinclude -Iexternal/glad/include -Iexternal/stb -g
LDFLAGS = -Lexternal/glfw/src -lglfw -lGL -ldl -lpthread -lX11 -lXrandr -lXi -lm

BUDDYMARK_SRC = external/glad/glad.c examples/buddymark.c src/engine.c src/renderer.c src/shader.c src/input.c src/font.c src/debug.c
EXAMPLE_SRC = external/glad/glad.c examples/example.c src/engine.c src/renderer.c src/shader.c src/input.c src/font.c src/debug.c
BUILD_DIR = build
OBJ_DIR = $(BUILD_DIR)/obj
BUDDYMARK_OBJ = $(BUDDYMARK_SRC:%.c=$(OBJ_DIR)/%.o)
EXAMPLE_OBJ = $(EXAMPLE_SRC:%.c=$(OBJ_DIR)/%.o)
BUDDYMARK_EXEC = $(BUILD_DIR)/buddymark
EXAMPLE_EXEC = $(BUILD_DIR)/example

all:  buddymark example

$(BUILD_DIR) $(OBJ_DIR):
	mkdir -p $@

$(BUDDYMARK_EXEC): $(BUDDYMARK_OBJ)
	$(CC) -o $@ $^ $(LDFLAGS)

$(EXAMPLE_EXEC): $(EXAMPLE_OBJ)
	$(CC) -o $@ $^ $(LDFLAGS)

$(OBJ_DIR)/%.o: %.c | $(BUILD_DIR) $(OBJ_DIR)
	mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -rf $(BUILD_DIR)

.PHONY: all clean buddymark example

buddymark: $(BUILD_DIR) $(OBJ_DIR) $(BUDDYMARK_EXEC)

example: $(BUILD_DIR) $(OBJ_DIR) $(EXAMPLE_EXEC)


