CXX      := g++-15
CXXFLAGS := -std=c++17 -Wall -Wextra
TARGET   := ../build/lbm

# This will find all .cpp files in the current directory
SRCS     := $(wildcard *.cpp)
# This converts the .cpp list into a .o (object) list
OBJS     := $(SRCS:.cpp=.o)

# Default Rule
all: $(TARGET)

# Link the target
$(TARGET): $(OBJS)
	$(CXX) $(OBJS) -o $(TARGET)
	@echo "Build successful: $(TARGET)"

# Compile source files to object files
%.o: %.cpp ParameterReader.hpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Clean up build artifacts
clean:
	rm -f *.o $(TARGET) *~

.PHONY: all clean
