# =============================
# 1. Thông tin dự án
# =============================
TARGET = restaurant

SRC_DIR = src
INC_DIR = include
APP_DIR = app
TEST_DIR = tests

BUILD_DIR = build
OBJ_DIR   = $(BUILD_DIR)/obj
BIN_DIR   = $(BUILD_DIR)/bin

# =============================
# 2. Cờ biên dịch
# =============================
CC = gcc
INCLUDE_PATHS = -I$(INC_DIR) -I$(SRC_DIR) -I$(APP_DIR)
CFLAGS = -Wall $(INCLUDE_PATHS)
TEST_CFLAGS = $(CFLAGS) -DTESTING
LDFLAGS = -pthread

# =============================
# 3. Nguồn cho APP
# =============================
SRC_FILES = $(wildcard $(SRC_DIR)/*.c)
APP_SRC   = $(wildcard $(APP_DIR)/*.c)

OBJ_FILES = \
  $(patsubst $(SRC_DIR)/%.c,$(OBJ_DIR)/src/%.o,$(SRC_FILES)) \
  $(patsubst $(APP_DIR)/%.c,$(OBJ_DIR)/app/%.o,$(APP_SRC))

# =============================
# 4. Nguồn cho TEST
# =============================
TEST_SRC  = $(wildcard $(TEST_DIR)/*.c)
TEST_BINS = $(patsubst $(TEST_DIR)/%.c,$(BIN_DIR)/%,$(TEST_SRC))

# =============================
# 5. Targets
# =============================
.PHONY: all tests clean run run_tests run_test_%

all: $(OBJ_DIR) $(BIN_DIR) $(BIN_DIR)/$(TARGET)

tests: $(TEST_BINS)

# Build app
$(BIN_DIR)/$(TARGET): $(OBJ_FILES)
	@mkdir -p $(dir $@)
	$(CC) $(LDFLAGS) $^ -o $@

# Build mỗi test thành binary riêng
$(BIN_DIR)/%: $(OBJ_DIR)/tests/%.o $(patsubst $(SRC_DIR)/%.c,$(OBJ_DIR)/src/%.o,$(SRC_FILES))
	@mkdir -p $(dir $@)
	$(CC) $(LDFLAGS) $(TEST_CFLAGS) $^ -o $@

# Build 1 test cụ thể bằng: make test_shared
test_%: $(BIN_DIR)/test_%
	@echo "===> Test binary $< đã build xong."

# Run toàn bộ test
run_tests: tests
	@echo "===> Chạy toàn bộ test..."
	@for t in $(TEST_BINS); do \
		echo "===> Running $$t"; \
		$$t || exit 1; \
	done
	@echo "===> Tất cả test passed!"

# Run một test riêng: make run_test_shared
run_test_%: $(BIN_DIR)/test_%
	@echo "===> Running $<"
	$<

# =============================
# 6. Compile rules
# =============================
$(OBJ_DIR)/src/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJ_DIR)/app/%.o: $(APP_DIR)/%.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJ_DIR)/tests/%.o: $(TEST_DIR)/%.c
	@mkdir -p $(dir $@)
	$(CC) $(TEST_CFLAGS) -c $< -o $@

# =============================
# 7. Utils
# =============================
$(OBJ_DIR) $(BIN_DIR):
	mkdir -p $@

clean:
	rm -rf $(BUILD_DIR)

run: all
	./$(BIN_DIR)/$(TARGET)
# =============================
# 8. Help (Usage Guide)
# =============================
.PHONY: help
help:
	@echo ""
	@echo "================= Hướng dẫn sử dụng Makefile ================="
	@echo "make all              : Biên dịch toàn bộ ứng dụng "
	@echo "make run              : Biên dịch và chạy ứng dụng chính"
	@echo "make tests            : Biên dịch toàn bộ test trong thư mục tests/"
	@echo "make test_<name>      : Biên dịch một test cụ thể (vd: make test_shared)"
	@echo "make run_test_<name>  : Biên dịch và chạy một test cụ thể"
	@echo "                        (vd: make run_test_shared)"
	@echo "make clean            : Xóa toàn bộ file build (obj + bin)"
	@echo "make help             : Hiển thị hướng dẫn này"
	@echo "=============================================================="
	@echo ""
