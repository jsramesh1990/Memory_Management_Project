# Memory Allocator Project Makefile

CC = gcc
AR = ar
RM = rm -f
MKDIR = mkdir -p

# Directories
SRCDIR = src
INCDIR = include
BUILDDIR = build
TARGETDIR = lib
TESTDIR = tests
BINDIR = bin

# Compiler flags
CFLAGS = -Wall -Wextra -Wpedantic -Wshadow -Wconversion
CFLAGS += -std=c11 -g -O2
CFLAGS += -I$(INCDIR)
CFLAGS += -D_GNU_SOURCE

# Platform detection
UNAME_S := $(shell uname -s)
ifeq ($(UNAME_S),Linux)
    CFLAGS += -D__linux__
    LIBS = -lpthread
endif
ifeq ($(UNAME_S),Darwin)
    CFLAGS += -D__APPLE__
    LIBS =
endif

# Debug build
debug: CFLAGS += -DDEBUG -g3 -fsanitize=address -fsanitize=undefined
debug: all

# Release build
release: CFLAGS += -O3 -DNDEBUG -flto
release: all

# Source files
SOURCES = $(wildcard $(SRCDIR)/*.c)
OBJECTS = $(patsubst $(SRCDIR)/%.c,$(BUILDDIR)/%.o,$(SOURCES))

# Test files
TEST_SOURCES = $(wildcard $(TESTDIR)/unit/*.c)
TEST_TARGETS = $(patsubst $(TESTDIR)/unit/%.c,$(BINDIR)/test_%,$(TEST_SOURCES))

# Targets
TARGET = $(TARGETDIR)/liballoc.a

.PHONY: all clean test valgrind coverage docs

all: directories $(TARGET)

directories:
	@$(MKDIR) $(BUILDDIR) $(TARGETDIR) $(BINDIR)

$(TARGET): $(OBJECTS)
	$(AR) rcs $@ $^

$(BUILDDIR)/%.o: $(SRCDIR)/%.c
	@$(MKDIR) $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

# Build test executables
$(BINDIR)/test_%: $(TESTDIR)/unit/%.c $(TARGET)
	$(CC) $(CFLAGS) -o $@ $< $(TARGET) $(LIBS)

# Run all tests
test: $(TEST_TARGETS)
	@echo "\n=== Running All Tests ===\n"
	@for test in $(TEST_TARGETS); do \
		echo "\n--- Running $$test ---"; \
		./$$test; \
		if [ $$? -ne 0 ]; then \
			echo "Test failed: $$test"; \
			exit 1; \
		fi \
	done
	@echo "\n=== All Tests Passed ===\n"

# Run specific test
test-%: $(BINDIR)/test_%
	./$(BINDIR)/test_$*

# Valgrind memory check
valgrind: $(TEST_TARGETS)
	@for test in $(TEST_TARGETS); do \
		echo "Running valgrind on $$test"; \
		valgrind --leak-check=full --error-exitcode=1 ./$$test; \
	done

# Code coverage
coverage: CFLAGS += -fprofile-arcs -ftest-coverage -O0
coverage: clean test
	@gcov -b $(SRCDIR)/*.c
	@echo "Coverage data generated"

# Clean
clean:
	$(RM) -r $(BUILDDIR) $(TARGETDIR) $(BINDIR)
	$(RM) *.gcda *.gcno *.gcov

# Help
help:
	@echo "Available targets:"
	@echo "  make            - Build library (debug mode)"
	@echo "  make release    - Build optimized library"
	@echo "  make test       - Build and run all tests"
	@echo "  make test-pool  - Run pool allocator tests"
	@echo "  make valgrind   - Run tests with memory leak detection"
	@echo "  make coverage   - Generate code coverage report"
	@echo "  make clean      - Remove build artifacts"
