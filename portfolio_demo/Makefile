CC ?= cc
CFLAGS ?= -std=c11 -Wall -Wextra -Werror -pedantic -O2

BUILD_DIR := build
TARGET := $(BUILD_DIR)/test_mini_psu_demo
SOURCES := src/mini_psu_demo.c tests/test_mini_psu_demo.c

.PHONY: all test clean

all: test

$(TARGET): $(SOURCES) include/mini_psu_demo.h
	mkdir -p $(BUILD_DIR)
	$(CC) $(CFLAGS) -Iinclude $(SOURCES) -o $(TARGET)

test: $(TARGET)
	./$(TARGET)

clean:
	rm -rf $(BUILD_DIR)
