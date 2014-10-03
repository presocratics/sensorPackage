CC=g++
CXXFLAGS = -g -std=c++11 -Wall -Werror

LIBS = -lm 

spanread: spanread.o ourerr.o
	$(CXX) -o ../../bin/spanread spanread.o ourerr.o $(CXXFLAGS) $(LIBS)

.cpp.o:
	$(CXX) $(CXXFLAGS) -c $< -o $@

.PHONY: clean
clean:
	rm -f *.o
	rm -f ../../bin/spanread
