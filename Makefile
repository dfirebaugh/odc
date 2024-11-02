CC = gcc
CFLAGS = -Iinclude -g $(shell pkg-config --cflags freetype2) -fPIC -fvisibility=hidden
LDFLAGS = -Wl,--whole-archive -lglfw -Wl,--no-whole-archive -lGL -ldl -lpthread -lportaudio -lfreetype -lX11 -lXrandr -lXi -lm

BUILD_DIR = build
LIB_DIR = $(BUILD_DIR)/lib
OBJ_DIR = $(BUILD_DIR)/obj
EXAMPLES_DIR = examples
BUILD_INCLUDE_DIR = $(BUILD_DIR)/include
RESOLVED_HEADER = $(BUILD_INCLUDE_DIR)/odc_resolved.h

CORE_SRC = src/glad.c src/debug.c src/engine.c src/renderer.c src/shader.c src/input.c src/font.c src/oscillator.c src/audio.c src/note_parser.c
CORE_OBJ = $(CORE_SRC:%.c=$(OBJ_DIR)/%.o)

LIBRARY = $(LIB_DIR)/libodc.so

all: $(LIBRARY) headers 

headers: $(BUILD_INCLUDE_DIR)
	@echo "Copying headers to $(BUILD_INCLUDE_DIR)"
	@cp -r include/* $(BUILD_INCLUDE_DIR)/

$(BUILD_DIR) $(OBJ_DIR) $(LIB_DIR) $(BUILD_INCLUDE_DIR):
	mkdir -p $@

$(LIBRARY): $(CORE_OBJ) | $(BUILD_DIR) $(LIB_DIR)
	$(CC) -shared -o $@ $^ $(LDFLAGS)

$(OBJ_DIR)/%.o: %.c | $(BUILD_DIR) $(OBJ_DIR)
	mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -rf $(BUILD_DIR)

.PHONY: all clean
