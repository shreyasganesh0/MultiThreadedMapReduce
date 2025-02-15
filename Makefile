CC = gcc
CFLAGS = -Wall -Wextra -Iinclude
DEBUG_FLAGS = -g -DDEBUG
SRC_DIR = src
BUILD_DIR = build
TARGET_COMBINE = $(BUILD_DIR)/combiner

# Default target
all: $(TARGET_COMBINE)

# Rule for building combiner binary directly
$(TARGET_COMBINE): $(SRC_DIR)/main.c
	@mkdir -p $(BUILD_DIR)
	$(CC) $(CFLAGS) $< -o $@

# Debug mode target
debug: CFLAGS += $(DEBUG_FLAGS)
debug: clean all

# Clean up build files
clean:
	rm -rf $(BUILD_DIR)

# Phony targets
.PHONY: all clean debug

