C_FLAGS ?= -Wall -Wextra -ggdb

BUILD_DIR ?= build
SRC_DIR := src
SRC := json.c
OBJ := $(patsubst %.c, $(BUILD_DIR)/%.o, $(SRC))
TARGET := $(BUILD_DIR)/libjsonc.so
DUMMY_TARGET := ./dummy/dummy

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c | $(BUILD_DIR)
	gcc -fPIC $(C_FLAGS) -c $< -o $@

$(TARGET): $(OBJ)
	gcc -shared -o $@ $(OBJ)

$(BUILD_DIR):
	mkdir -p $@

$(DUMMY_TARGET): $(TARGET)
	gcc ./dummy/dummy.c $(C_FLAGS) -L./$(BUILD_DIR) -Wl,-rpath,./ -ljsonc -o $@

dummy: $(DUMMY_TARGET)

all: $(TARGET)

clean:
	rm -rf $(BUILD_DIR)
	rm -f $(DUMMY_TARGET)

.PHONY: all clean dummy
