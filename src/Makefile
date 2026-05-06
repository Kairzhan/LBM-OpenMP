export OMPI_CXX=g++-15

CXX      := mpicxx
CXXFLAGS := -std=c++17 -fopenmp -Wall -Wextra -O3 #-foffload=nvptx-none
LDFLAGS  := -lmpi
TARGET   := ../build/lbm

# This will find all .cpp files in the current directory
SRCS     := $(wildcard *.cpp)
# This converts the .cpp list into a .o (object) list
OBJS     := $(SRCS:.cpp=.o)

# Default Rule
all: $(TARGET)

# Link the target
$(TARGET): $(OBJS)
	$(CXX) $(LDFLAGS) $(OBJS) -fopenmp -o $(TARGET)
	@echo "Build successful: $(TARGET)"

# Compile source files to object files
%.o: %.cpp ParameterReader.hpp ParameterStore.hpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Clean up build artifacts
clean:
	rm -f *.o $(TARGET) *~

.PHONY: all clean
