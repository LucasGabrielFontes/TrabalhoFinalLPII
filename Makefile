# Makefile
.RECIPEPREFIX := >
CXX ?= g++
BUILD_DIR ?= build
BIN_DIR ?= $(BUILD_DIR)/bin
TARGETS := $(BIN_DIR)/log_demo $(BIN_DIR)/chat_server $(BIN_DIR)/client_simple
SRC := src/tslogger.cpp src/main_logging_demo.cpp src/main_server.cpp examples/client_simple.cpp
OBJ := $(patsubst src/%.cpp,$(BUILD_DIR)/%.o,$(filter src/%.cpp,$(SRC))) \
       $(patsubst examples/%.cpp,$(BUILD_DIR)/%.o,$(filter examples/%.cpp,$(SRC)))
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

.PHONY: all release debug run clean

all: release

release: MODE := release
release: $(TARGETS)

debug: MODE := debug
debug: $(TARGETS)

$(BIN_DIR)/log_demo: $(OBJ) | $(BIN_DIR)
> $(CXX) $(CXXFLAGS) $(filter %main_logging_demo.o,$(OBJ)) $(filter %tslogger.o,$(OBJ)) -o $@ $(LDFLAGS) $(LDLIBS)

$(BIN_DIR)/chat_server: $(OBJ) | $(BIN_DIR)
> $(CXX) $(CXXFLAGS) $(filter %main_server.o,$(OBJ)) $(filter %tslogger.o,$(OBJ)) -o $@ $(LDFLAGS) $(LDLIBS)

$(BIN_DIR)/client_simple: $(OBJ) | $(BIN_DIR)
> $(CXX) $(CXXFLAGS) $(filter %client_simple.o,$(OBJ)) -o $@ $(LDFLAGS) $(LDLIBS)

$(BUILD_DIR)/%.o: src/%.cpp | $(BUILD_DIR)
> mkdir -p $(dir $@)
> $(CXX) $(CPPFLAGS) $(CXXFLAGS) -c $< -o $@

$(BUILD_DIR)/%.o: examples/%.cpp | $(BUILD_DIR)
> mkdir -p $(dir $@)
> $(CXX) $(CPPFLAGS) $(CXXFLAGS) -c $< -o $@

$(BUILD_DIR):
> mkdir -p $@

$(BIN_DIR):
> mkdir -p $@

run: $(BIN_DIR)/chat_server
> TSLOG_LEVEL=$(TSLOG_LEVEL) $(BIN_DIR)/chat_server

clean:
> rm -rf $(BUILD_DIR)

-include $(DEPS)