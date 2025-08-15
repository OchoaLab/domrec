# Define the C++ compiler and flags
CXX=g++
# Common flags for C++17, warnings, and debugging
CXXFLAGS=-std=c++17 -Wall -Wextra -g

# Define the executable name
TARGET=build/domrec

# List your source files
SOURCES=src/main.cpp src/count_lines.cpp

# Automatically generate object file names from source files
OBJECTS=$(SOURCES:.cpp=.o)

# Default target: builds the executable
all: $(TARGET)

# Rule to build the executable from object files
$(TARGET): $(OBJECTS)
	$(CXX) $(CXXFLAGS) $(OBJECTS) -o $(TARGET)

# Rule to build object files from source files (pattern rule)
%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Clean target: removes generated files
clean:
	rm -f $(TARGET) $(OBJECTS)

test:
	. tests/test.bash

.PHONY: all clean test  # Declare 'all' and 'clean' as phony targets (actions, not actual files)
