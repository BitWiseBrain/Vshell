CXX ?= g++
CXXFLAGS ?= -std=c++17 -Wall -Wextra -Wpedantic -O2
CPPFLAGS ?= -Iinclude
BUILD_DIR := build
SOURCES := main.cpp src/linkedlist.cpp src/hashtable.cpp src/directorystack.cpp \
           src/parser.cpp src/shell.cpp
OBJECTS := $(SOURCES:%.cpp=$(BUILD_DIR)/%.o)

.PHONY: all clean

DEPFILES := $(OBJECTS:.o=.d)

all: $(BUILD_DIR)/vshell

$(BUILD_DIR)/vshell: $(OBJECTS)
	$(CXX) $(CXXFLAGS) $^ -o $@

$(BUILD_DIR)/%.o: %.cpp
	mkdir -p $(dir $@)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -MMD -MP -c $< -o $@

clean:
	rm -rf $(BUILD_DIR)

-include $(DEPFILES)
