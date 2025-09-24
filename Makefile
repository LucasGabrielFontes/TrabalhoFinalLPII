# Makefile
.RECIPEPREFIX := >

CXX ?= g++
BUILD_DIR ?= build
BIN_DIR ?= $(BUILD_DIR)/bin
TARGET := $(BIN_DIR)/log_demo

SRC := src/tslogger.cpp src/main_logging_demo.cpp
OBJ := $(patsubst src/%.cpp,$(BUILD_DIR)/%.o,$(SRC))
DEPS := $(OBJ:.o=.d)

CPPFLAGS ?= -Iinclude -MMD -MP
CXXFLAGS ?= -std=c++20 -Wall -Wextra -O2
LDFLAGS ?=
LDLIBS ?= -pthread

THREADS ?= 8
LINES ?= 200
TSLOG_LEVEL ?= INFO

MODE ?= release
ifeq ($(MODE),debug)
  CXXFLAGS := $(filter-out -O2,$(CXXFLAGS)) -O0 -g
endif

USE_THIRD_PARTY_TSLOG ?= 0
ifeq ($(USE_THIRD_PARTY_TSLOG),1)
  CPPFLAGS += -DTSLOGGER_WITH_TSLOG=1 -Ithird_party/libtslog/include
  LDFLAGS += -Lthird_party/libtslog
  LDLIBS += -ltslog
endif

.PHONY: all release debug run clean

all: release
release: MODE := release
release: $(TARGET)
debug: MODE := debug
debug: $(TARGET)

$(TARGET): $(OBJ) | $(BIN_DIR)
> $(CXX) $(CXXFLAGS) $(OBJ) -o $@ $(LDFLAGS) $(LDLIBS)

$(BUILD_DIR)/%.o: src/%.cpp | $(BUILD_DIR)
> mkdir -p $(dir $@)
> $(CXX) $(CPPFLAGS) $(CXXFLAGS) -c $< -o $@

$(BUILD_DIR):
> mkdir -p $@

$(BIN_DIR):
> mkdir -p $@

run: $(TARGET)
> TSLOG_LEVEL=$(TSLOG_LEVEL) $(TARGET) $(THREADS) $(LINES)

clean:
> rm -rf $(BUILD_DIR)

-include $(DEPS)