# Compiler
CXX := g++

# Compiler flags
CXXFLAGS := -std=c++17 -Wall -I./inc -pthread -O3 -mavx2

# Directories
SRC_DIR := src
INC_DIR := inc
BUILD_DIR := build

# Main file (in the root directory)
MAIN := main.cpp

# Find all .cpp files in the src directory (excluding main.cpp if it's there)
SOURCES := $(filter-out $(MAIN),$(wildcard $(SRC_DIR)/*.cpp))

# Generate object file names (excluding main.o)
OBJECTS := $(patsubst $(SRC_DIR)/%.cpp,$(BUILD_DIR)/%.o,$(SOURCES))

# Output executable name
TARGET := main.exe

# Default target
all: $(BUILD_DIR) $(TARGET)

# Create build directory if it doesn't exist
$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

# Link object files and main.cpp to create the executable
$(TARGET): $(OBJECTS) $(MAIN)
	$(CXX) $(CXXFLAGS) $(MAIN) $(OBJECTS) -o $@

# Compile source files into object files
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Clean up built files
clean:
	rm -rf $(BUILD_DIR) $(TARGET)

.PHONY: all clean