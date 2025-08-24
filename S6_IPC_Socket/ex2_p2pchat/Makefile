# Compiler và flags
CC      := gcc
CFLAGS  := -Wall -Wextra -g -Iinclude
LDFLAGS := -lpthread

# Thư mục
SRC_DIR := src
APP_DIR := app
BUILD_DIR := build

# File nguồn
SRC_FILES := $(wildcard $(SRC_DIR)/*.c)
APP_FILE  := $(APP_DIR)/main_p2pchat.c

# Biến object files
OBJ_FILES := $(patsubst $(SRC_DIR)/%.c,$(BUILD_DIR)/%.o,$(SRC_FILES))
APP_OBJ   := $(BUILD_DIR)/main_p2pchat.o

# Output
TARGET := $(BUILD_DIR)/p2pchat

# Rule mặc định
all: $(TARGET)

# Link final binary
$(TARGET): $(OBJ_FILES) $(APP_OBJ) | $(BUILD_DIR)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

# Build object từ src/
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

# Build object từ app/
$(BUILD_DIR)/%.o: $(APP_DIR)/%.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

# Tạo build dir nếu chưa có
$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

# Clean
clean:
	rm -rf $(BUILD_DIR)/*

# Run
run: all
	./$(TARGET)
