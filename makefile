# Compiler and flags
CXX = g++
CXXFLAGS = -O2
LIBS = -lraylib

ifdef DEBUG
	CXXFLAGS = -O0 -g
endif

# Source files and output binary
SRCS = common.cpp collision.cpp game.cpp
HEADERS = common.hpp collision.hpp
OBJS = $(SRCS:.cpp=.o)
TARGET = out

ifdef MINGW
	CXX = x86_64-w64-mingw32-g++
	CXXFLAGS := $(CXXFLAGS) -Iraylib/src -Lraylib/src
	LIBS := $(LIBS) -static
endif


all: $(TARGET)

# Rule to build the target executable
$(TARGET): main.cpp $(OBJS)
	$(CXX) -o $@ main.cpp $(OBJS) $(CXXFLAGS) $(LIBS)

# Rule to compile source files into object files
%.o: %.cpp $(HEADERS)
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Clean rule to remove all binaries and objects
clean:
	rm $(OBJS) $(TARGET)
