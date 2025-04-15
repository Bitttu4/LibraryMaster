# Compiler and flags
CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -O2
LDFLAGS = -pthread

# Source files and object files
SRCS = main.cpp book.cpp library.cpp server.cpp
OBJS = $(SRCS:.cpp=.o)

# Output executable
TARGET = library_system

# Build target
all: $(TARGET)

# Create target directory
$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDFLAGS)

# Compile source files
%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Create the www directory and copy static files
setup:
	mkdir -p www

# Clean up
clean:
	rm -f $(OBJS) $(TARGET)

# Phony targets
.PHONY: all clean setup
