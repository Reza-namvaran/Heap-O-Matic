# ===============================
# Compiler & Flags
# ===============================

CC      := gcc
CFLAGS  := -Wall -Wextra -Wpedantic -std=gnu11 -g -O2 -D_GNU_SOURCE
INCLUDES:= -Iinclude

# Enable / disable features here
CFLAGS += \
    -DENABLE_GC=1 \
    -DENABLE_QUARANTINE=1 \
    -DENABLE_POISONING=0 \
    -DENABLE_SPRAY_DETECT=0 \
    -DENABLE_DEBUG=1

LDFLAGS := -pthread

# ===============================
# Directories
# ===============================

SRC_DIR     := src
CORE_DIR    := $(SRC_DIR)/core
GC_DIR      := $(SRC_DIR)/gc
POOL_DIR    := $(SRC_DIR)/pools
SEC_DIR     := $(SRC_DIR)/security
UTIL_DIR    := $(SRC_DIR)/utils

TEST_DIR    := tests
UNIT_DIR    := $(TEST_DIR)/unit
STRESS_DIR  := $(TEST_DIR)/stress

BUILD_DIR   := build
BIN_DIR     := bin

# ===============================
# Source Files
# ===============================

CORE_SRC := \
    $(CORE_DIR)/alloc.c \
    $(CORE_DIR)/hfree.c \
    $(CORE_DIR)/heap_init.c \
    $(CORE_DIR)/coalescing.c \
    $(CORE_DIR)/split.c

GC_SRC := \
    $(GC_DIR)/gc.c

POOL_SRC := \
    $(POOL_DIR)/pool.c

SEC_SRC := \
    $(SEC_DIR)/quarantine.c \
    $(SEC_DIR)/validate.c

UTIL_SRC := \
    $(UTIL_DIR)/debug.c \
    $(UTIL_DIR)/helpers.c

LIB_SRC := \
    $(CORE_SRC) \
    $(GC_SRC) \
    $(POOL_SRC) \
    $(SEC_SRC) \
    $(UTIL_SRC)

LIB_OBJ := $(LIB_SRC:%.c=$(BUILD_DIR)/%.o)

# ===============================
# Test Sources
# ===============================

UNIT_TEST_SRC := $(wildcard $(UNIT_DIR)/*.c)
STRESS_TEST_SRC := $(wildcard $(STRESS_DIR)/*.c)

UNIT_TEST_BIN := $(UNIT_TEST_SRC:$(UNIT_DIR)/%.c=$(BIN_DIR)/%)
STRESS_TEST_BIN := $(STRESS_TEST_SRC:$(STRESS_DIR)/%.c=$(BIN_DIR)/%)

# ===============================
# Default Target
# ===============================

.PHONY: all
all: tests

# ===============================
# Build Rules
# ===============================

$(BUILD_DIR)/%.o: %.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

$(BIN_DIR)/%: $(UNIT_DIR)/%.c $(LIB_OBJ)
	@mkdir -p $(BIN_DIR)
	$(CC) $(CFLAGS) $(INCLUDES) $^ -o $@ $(LDFLAGS)

$(BIN_DIR)/%_stress: $(STRESS_DIR)/%.c $(LIB_OBJ)
	@mkdir -p $(BIN_DIR)
	$(CC) $(CFLAGS) $(INCLUDES) $^ -o $@ $(LDFLAGS)

# ===============================
# Tests
# ===============================

.PHONY: tests
tests: unit-tests stress-tests

.PHONY: unit-tests
unit-tests: $(UNIT_TEST_BIN)
	@echo "=== Running unit tests ==="
	@for t in $(UNIT_TEST_BIN); do \
		echo ">> $$t"; \
		./$$t || exit 1; \
	done

.PHONY: stress-tests
stress-tests: $(STRESS_TEST_BIN)
	@echo "=== Running stress tests ==="
	@for t in $(STRESS_TEST_BIN); do \
		echo ">> $$t"; \
		./$$t || exit 1; \
	done

# ===============================
# Single Test
# ===============================

# Example:
# make run TEST=test_gc
.PHONY: run
run:
	@if [ -z "$(TEST)" ]; then \
		echo "Usage: make run TEST=test_gc"; \
		exit 1; \
	fi
	@./$(BIN_DIR)/$(TEST)

# ===============================
# Clean
# ===============================

.PHONY: clean
clean:
	rm -rf $(BUILD_DIR) $(BIN_DIR)

