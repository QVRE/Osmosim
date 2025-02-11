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
TARGET = osmosim

#make sure you extract the win64_mingw-w64.zip raylib release as raylib/
ifdef MINGW
	CXX = x86_64-w64-mingw32-g++
	CXXFLAGS += -Iraylib/include -Lraylib/lib
	LIBS += -static -lkernel32 -lgdi32 -luser32 -lwinmm
	TARGET := $(addsuffix .exe,$(TARGET))
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
