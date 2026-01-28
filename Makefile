CC = gcc
CFLAGS = -Wall -Wextra -Werror -std=c99 -pedantic -O2 -g -fPIC
LDFLAGS = -lm

SRC_DIR = src
LIB_DIR = $(SRC_DIR)/lib
CLI_DIR = $(SRC_DIR)/cli
TEST_DIR = tests
BUILD_DIR = build
INCLUDE_DIR = include

# Library sources (core logic)
LIB_SOURCES = $(wildcard $(LIB_DIR)/*.c)
LIB_OBJECTS = $(LIB_SOURCES:$(LIB_DIR)/%.c=$(BUILD_DIR)/lib/%.o)

# CLI sources (main + CLI-specific)
CLI_SOURCES = $(wildcard $(CLI_DIR)/*.c)
CLI_OBJECTS = $(CLI_SOURCES:$(CLI_DIR)/%.c=$(BUILD_DIR)/cli/%.o)

# Test sources
TEST_SOURCES = $(wildcard $(TEST_DIR)/*.c)
TEST_OBJECTS = $(TEST_SOURCES:$(TEST_DIR)/%.c=$(BUILD_DIR)/test/%.o)

# Targets
SHARED_LIB = libtaguchi.so
STATIC_LIB = libtaguchi.a
CLI_TARGET = taguchi
TEST_TARGET = $(BUILD_DIR)/test_runner

# Platform-specific library extension
UNAME_S := $(shell uname -s)
ifeq ($(UNAME_S),Darwin)
    SHARED_LIB = libtaguchi.dylib
    SHARED_FLAGS = -dynamiclib -install_name @rpath/$(SHARED_LIB)
else ifeq ($(OS),Windows_NT)
    SHARED_LIB = taguchi.dll
    SHARED_FLAGS = -shared
else
    SHARED_FLAGS = -shared -Wl,-soname,$(SHARED_LIB)
endif

.PHONY: all lib cli test check install clean

all: lib cli

# Build shared library
lib: $(SHARED_LIB) $(STATIC_LIB)

$(SHARED_LIB): $(LIB_OBJECTS)
	$(CC) $(SHARED_FLAGS) $(LIB_OBJECTS) -o $@ $(LDFLAGS)

$(STATIC_LIB): $(LIB_OBJECTS)
	ar rcs $@ $(LIB_OBJECTS)

$(BUILD_DIR)/lib/%.o: $(LIB_DIR)/%.c | $(BUILD_DIR)/lib
	$(CC) $(CFLAGS) -I. -I$(INCLUDE_DIR) -c $< -o $@

# Build CLI
cli: $(CLI_TARGET)

$(CLI_TARGET): $(CLI_OBJECTS) $(SHARED_LIB)
	$(CC) $(CLI_OBJECTS) -L. -ltaguchi -o $@ $(LDFLAGS)

$(BUILD_DIR)/cli/%.o: $(CLI_DIR)/%.c | $(BUILD_DIR)/cli
	$(CC) $(CFLAGS) -I. -I$(INCLUDE_DIR) -I$(LIB_DIR) -c $< -o $@

# Build tests
test: $(TEST_TARGET)
	LD_LIBRARY_PATH=. ./$(TEST_TARGET)
	@if command -v valgrind >/dev/null 2>&1; then \
		echo "Running valgrind..."; \
		LD_LIBRARY_PATH=. valgrind --leak-check=full --error-exitcode=1 ./$(TEST_TARGET); \
	else \
		echo "Warning: valgrind not found, skipping memory check."; \
	fi

$(TEST_TARGET): $(TEST_OBJECTS) $(LIB_OBJECTS)
	$(CC) $(TEST_OBJECTS) $(LIB_OBJECTS) -o $@ $(LDFLAGS)

$(BUILD_DIR)/test/%.o: $(TEST_DIR)/%.c | $(BUILD_DIR)/test
	$(CC) $(CFLAGS) -I. -I$(INCLUDE_DIR) -c $< -o $@

# Static analysis
check: test
	@echo "Running static analysis..."
	cppcheck --enable=all --suppress=missingIncludeSystem $(LIB_DIR) $(CLI_DIR)

# Create build directories
$(BUILD_DIR)/lib $(BUILD_DIR)/cli $(BUILD_DIR)/test:
	mkdir -p $@

# Installation
PREFIX = /usr/local
LIBDIR = $(PREFIX)/lib
INCDIR = $(PREFIX)/include
BINDIR = $(PREFIX)/bin

install: lib cli
	install -d $(LIBDIR) $(INCDIR) $(BINDIR)
	install -m 755 $(SHARED_LIB) $(LIBDIR)/
	install -m 644 $(STATIC_LIB) $(LIBDIR)/
	install -m 644 $(INCLUDE_DIR)/taguchi.h $(INCDIR)/
	install -m 755 $(CLI_TARGET) $(BINDIR)/
	@if command -v ldconfig >/dev/null 2>&1; then ldconfig; fi

# Uninstall
uninstall:
	rm -f $(LIBDIR)/$(SHARED_LIB)
	rm -f $(LIBDIR)/$(STATIC_LIB)
	rm -f $(INCDIR)/taguchi.h
	rm -f $(BINDIR)/$(CLI_TARGET)

# Clean
clean:
	rm -rf $(BUILD_DIR) $(SHARED_LIB) $(STATIC_LIB) $(CLI_TARGET)

# Python bindings
.PHONY: python-install python-test
python-install: lib
	cd bindings/python && python3 setup.py install --user

python-test: python-install
	cd bindings/python && python3 test_taguchi.py

# Node bindings
.PHONY: node-install node-test
node-install: lib
	cd bindings/node && npm install

node-test: node-install
	cd bindings/node && npm test

# Documentation
.PHONY: docs
docs:
	doxygen Doxyfile
