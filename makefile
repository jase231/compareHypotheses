# Compiler
CXX = g++
# ROOT flags
ROOTCFLAGS := $(shell root-config --cflags)
ROOTLIBS := $(shell root-config --libs)

# Source files
SRCS = compare_hypotheses.cpp main.cpp

# Object files
OBJS = $(SRCS:.cpp=.o)

# Executable name
EXEC = compare_hypotheses

# Default rule to build the executable
all: $(EXEC)

# Rule to create the executable by linking object files
$(EXEC): $(OBJS)
	$(CXX) -o $@ $(OBJS) $(ROOTCFLAGS) $(ROOTLIBS)

# Rule to compile the object files
%.o: %.cpp
	$(CXX) -c $< $(ROOTCFLAGS) $(ROOTLIBS)

# Clean up generated files
clean:
	rm -f $(OBJS) $(EXEC)

.PHONY: clean

