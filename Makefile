#
# This is a common Makefile for code examples from the book
# "Software - Principles and Practice using C++" by Bjarne Stroustrup
#

#
# Usage:
#     make        - Build all examples
#     make clean  - Clean all examples
#

INCLUDES = 
LIBS     = -lstdc++
CXXFLAGS = $(INCLUDES) -std=c++17 -Wall  -g 
LIBFLAGS =  
AR       = ar

.SUFFIXES: .cpp .o

# Create a list of source files.
SOURCES  = $(shell ls *.cpp)
# Create a list of object files from the source file lists.
OBJECTS = ${SOURCES:.cpp=.o}     
# Create a list of targets.
TARGETS = test 

# Build all targets by default
all: 	$(TARGETS)


$(TARGETS): $(OBJECTS)
		$(CXX) $(CXXFLAGS) -o $@ $(OBJECTS) 

# A rule to build .o file out of a .cpp file
%.o: %.cpp %.h
	$(CXX) $(CXXFLAGS) -o $@ -c $<

# A rule to clean all the intermediates and targets
clean:	
	rm -rf $(TARGETS) $(OBJECTS) *.out *.stackdump
