CPP := g++
CPPFLAGS := -std=c++11 -Wall -O2 -fopenmp

SRC := $(wildcard *.cpp)
EXE := $(patsubst %.cpp,%,$(filter %.cpp,$(SRC)))

.PHONY: all
all: $(EXE)

# pattern rules
% : %.cpp
	$(CPP) $(CPPFLAGS) -o $@ $< $(LIB)

.PHONY: clean
clean:
	$(RM) $(EXE)
