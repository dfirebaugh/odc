CC = gcc
CFLAGS = -Iinclude -Iexternal/glad/include -Iexternal/stb -g $(shell pkg-config --cflags freetype2) -fPIC -fvisibility=hidden
LDFLAGS = -Lexternal/glfw/src -Wl,--whole-archive -lglfw -Wl,--no-whole-archive -lGL -ldl -lpthread -lportaudio -lfreetype -lX11 -lXrandr -lXi -lm

BUILD_DIR = build
LIB_DIR = $(BUILD_DIR)/lib
OBJ_DIR = $(BUILD_DIR)/obj
EXAMPLES_DIR = examples
BUILD_INCLUDE_DIR = $(BUILD_DIR)/include
RESOLVED_HEADER = $(BUILD_INCLUDE_DIR)/odc_resolved.h

CORE_SRC = external/glad/glad.c src/debug.c src/engine.c src/renderer.c src/shader.c src/input.c src/font.c src/oscillator.c src/audio.c src/note_parser.c
CORE_OBJ = $(CORE_SRC:%.c=$(OBJ_DIR)/%.o)

LIBRARY = $(LIB_DIR)/libengine.so

BUDDYMARK_SRC = $(EXAMPLES_DIR)/buddymark.c
EXAMPLE_SRC = $(EXAMPLES_DIR)/example.c
SOUND_EXAMPLE_SRC = $(EXAMPLES_DIR)/sound.c
GOL_EXAMPLE_SRC = $(EXAMPLES_DIR)/gol.c

BUDDYMARK_OBJ = $(BUDDYMARK_SRC:%.c=$(OBJ_DIR)/%.o)
EXAMPLE_OBJ = $(EXAMPLE_SRC:%.c=$(OBJ_DIR)/%.o)
SOUND_EXAMPLE_OBJ = $(SOUND_EXAMPLE_SRC:%.c=$(OBJ_DIR)/%.o)
GOL_EXAMPLE_OBJ = $(GOL_EXAMPLE_SRC:%.c=$(OBJ_DIR)/%.o)

BUDDYMARK_EXEC = $(BUILD_DIR)/buddymark
EXAMPLE_EXEC = $(BUILD_DIR)/example
SOUND_EXAMPLE_EXEC = $(BUILD_DIR)/sound
GOL_EXAMPLE_EXEC = $(BUILD_DIR)/gol

all: $(LIBRARY) headers buddymark example sound gol

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

$(BUDDYMARK_EXEC): $(BUDDYMARK_OBJ) $(LIBRARY)
	$(CC) -o $@ $^ -L$(LIB_DIR) -lengine $(LDFLAGS)

$(EXAMPLE_EXEC): $(EXAMPLE_OBJ) $(LIBRARY)
	$(CC) -o $@ $^ -L$(LIB_DIR) -lengine $(LDFLAGS)

$(SOUND_EXAMPLE_EXEC): $(SOUND_EXAMPLE_OBJ) $(LIBRARY)
	$(CC) -o $@ $^ -L$(LIB_DIR) -lengine $(LDFLAGS)

$(GOL_EXAMPLE_EXEC): $(GOL_EXAMPLE_OBJ) $(LIBRARY)
	$(CC) -o $@ $^ -L$(LIB_DIR) -lengine $(LDFLAGS)

buddymark: $(BUILD_DIR) $(OBJ_DIR) $(BUDDYMARK_EXEC)
example: $(BUILD_DIR) $(OBJ_DIR) $(EXAMPLE_EXEC)
sound: $(BUILD_DIR) $(OBJ_DIR) $(SOUND_EXAMPLE_EXEC)
gol: $(BUILD_DIR) $(OBJ_DIR) $(GOL_EXAMPLE_EXEC)

clean:
	rm -rf $(BUILD_DIR)

.PHONY: all clean buddymark example sound gol
