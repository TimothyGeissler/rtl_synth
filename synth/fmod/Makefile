# Makefile for Functional Model Framework

CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -O2
INCLUDES = -I.
LDFLAGS = 

# Source files
SOURCES = main.cpp fmodel.cpp components.cpp

# Object files
OBJECTS = $(SOURCES:.cpp=.o)

# Target executable
TARGET = fmodel_sim

# Default target
all: $(TARGET)

# Build the main executable
$(TARGET): $(OBJECTS)
	$(CXX) $(CXXFLAGS) $(INCLUDES) -o $@ $^ $(LDFLAGS)

# Compile source files
%.o: %.cpp
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

# Clean build artifacts
clean:
	rm -f $(OBJECTS) $(TARGET)

# Run tests
test: $(TARGET)
	@echo "Running Full Adder Test..."
	./$(TARGET) ../../full_adder_netlist.json test_vectors/full_adder_tests.txt
	@echo ""
	@echo "Running 4-bit Adder Test..."
	./$(TARGET) ../../adder_4bit_netlist.json test_vectors/adder_4bit_tests.txt

# Run individual component tests
test-components:
	@echo "Testing individual components..."
	cd components && make test

# Install (copy to system)
install: $(TARGET)
	cp $(TARGET) /usr/local/bin/

# Uninstall
uninstall:
	rm -f /usr/local/bin/$(TARGET)

# Help
help:
	@echo "Available targets:"
	@echo "  all          - Build the functional model simulator"
	@echo "  clean        - Remove build artifacts"
	@echo "  test         - Run all tests"
	@echo "  test-components - Test individual components"
	@echo "  install      - Install to system"
	@echo "  uninstall    - Remove from system"
	@echo "  help         - Show this help"

.PHONY: all clean test test-components install uninstall help
