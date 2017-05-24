CXX      = g++ --std=c++11
CXXFLAGS = -g -Wall `oficina2-config --cppflags`
CXXLIBS  = `oficina2-config --libs`
CXXOUT   = -o
CXXOBJ   = -c

DEL      = rm -rf

BIN      = bin/Basic3D
FILES    = src/main.cpp

.PHONY: clean

all: $(OBJS)
	$(CXX) $(FILES) $(CXXFLAGS) $(CXXLIBS) $(CXXOUT) $(BIN)

clean:
	$(DEL) obj/*.o
