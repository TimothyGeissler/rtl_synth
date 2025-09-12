# Compiler and flags
CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -O2 -I.

# Source files
SOURCES = main.cpp fmodel.cpp \
	components/quad_and_74hc08.cpp \
	components/quad_or_74hc32.cpp \
	components/quad_nand_74hc00.cpp \
	components/quad_nor_74hc02.cpp \
	components/quad_xor_74hc86.cpp \
	components/quad_not_74hc04.cpp \
	components/dual_dff_74hc74.cpp

# Object files
OBJECTS = $(SOURCES:.cpp=.o)

# Executable name
TARGET = fmodel_sim

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CXX) $(CXXFLAGS) -o $@ $(OBJECTS)

clean:
	rm -f $(OBJECTS) $(TARGET)

run_adder:
	./$(TARGET) ../netlist/adder_4bit.net test_vectors/adder_4bit_tests.txt

run_full_adder:
	./$(TARGET) ../netlist/full_adder.net test_vectors/full_adder_tests.txt

.PHONY: all clean run_adder run_full_adder
