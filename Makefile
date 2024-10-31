CC = gcc
CFLAGS = -Iinclude -Iexternal/glad/include -Iexternal/stb -g
LDFLAGS = -Lexternal/glfw/src -lglfw -lGL -ldl -lpthread -lportaudio -lX11 -lXrandr -lXi -lm

COMMON_SRC = external/glad/glad.c src/engine.c src/renderer.c src/shader.c src/input.c src/font.c src/debug.c src/oscillator.c src/audio.c src/note_parser.c

BUDDYMARK_SRC = $(COMMON_SRC) examples/buddymark.c
EXAMPLE_SRC = $(COMMON_SRC) examples/example.c
SOUND_EXAMPLE_SRC= $(COMMON_SRC) examples/sound.c

BUILD_DIR = build
OBJ_DIR = $(BUILD_DIR)/obj
BUDDYMARK_OBJ = $(BUDDYMARK_SRC:%.c=$(OBJ_DIR)/%.o)
EXAMPLE_OBJ = $(EXAMPLE_SRC:%.c=$(OBJ_DIR)/%.o)
SOUND_EXAMPLE_OBJ = $(SOUND_EXAMPLE_SRC:%.c=$(OBJ_DIR)/%.o)
NOTE_PARSER_EXAMPLE_OBJ = $(NOTE_PARSER_EXAMPLE_SRC:%.c=$(OBJ_DIR)/%.o)
BUDDYMARK_EXEC = $(BUILD_DIR)/buddymark
EXAMPLE_EXEC = $(BUILD_DIR)/example
SOUND_EXAMPLE_EXEC = $(BUILD_DIR)/sound

all:  buddymark example sound

$(BUILD_DIR) $(OBJ_DIR):
	mkdir -p $@

$(BUDDYMARK_EXEC): $(BUDDYMARK_OBJ)
	$(CC) -o $@ $^ $(LDFLAGS)

$(EXAMPLE_EXEC): $(EXAMPLE_OBJ)
	$(CC) -o $@ $^ $(LDFLAGS)

$(SOUND_EXAMPLE_EXEC): $(SOUND_EXAMPLE_OBJ)
	$(CC) -o $@ $^ $(LDFLAGS)

$(NOTE_PARSER_EXAMPLE_EXEC): $(NOTE_PARSER_EXAMPLE_OBJ)
	$(CC) -o $@ $^ $(LDFLAGS)

$(OBJ_DIR)/%.o: %.c | $(BUILD_DIR) $(OBJ_DIR)
	mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -rf $(BUILD_DIR)

.PHONY: all clean buddymark example sound

buddymark: $(BUILD_DIR) $(OBJ_DIR) $(BUDDYMARK_EXEC)

example: $(BUILD_DIR) $(OBJ_DIR) $(EXAMPLE_EXEC)

sound: $(BUILD_DIR) $(OBJ_DIR) $(SOUND_EXAMPLE_EXEC)

